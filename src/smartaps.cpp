#include <Arduino.h>
#include <Wire.h>
#include "smartaps.h"


using namespace esphome;
using namespace esphome::sensor;


static const char *TAG = "smartaps";

SmartAPS::SmartAPS() :
    shell_overview(this),
    shell_screensaver(this),
    shell_detail(this)
{
    sensor_usb_v = new Sensor();
    sensor_usb_c = new Sensor();
    sensor_port_a_v = new Sensor();
    sensor_port_a_c = new Sensor();
    sensor_port_b_v = new Sensor();
    sensor_port_b_c = new Sensor();
    wh_port_a = wh_port_b = wh_usb = 0.0f;
    ah_port_a = ah_port_b = ah_usb = 0.0f;
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
    sensor_usb_v->set_icon("mdi:alpha-v-circle");
    sensor_usb_v->set_force_update(false);
    // INA226 USB Current
    App.register_sensor(sensor_usb_c);
    sensor_usb_c->set_name("USB Current");
    sensor_usb_c->set_unit_of_measurement("A");
    sensor_usb_c->set_accuracy_decimals(1);
    sensor_usb_c->set_icon("mdi:alpha-a-circle");
    sensor_usb_c->set_force_update(false);
    // OUT A Voltage
    App.register_sensor(sensor_port_a_v);
    sensor_port_a_v->set_name("Port A Voltage");
    sensor_port_a_v->set_unit_of_measurement("V");
    sensor_port_a_v->set_accuracy_decimals(1);
    sensor_port_a_v->set_icon("mdi:alpha-v-circle");
    sensor_port_a_v->set_force_update(false);
    // OUT A Current
    App.register_sensor(sensor_port_a_c);
    sensor_port_a_c->set_name("Port A Current");
    sensor_port_a_c->set_unit_of_measurement("A");
    sensor_port_a_c->set_accuracy_decimals(1);
    sensor_port_a_c->set_icon("mdi:alpha-a-circle");
    sensor_port_a_c->set_force_update(false);
    // OUT B Voltage
    App.register_sensor(sensor_port_b_v);
    sensor_port_b_v->set_name("Port B Voltage");
    sensor_port_b_v->set_unit_of_measurement("V");
    sensor_port_b_v->set_accuracy_decimals(1);
    sensor_port_b_v->set_icon("mdi:alpha-v-circle");
    sensor_port_b_v->set_force_update(false);
    // OUT B Current
    App.register_sensor(sensor_port_b_c);
    sensor_port_b_c->set_name("Port B Current");
    sensor_port_b_c->set_unit_of_measurement("A");
    sensor_port_b_c->set_accuracy_decimals(1);
    sensor_port_b_c->set_icon("mdi:alpha-a-circle");
    sensor_port_b_c->set_force_update(false);
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
    beeper.beep1();
    // INA226
    Wire.begin(SDA_PIN, SCL_PIN, 400000);
    ina226_port_a.init(&Wire, INA226_ADDR_PORT_A);
    ina226_port_a.reset();
    ina226_port_b.init(&Wire, INA226_ADDR_PORT_B);
    ina226_port_b.reset();
    ina226_usb.init(&Wire, INA226_ADDR_USB);
    ina226_usb.reset();
    // TEMT6000
    temt6000.init(GPIO_TEMT6000);
    // Button        
    sw1.begin(false);
    sw2.begin(false);
    sw3.begin(false);
    // Timestamps
    _timestamp_per_60s = millis();
    // Shells
    shell_overview.init();
    shell_detail.init();
    _cur_shell = NULL;
    _next_shell = &shell_overview;
    oled.clearDisplay();
    oled.display();
}


void SmartAPS::loop()
{
    unsigned long now = millis();
    if (now - _timestamp_per_60s > 60000)
    {
        _publish_uptime();
        _timestamp_per_60s = now;
    }
    if (_next_shell != _cur_shell)
    {
        if (_cur_shell != NULL) _cur_shell->leave(now);
        if (_next_shell != NULL) _next_shell->enter(now);
        _cur_shell = _next_shell;
    }
    _next_shell = _cur_shell->loop(now);
    beeper.loop();
}