#include <Arduino.h>
#include "beeper.h"

using namespace esphome;
using namespace sequencer;

#define BEEPER_GPIO 27
#define BEEPER_LEDC_CHANNEL 0


std::vector<action_t> _beep_pattenr_1 =
{
    { 0x008007D0, 100, false },  // 2000Hz, 50% duty, 100ms
    { 0x00000000,   0, false }
};


std::vector<action_t> _beep_pattenr_2 =
{
    { 0x008003E8, 100, false },  // 1000Hz, 50% duty, 100ms
    { 0x008005DC, 100, false },  // 1500Hz, 50% duty, 100ms
    { 0x00000000,   0, false }
};


std::vector<action_t> _beep_pattenr_3 =
{
    { 0x008005DC, 100, false },  // 1500Hz, 50% duty, 100ms
    { 0x008003E8, 100, false },  // 1000Hz, 50% duty, 100ms
    { 0x00000000,   0, false }
};



Beeper::Beeper()
{
    _highfreq_requested = false;
}


void Beeper::begin(void)
{
    ledcSetup(BEEPER_LEDC_CHANNEL, 3000, 8);    // Set at 3000Hz 8Bit  as initial
    ledcAttachPin(BEEPER_GPIO, BEEPER_LEDC_CHANNEL);
}


void Beeper::stop(void)
{
    ledcWrite(BEEPER_LEDC_CHANNEL, 0);
    if (_highfreq_requested)
    {
        _highfreq_requested = false;
        _highfreq.stop();
    }
    Sequencer::stop();
}


void Beeper::loop(void)
{
    // Sequencer::loop return false if sequence finished
    if (Sequencer::loop() == false)
    {
        _highfreq_requested = false;
        _highfreq.stop();
    }
}


bool Beeper::execute(const action_t* action)
{
    // Opcode is 0x00DDFFFF  FFFF-> Frequency, DD -> Duty
    if (action->opcode != 0)
    {
        int freq = action->opcode & 0xFFFF;
        int duty = (action->opcode >> 16) & 0xFF;
        ledcWriteTone(BEEPER_LEDC_CHANNEL, freq);
        ledcWrite(BEEPER_LEDC_CHANNEL, duty);
    }
    else
    {
        // stop
        ledcWrite(BEEPER_LEDC_CHANNEL, 0);
    }
    return true;
}


void Beeper::beep1(void)
{
    if (!_highfreq_requested)
    {
        _highfreq.start();
        _highfreq_requested = true;
    }
    start(&_beep_pattenr_1, 1);
}


void Beeper::beep2(void)
{
    if (!_highfreq_requested)
    {
        _highfreq.start();
        _highfreq_requested = true;
    }
    start(&_beep_pattenr_2, 1);
}


void Beeper::beep3(void)
{
    if (!_highfreq_requested)
    {
        _highfreq.start();
        _highfreq_requested = true;
    }
    start(&_beep_pattenr_3, 1);
}