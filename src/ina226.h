#ifndef INA226_H
#define INA226_H

#include <Wire.h>


class INA226
{
public:
    INA226();
    void init(TwoWire* bus, uint8_t addr);
    void start(uint8_t mode);
    bool read(int16_t& shunt, int16_t& bus);
private:
    TwoWire* _bus;
    uint8_t _addr;
    bool read16(uint8_t reg, int16_t& result);
    void write16(uint8_t reg, uint16_t val);
};

#endif // INA226_H
