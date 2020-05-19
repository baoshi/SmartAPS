#include <Arduino.h>
#include "temt6000.h"


TEMT6000::TEMT6000()
{
}


void TEMT6000::init(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, INPUT);
    analogSetPinAttenuation(pin, ADC_0db);  // max 1.1V -> 110uA -> 220lx
}


float TEMT6000::read(void)
{
    int16_t v = analogRead(_pin);
    float lx = (v *  0.0537f); //   ( / 4095.0 * 1.1  / 10000 * 2000000 )
    return lx;
}