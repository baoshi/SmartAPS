#ifndef TEMT6000_H
#define TEMT6000_H



class TEMT6000
{
public:
    TEMT6000();
    void init(uint8_t pin);
    float read(void);
private:
    uint8_t _pin;
};


#endif