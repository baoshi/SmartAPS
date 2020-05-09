#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "ssd1322.h"
#include "terminal.h"
#include "sht20.h"
#include "button.h"
#include "beeper.h"
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
SHT20 sht20;

#define GPIO_OUT_A      25
#define GPIO_OUT_B      32
#define GPIO_OUT_USB    16

#define GPIO_BUTTON_S1  0
#define GPIO_BUTTON_S2  34
#define GPIO_BUTTON_S3  35


SmartAPS::SmartAPS() : _uptime(0)
{
    sensor_usb_v = new Sensor();
    sensor_usb_c = new Sensor();
    sensor_temperature = new Sensor();
    sensor_humidity = new Sensor();
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
    // SHT20 temperature
    App.register_sensor(sensor_temperature);
    sensor_temperature->set_name("Temperature");
    sensor_temperature->set_unit_of_measurement("\302\260C");   // degreesign in UTF-8 0xc2 0xb0
    sensor_temperature->set_accuracy_decimals(1);
    sensor_temperature->set_force_update(false);
    // SHT20 humidity
    App.register_sensor(sensor_humidity);
    sensor_humidity->set_name("Humidity");
    sensor_humidity->set_unit_of_measurement("%");
    sensor_humidity->set_accuracy_decimals(0);
    sensor_humidity->set_force_update(false);
    sensor_humidity->set_icon("mdi:water-percent");
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
    seconds = seconds % 60;
    const char *fmt = "%d days, %02d:%02d:%02d";
    #define UPTIME_LEN 31
    char s[UPTIME_LEN + 1];
    snprintf(s, UPTIME_LEN, fmt, days, hours, minutes, seconds);
    s[UPTIME_LEN] = '\0';
    this->publish_state(s);
}


void SmartAPS::setup()
{
    Wire.begin();
    sht20.begin(&Wire);
    display.begin();
    s1.init(GPIO_BUTTON_S1, LOW, INPUT_PULLUP);
    s2.init(GPIO_BUTTON_S2, LOW, INPUT);
    s3.init(GPIO_BUTTON_S3, LOW, INPUT);
    terminal.begin(&display);
    s1.begin(false);
    s2.begin(false);
    s3.begin(false);
    beeper.setup();
    _timestamp_per10ms =_timestamp_per1000ms = _timestamp_per1minute = millis();
    highfreq.start();
}


unsigned long last_exec = 0;
unsigned long worst = 10000;
void SmartAPS::loop()
{
    unsigned long now = millis();
    if (now - _timestamp_per10ms >= 9)
    {
        display.clearDisplay();
        terminal.home();
        terminal.printf("This loop: %lu\n", now - last_exec);
        if (now - last_exec > worst)
            worst = now - last_exec;
        terminal.printf("Worst: %lu\n", worst);
        last_exec = now;
        _timestamp_per10ms = now;
        delay(1);
        display.display();
    }
    /*
    if (now - _timestamp_per1000ms > 1000)
    {
        publish_uptime();
        _timestamp_per1000ms = now;
    }
    if (now - _timestamp_per1minute > 60000)
    {
        if (!isnan(sht20.temperature))
            sensor_temperature->publish_state(sht20.temperature);
        if (!isnan(sht20.humidity))
            sensor_humidity->publish_state(sht20.humidity);
        _timestamp_per1minute = now;
    }
    */
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
                worst = 0;
                terminal.printf("S2 click (%dms)\n", event_param);
                break;
            case BUTTON_EVENT_DOWN:
                terminal.printf("S2 down (%dms)\n", event_param);
                beeper.beep2();
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
                terminal.printf("S3 click (%dms)\n", event_param);
                break;
            case BUTTON_EVENT_DOWN:
                terminal.printf("S3 down (%dms)\n", event_param);
                beeper.beep3();
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
    sht20.loop();
}




