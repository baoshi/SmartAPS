#include <functional>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "ssd1322.h"
#include "terminal.h"
#include "button.h"
#include "beeper.h"
#include "ina226.h"
#include "smartaps.h"


using namespace esphome;
using namespace esphome::sensor;

extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_a;
extern gpio::GPIOSwitch *out_b;
extern gpio::GPIOSwitch *out_usb;

static const char *TAG = "smartaps";

SSD1322 display(/*mosi*/23, /*sclk*/18, /*cs/*/17, /*dc*/19, /*rst*/5);
Terminal terminal;
Beeper beeper;
INA226 ina226_a, ina226_b, ina226_usb;


#define GPIO_OUT_A      25
#define GPIO_OUT_B      32
#define GPIO_OUT_USB    16

#define GPIO_BUTTON_S1  0
#define GPIO_BUTTON_S2  34
#define GPIO_BUTTON_S3  35


SmartAPS::SmartAPS() :
    _uptime(0),
    _sample_buffer_mux(portMUX_INITIALIZER_UNLOCKED),
    _sample_count(0),
    _sample_timer(NULL)
{
    sensor_usb_v = new Sensor();
    sensor_usb_c = new Sensor();
}


void SmartAPS::register_sensors(void)
{
    // INA226 USB Voltage
    App.register_sensor(sensor_usb_v);
    sensor_usb_v->set_name("USB Voltage");
    sensor_usb_v->set_unit_of_measurement("V");
    sensor_usb_v->set_accuracy_decimals(1);
    sensor_usb_v->set_force_update(false);
    // INA226 USB Current
    App.register_sensor(sensor_usb_c);
    sensor_usb_c->set_name("USB Current");
    sensor_usb_c->set_unit_of_measurement("A");
    sensor_usb_c->set_accuracy_decimals(1);
    sensor_usb_c->set_force_update(false);
}


void SmartAPS::publish_uptime(void)
{
    const uint32_t ms = millis();
    const uint64_t ms_mask = (1ULL << 32) - 1ULL;
    const uint32_t last_ms = _uptime & ms_mask;
    if (ms < last_ms) {
        _uptime += ms_mask + 1ULL;
        ESP_LOGD(TAG, "Detected roll-over \xf0\x9f\xa6\x84");
    }
    _uptime &= ~ms_mask;
    _uptime |= ms;
    uint32_t seconds = (uint32_t)(_uptime / 1000UL);
    uint32_t minutes = (uint32_t)(seconds % 3600) / 60;
    uint32_t hours = (uint32_t)(seconds % 86400) / 3600;
    uint32_t days = (uint32_t)(seconds / 86400);
    // seconds = seconds % 60;
    const char *fmt = "%d days, %02d:%02d";
    #define UPTIME_LEN 31
    char s[UPTIME_LEN + 1];
    snprintf(s, UPTIME_LEN, fmt, days, hours, minutes);
    s[UPTIME_LEN] = '\0';
    this->publish_state(s);
}


#define SAMPLE_BIT  0x01
#define STOP_BIT     0x02
static TaskHandle_t sample_task_handle;


void SmartAPS::_sample_fn(void * param)
{
    SmartAPS* me = reinterpret_cast<SmartAPS*>(param);
    me->_sample_count = 0;
    uint32_t notify_val;
    ESP_LOGI(TAG, "Sampling task started");
    for (;;)
    {
        if (xTaskNotifyWait(pdFALSE, ULONG_MAX, &notify_val, portMAX_DELAY) == pdPASS)
        {
            if ((notify_val & STOP_BIT) != 0)
            {
                // TODO: need cleanup here
                break; // exit infinite loop
            }
            if ((notify_val & SAMPLE_BIT) != 0)
            {
                int16_t s, b;
                if (ina226_a.read(s, b))
                {
                    portENTER_CRITICAL(&(me->_sample_buffer_mux));
                    if (me->_sample_count < me->_sample_buffer_length)
                    {
                        me->_sample_buffer_v[me->_sample_count] = b;
                        me->_sample_buffer_c[me->_sample_count] = s;
                        ++(me->_sample_count);
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Sample buffer overflow");
                    }
                    portEXIT_CRITICAL(&(me->_sample_buffer_mux));
                }
                else
                {
                    ESP_LOGW(TAG, "INA226 read failure");
                }
            }
        } 
        else
        {
            // Wait failed, should not happen when we set max delay
        }
    }
    ESP_LOGI(TAG, "Sampling task end");
    vTaskDelete(NULL);
}


void IRAM_ATTR on_timer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (sample_task_handle != NULL)
    {
        xTaskNotifyFromISR(sample_task_handle, SAMPLE_BIT, eSetBits, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR ();
        }
    }
}


