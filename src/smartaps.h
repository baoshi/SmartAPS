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
    portMUX_TYPE _sample_buffer_mux;
    const static int _sample_buffer_length = 64;
    int16_t _sample_buffer_v[_sample_buffer_length];
    int16_t _sample_buffer_c[_sample_buffer_length];
    int _sample_count;
    static void _sample_fn(void *);
    hw_timer_t *_sample_timer;
};


#endif // SMARTAPS_H
