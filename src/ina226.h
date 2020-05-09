#ifndef INA226_H
#define INA226_H

#include "esphome.h"
#include "sequencer.h"

using namespace esphome;
using namespace esphome::sensor;
using namespace esphome::i2c;
using namespace sequencer;

class INA226 : public Component, public I2CDevice, public Sequencer
{
public:
    Sensor *temperature;
    Sensor *humidity;
    INA226();
    void setup() override;
    void loop() override;
    void dump_config() override;
private:
    int _fail_count;
    bool execute(const action_t& action) override;
};

#endif // INA226_H
