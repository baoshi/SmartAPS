#ifndef SMARTAPS_H
#define SMARTAPS_H


#include "esphome.h"
#include "button.h"
#include "sht20.h"


using namespace esphome;
using namespace esphome::sensor;
using namespace esphome::text_sensor;


class SmartAPS : public Component, public TextSensor
{
public:
    SmartAPS();
    void register_sensors(void);
    void setup() override;
    void loop() override;
    
protected:
    unsigned long _timestamp_per10ms;
    unsigned long _timestamp_per1000ms;
    unsigned long _timestamp_per1minute;
    uint64_t _uptime;
    Button s1, s2, s3;
    Sensor *sensor_usb_v, *sensor_usb_c;
    Sensor *sensor_temperature, *sensor_humidity;
    void publish_uptime(void);
    HighFrequencyLoopRequester highfreq;
};


#endif // SMARTAPS_H
