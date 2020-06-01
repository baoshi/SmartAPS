#ifndef BEEPER_H
#define BEEPER_H

#include "esphome.h"
#include "sequencer.h"

using namespace esphome;
using namespace sequencer;

class Beeper : public Sequencer
{
public:
    Beeper();
    void begin(void);
    void stop(void);
    void loop(void);
    void beep1(void);
    void beep2(void);
    void beep3(void);
private:
    bool execute(const action_t* action) override;
    HighFrequencyLoopRequester _highfreq;
    bool _highfreq_requested;
};

#endif // BEEPER_H

