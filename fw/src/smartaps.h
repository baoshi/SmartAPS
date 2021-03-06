#ifndef SMARTAPS_H
#define SMARTAPS_H


#include "esphome.h"
#include "ssd1322.h"
#include "terminal.h"
#include "button.h"
#include "beeper.h"
#include "ina226.h"
#include "temt6000.h"
#include "shell.h"
#include "shell_overview.h"
#include "shell_screensaver.h"
#include "shell_detail.h"


using namespace esphome;
using namespace esphome::sensor;
using namespace esphome::text_sensor;



#define GPIO_OUT_A          26
#define GPIO_OUT_B          32
#define GPIO_OUT_USB        16

#define GPIO_BUTTON_S1      0
#define GPIO_BUTTON_S2      34
#define GPIO_BUTTON_S3      35

#define SDA_PIN             21
#define SCL_PIN             22
#define INA226_ADDR_USB     0x45
#define INA226_ADDR_PORT_A  0x4a
#define INA226_ADDR_PORT_B  0x41

#define GPIO_TEMT6000       36

#define SHUNT_TO_AMP_USB(s) (s * 0.0001f)   // ( * 2.5u / 0.025)
#define SHUNT_TO_AMP_PORT(s) (s * 0.00025f) // (* 2.5u / 0.01)
#define BUS_TO_V(b) (b * 0.00125f)


class SmartAPS : public Component, public TextSensor
{
public:
    SmartAPS();
    void register_sensors(void);
    void setup() override;
    void loop() override;

public:
    Sensor *sensor_usb_v, *sensor_usb_c;
    Sensor *sensor_port_a_v, *sensor_port_a_c;
    Sensor *sensor_port_b_v, *sensor_port_b_c;
    
    SSD1322 oled;
    Button sw1, sw2, sw3;
    Beeper beeper;
    INA226 ina226_port_a, ina226_port_b, ina226_usb;
    TEMT6000 temt6000;

    float wh_port_a, wh_port_b, wh_usb;
    float ah_port_a, ah_port_b, ah_usb;

    OverviewShell shell_overview;
    ScreenSaverShell shell_screensaver;
    DetailShell shell_detail;

private:    // TextSensor related
    uint64_t _uptime;
    void _publish_uptime(void);
    unsigned long _timestamp_per_60s;

private:
    Shell* _cur_shell;
    Shell* _next_shell;
};


#endif // SMARTAPS_H
