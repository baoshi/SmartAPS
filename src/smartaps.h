#ifndef SMARTAPS_H
#define SMARTAPS_H


#include "esphome.h"
#include "button.h"


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
    unsigned long _timestamp_per_1s;
    unsigned long _timestamp_per_1m;
    uint64_t _uptime;
    Button s1, s2, s3;
    Sensor *sensor_usb_v, *sensor_usb_c;
    void publish_uptime(void);
    HighFrequencyLoopRequester highfreq;

private:
    portMUX_TYPE _sample_bufer_mux;
    unsigned long _sample_buffer[1000];
    int _sample_buffer_count;
    static void _sample_fn(void *);
    hw_timer_t *_sample_timer;
};


#endif // SMARTAPS_H
