#ifndef INA226_H
#define INA226_H

class INA226
{
public:
    INA226(uint8_t addr);
    void start(void);
    bool read(int16_t shunt, int16_t bus);
private:
    uint8_t _addr;
};

#endif // INA226_H