void SmartAPS::setup()
{
    Wire.begin();
    ina226_a.begin(&Wire, 0x4a);
    ina226_b.begin(&Wire, 0x41);
    ina226_usb.begin(&Wire, 0x45);
    display.begin();
    s1.init(GPIO_BUTTON_S1, LOW, INPUT_PULLUP);
    s2.init(GPIO_BUTTON_S2, LOW, INPUT);
    s3.init(GPIO_BUTTON_S3, LOW, INPUT);
    terminal.begin(&display);
    s1.begin(false);
    s2.begin(false);
    s3.begin(false);
    beeper.setup();
    _timestamp_per_1s = _timestamp_per_1m = millis();
    _sample_timer = timerBegin(0, 80, true);    // 1us timer, count up
    timerAttachInterrupt(_sample_timer, on_timer, true);
}


void SmartAPS::loop()
{
    unsigned long now = millis();
    if (now - _timestamp_per_1s > 1000)
    {
        _timestamp_per_1s = now;
    }
    if (now - _timestamp_per_1m > 60000)
    {
        publish_uptime();
        _timestamp_per_1m = now;
    }
    // Process sample buffer
    portENTER_CRITICAL(&_sample_buffer_mux);
    for (int i = 0; i < _sample_count; ++i)
    {
        /*
        float v = _sample_buffer_v[i] * 0.00125;
        int16_t s = _sample_buffer_c[i];
        float c = s / 10000.0f; // ( * 2.5 / 1000000 / 0.025);
        terminal.printf("%.2fV, %.3fA\n", v, c);
        */
       terminal.printf("%d, %d\n", _sample_buffer_v[i], _sample_buffer_c[i]);
       if ((_sample_buffer_v[i] == -1) || (_sample_buffer_c[i] == -1))
       {
            timerAlarmDisable(_sample_timer);
            xTaskNotify(sample_task_handle, STOP_BIT, eSetBits);
            sample_task_handle = NULL;
       }
    }
    _sample_count = 0;
    portEXIT_CRITICAL(&_sample_buffer_mux);
    // Process buttons
    uint32_t e1 = s1.update(now);
    uint32_t e2 = s2.update(now);
    uint32_t e3 = s3.update(now);
    uint32_t event_id = BUTTON_EVENT_ID(e1);
    uint32_t event_param = BUTTON_EVENT_PARAM(e1); 
    if (event_id != BUTTON_EVENT_NONE)
    {
        switch (event_id)
        {
            case BUTTON_EVENT_CLICK:
                sensor_usb_v->publish_state(5.12);
                sensor_usb_c->publish_state(1.1);
                terminal.printf("S1 click (%dms)\n", event_param);
                if (id(sntp_time).now().is_valid())
                {
                    char buffer[64];
                    size_t ret = id(sntp_time).now().strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M");
                    terminal.println(buffer);
                }
                else
                {
                    terminal.println("No time yet");
                }
                break;
            case BUTTON_EVENT_DOWN:
                terminal.printf("S1 down (%dms)\n", event_param);
                beeper.beep1();
                break;
            case BUTTON_EVENT_HOLD_START:
                terminal.printf("S1 hold start (%dms)\n", event_param);
                break;
            case BUTTON_EVENT_HOLD_END:
                terminal.printf("S1 hold end (%dms)\n", event_param);
                break;
        }
    }
    event_id = BUTTON_EVENT_ID(e2);
    event_param = BUTTON_EVENT_PARAM(e2);
    if (event_id != BUTTON_EVENT_NONE)
    {
        
        switch (event_id)
        {
            case BUTTON_EVENT_CLICK:
                //terminal.printf("S2 click (%dms)\n", event_param);
                if (sample_task_handle == NULL)
                {
                    xTaskCreate(_sample_fn, "SAMPLING", 2000, this, uxTaskPriorityGet(NULL) + 1, &sample_task_handle);
                    timerAlarmWrite(_sample_timer, 50000, true);    // 50ms
                    timerAlarmEnable(_sample_timer);
                }
                break;
            case BUTTON_EVENT_DOWN:
                //terminal.printf("S2 down (%dms)\n", event_param);
                //beeper.beep2();
                break;
            case BUTTON_EVENT_HOLD_START:
                terminal.printf("S2 hold start (%dms)\n", event_param);
                break;
            case BUTTON_EVENT_HOLD_END:
                terminal.printf("S2 hold end (%dms)\n", event_param);
                break;
        }
    }
    event_id = BUTTON_EVENT_ID(e3);
    event_param = BUTTON_EVENT_PARAM(e3); 
    if (event_id != BUTTON_EVENT_NONE)
    {
        switch (event_id)
        {
            case BUTTON_EVENT_CLICK:
                //terminal.printf("S3 click (%dms)\n", event_param);
                if (sample_task_handle != NULL)
                {
                    timerAlarmDisable(_sample_timer);
                    xTaskNotify(sample_task_handle, STOP_BIT, eSetBits);
                    sample_task_handle = NULL;
                }
                break;
            case BUTTON_EVENT_DOWN:
                //terminal.printf("S3 down (%dms)\n", event_param);
                //beeper.beep3();
                break;
            case BUTTON_EVENT_HOLD_START:
                terminal.printf("S3 hold start (%dms)\n", event_param);
                break;
            case BUTTON_EVENT_HOLD_END:
                terminal.printf("S3 hold end (%dms)\n", event_param);
                break;
        }
    }
    beeper.loop();
    display.display();
}




