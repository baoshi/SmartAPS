#include <Arduino.h>
#include "sequencer.h"

using namespace sequencer;


Sequencer::Sequencer() :  
    _running(false),
    _actions(NULL),
    _cur_action(0),
    _cycles(0),
    _cur_cycle(0),
    _cur_action_start_time(0)
{
}


/*
 * return true if the actions are ongoing or not running.
 * return false if action list finished.
 */
bool Sequencer::loop(void)
{
    bool ret = true;
    if (_running)
    {
        unsigned long now = millis();
        if (_cur_action == -1)
        {
            // Startup 
            _cur_action = 0;
            _cur_action_start_time = now;
            _cur_cycle = 0;
            ret = execute(&(_actions->at(0)));
            if (ret == false)
            {
                // startup failed
                _running = false;
            }
        }
        else 
        {
            // 2nd time onwards into loop
            const action_t* act = &(_actions->at(_cur_action));
            if (now - _cur_action_start_time >= act->duration)
            {
                // proceed to next action, or restart, or finish
                int len = _actions->size();
                ++_cur_action;
                if (_cur_action >= len)
                {
                    // Reach the end
                    ++_cur_cycle;
                    if ((_cycles != 0) && (_cur_cycle >= _cycles))
                    {
                        _running = false;
                        ret = false;    // finished
                    }
                    else
                    {
                        _cur_action = 0; // restart
                    }
                }
                if (ret)
                {
                    _cur_action_start_time = now;
                    ret = execute(&(_actions->at(_cur_action)));
                    if (ret == false)
                    {
                        // execute returns false if failed, abort
                        _running = false;
                    }
                }
            }
            else
            {
                // not finished yet, continue current action
                if (act->repeat)
                    ret = execute(act);
            }
        }
    }
    return ret;
}


bool Sequencer::busy(void)
{
    return (_running);
}


void Sequencer::stop(void)
{
    _running = false;
}


void Sequencer::start(const std::vector<action_t>* acts, int cycles)
{
    int len = acts->size();
    if (len > 0)
    {
        _actions = acts;
        _cur_action = -1;   // To indicate startup
        _cycles = cycles;
        _running = true;
    }
}