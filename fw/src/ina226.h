#ifndef INA226_H
#define INA226_H

#include <Wire.h>

typedef enum 
{
    SAMPLE_MODE_10MS_TRIGGERED,
    SAMPLE_MODE_10MS_CONTINUOUS,
    SAMPLE_MODE_300MS_CONTINUOUS
} ina226_sample_mode_t;

class INA226
{
public:
    INA226();
    void init(TwoWire* bus, uint8_t addr);
    void reset(void);
    void start(ina226_sample_mode_t mode);
    bool read(int16_t& shunt, int16_t& bus);
    int16_t cache_shunt, cache_bus;
private:
    TwoWire* _bus;
    uint8_t _addr;
    bool read16(uint8_t reg, int16_t& result);
    void write16(uint8_t reg, uint16_t val);
};

#endif // INA226_H
