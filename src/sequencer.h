#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <vector>

namespace sequencer
{

typedef struct
{
    uint32_t    opcode;         // Operation code for execute()
    uint32_t    duration;       // Duration for this action
    bool        repeat;         // true of this action need to be repeatedly called
} action_t;


class Sequencer
{
public:
    Sequencer();
    bool loop(void);
    bool busy(void);
    void stop(void);
private:
    bool _running;
    const std::vector<action_t>* _actions;
    int _cur_action;
    int _cycles;
    int _cur_cycle;
    unsigned long _cur_action_start_time;
    virtual bool execute(const action_t* action) = 0;
protected:    
    void start(const std::vector<action_t>* actions, int cycles);
};

}

#endif