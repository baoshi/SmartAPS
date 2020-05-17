#include <Arduino.h>
#include <Wire.h>
#include "smartaps.h"


using namespace esphome;
using namespace esphome::sensor;


static const char *TAG = "smartaps";

SmartAPS::SmartAPS() :
    shell_overview(this),
    shell_detail(this)
{
    sensor_usb_v = new Sensor();
    sensor_usb_c = new Sensor();
    sensor_out_a_v = new Sensor();
    sensor_out_a_c = new Sensor();
    sensor_out_b_v = new Sensor();
    sensor_out_b_c = new Sensor();
    _uptime = 0;
    _cur_shell = NULL;
    _next_shell = NULL;
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
    // OUT A Voltage
    App.register_sensor(sensor_out_a_v);
    sensor_out_a_v->set_name("Port A Voltage");
    sensor_out_a_v->set_unit_of_measurement("V");
    sensor_out_a_v->set_accuracy_decimals(1);
    sensor_out_a_v->set_force_update(false);
    // OUT A Current
    App.register_sensor(sensor_out_a_c);
    sensor_out_a_c->set_name("Port A Current");
    sensor_out_a_c->set_unit_of_measurement("A");
    sensor_out_a_c->set_accuracy_decimals(1);
    sensor_out_a_c->set_force_update(false);
    // OUT B Voltage
    App.register_sensor(sensor_out_b_v);
    sensor_out_b_v->set_name("Port B Voltage");
    sensor_out_b_v->set_unit_of_measurement("V");
    sensor_out_b_v->set_accuracy_decimals(1);
    sensor_out_b_v->set_force_update(false);
    // OUT B Current
    App.register_sensor(sensor_out_b_c);
    sensor_out_b_c->set_name("Port B Current");
    sensor_out_b_c->set_unit_of_measurement("A");
    sensor_out_b_c->set_accuracy_decimals(1);
    sensor_out_b_c->set_force_update(false);
}


void SmartAPS::_publish_uptime(void)
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

/*
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
                if (me->ina226_usb.read(s, b))
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
*/

void SmartAPS::setup()
{
    // Display
    oled.init(/*mosi*/23, /*sclk*/18, /*cs/*/17, /*dc*/19, /*rst*/5);
    // Buttons
    sw1.init(GPIO_BUTTON_S1, LOW, INPUT_PULLUP);
    sw2.init(GPIO_BUTTON_S2, LOW, INPUT);
    sw3.init(GPIO_BUTTON_S3, LOW, INPUT);
    // Beeper
    beeper.begin();
    // INA226
    Wire.begin(SDA_PIN, SCL_PIN, 400000);
    ina226_port_a.init(&Wire, INA226_ADDR_PORT_A);
    ina226_port_b.init(&Wire, INA226_ADDR_PORT_B);
    ina226_usb.init(&Wire, INA226_ADDR_USB);
    // Button        
    sw1.begin(false);
    sw2.begin(false);
    sw3.begin(false);
    // Terminal 
    terminal.begin(&oled);
    // Timestamps
    _timestamp_per_1m = millis();
    // Shells
    shell_overview.init();
    shell_detail.init();
    _cur_shell = NULL;
    _next_shell = &shell_overview;
    //_sample_timer = timerBegin(0, 80, true);    // 1us timer, count up
    //timerAttachInterrupt(_sample_timer, on_timer, true);
    oled.clearDisplay();
    oled.display();
}


void SmartAPS::loop()
{
    unsigned long now = millis();
    if (now - _timestamp_per_1m > 60000)
    {
        _publish_uptime();
        _timestamp_per_1m = now;
    }
    if (_next_shell != _cur_shell)
    {
        if (_cur_shell != NULL) _cur_shell->leave(now);
        if (_next_shell != NULL) _next_shell->enter(now);
        _cur_shell = _next_shell;
    }
    _next_shell = _cur_shell->loop(now);

#if 0    
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
       int16_t v = _sample_buffer_v[i];
       int16_t c = _sample_buffer_c[i];
       terminal.printf("%.2fV, %.3fA\n", v * 0.00125, c / 10000.0f);
       if ((v == -1) || (c == -1) )
       {
            timerAlarmDisable(_sample_timer);
            xTaskNotify(sample_task_handle, STOP_BIT, eSetBits);
            sample_task_handle = NULL;
            beeper.beep1();
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
                    xTaskCreatePinnedToCore(_sample_fn, "SAMPLING", 2000, this, uxTaskPriorityGet(NULL) + 1, &sample_task_handle, 1);
                    timerAlarmWrite(_sample_timer, 10000, true);    // 10ms
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
#endif
}