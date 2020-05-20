#include <Arduino.h>

#include "ina226.h"


INA226::INA226() :
    _bus(NULL),
    _addr(0)
{
}



bool INA226::read16(uint8_t reg, int16_t& result)
{
    _bus->beginTransmission(_addr);
    _bus->write(reg);
    _bus->endTransmission();
    if (_bus->requestFrom(_addr, (uint8_t)2) == 2)
    {
        uint16_t hh = _bus->read();
        uint8_t ll = _bus->read();
        result = (int16_t)((hh << 8) | ll);
        return true;
    }
    return false;
}


void INA226::write16(uint8_t reg, uint16_t val)
{
    uint8_t ll = (uint8_t)(val & 0x00FF);
    uint8_t hh = (uint8_t)(val >> 8);
    _bus->beginTransmission(_addr);
    _bus->write(reg);
    _bus->write(hh);
    _bus->write(ll);
    _bus->endTransmission();
} 


void INA226::init(TwoWire* bus, uint8_t addr)
{
    cache_shunt = 0;
    cache_bus = 0;
    _bus = bus;
    _addr = addr;
}


void INA226::reset(void)
{
    write16(0x00, 0x8000);
}


void INA226::start(ina226_sample_mode_t mode)
{
    switch (mode)
    {
    case SAMPLE_MODE_10MS_TRIGGERED:
        /*
         * shunt measurement at 1.1ms
         * bus measurement at 1.1ms
         * 4 average
         * triggled
         * total 8.8ms/cycle
         * CFG = 0100 0011 0010 0011
         */
        write16(0x00, 0x4323);
        break;
    case SAMPLE_MODE_300MS_CONTINUOUS:
        /*
         * shunt measurement at 8.244ms
         * bus measurement at 8.244ms
         * 16 average
         * continuous
         * total 264ms/cycle
         * CFG = 0100 0101 1111 1111
         */
        write16(0x00, 0x45FF);
        break;
    }
   
}

bool INA226::read(int16_t& shunt, int16_t& bus)
{
    bool ret;
    ret = read16(0x01, shunt);
    ret &= read16(0x02, bus);
    if (ret)
    {
        cache_shunt = shunt;
        cache_bus = bus;
    }
    return ret;
}

