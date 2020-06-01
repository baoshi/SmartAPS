#ifndef SHT20_H
#define SHT20_H

#include <Wire.h>
#include "esphome.h"
#include "sequencer.h"

using namespace esphome;
using namespace sequencer;

class SHT20 : public Sequencer
{
public:
    SHT20();
    void begin(TwoWire* bus);
    void loop();
    float temperature, humidity;

private:
    bool execute(const action_t* action) override;
    bool _verify_crc(uint16_t data, uint8_t crc);
    TwoWire* _bus;

};

#endif // SHT20
