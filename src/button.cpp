#include <Arduino.h>
#include "Button.h"


enum 
{
    STATE_IDLE,
    STATE_INIT,
    STATE_WAIT_FOR_RELEASE,
    STATE_UP,
    STATE_DOWN_DEBOUNCE,
    STATE_DOWN,
    STATE_HOLD_START,
    STATE_HOLD_END_DEBOUNCE,
    STATE_UP_DEBOUNCE,
};


#define DEBOUNCE_MILLIS 10
#define MIN_HOLDING_MILLIS 1000


Button::Button()
{
    _pressed = LOW;
    _released = HIGH;
    _state = STATE_IDLE;
    _debounce_time = DEBOUNCE_MILLIS;
    _min_hold_time = MIN_HOLDING_MILLIS;
    _time_down = _time_up = 0;
}


Button::~Button()
{
}


void Button::init(uint8_t pin, uint8_t active, uint8_t mode)
{
    _pin = pin;
    pinMode(_pin, mode);
    if (active == LOW)
    {
        _pressed = LOW;
        _released = HIGH;
    }
    else
    {
        _pressed = HIGH;
        _released = LOW;
    }
    _state = STATE_IDLE;
    _debounce_time = DEBOUNCE_MILLIS;
    _min_hold_time = MIN_HOLDING_MILLIS;
}


void Button::begin(bool wait_for_release)
{
    int current = digitalRead(_pin);
    _time_down = 0;
    _time_up = 0;
    /* If wait_for_release is true and button is pressed when this function is called,
     * we will wait for button release first then treat next press-release as first click.
     * Otherwise, first release will be treated as click.
     */
    if (current == _pressed)
    {
        if (wait_for_release) 
        {
            _state = STATE_WAIT_FOR_RELEASE;
        }
        else
        {
            _state = STATE_DOWN_DEBOUNCE;
            _time_down = millis();
        }
    }
    else
    {
        _state = STATE_UP;
    }
}


uint32_t Button::update(unsigned long now)
{
    uint32_t result = BUTTON_EVENT_NONE;
    if (now == 0)
        now = millis();
    int current = digitalRead(_pin);
    switch (_state)
    {
        case STATE_IDLE:
            break;
        case STATE_WAIT_FOR_RELEASE:
            if (current == _released)
                _state = STATE_UP;
            break;
        case STATE_UP:
            if (current == _pressed)
            {
                _state = STATE_DOWN_DEBOUNCE;
                _time_down = now;
            }
            break;
        case STATE_DOWN_DEBOUNCE:
            if (current == _released)
            {
                // If button up, go back to UP state (bouncing)
                _state = STATE_UP;
            }
            else
            {
                if (now - _time_down > _debounce_time)
                {
                    result = BUTTON_EVENT_DOWN | ((now - _time_down) << 4);
                    _state = STATE_DOWN;
                }
            }
            break;
        case STATE_DOWN:
            if (current == _released) // Released
            {
                _state = STATE_UP_DEBOUNCE;
                _time_up = now;
            }
            else
            {
                if (now - _time_down > _min_hold_time)
                {
                    // button was pressed long enough
                    result = BUTTON_EVENT_HOLD_START | ((now - _time_down) << 4);
                    _state = STATE_HOLD_START;
                }
            }
            break;
        case STATE_UP_DEBOUNCE:
            if (current == _pressed)
            {
                // Bouncing, go back to DOWN state (bouncing)
                _state = STATE_DOWN;
            }
            else
            {
                if (now - _time_up > _debounce_time)
                {
                    result = BUTTON_EVENT_CLICK | ((now - _time_down) << 4);
                    _state = STATE_UP;
                }
            }
            break;
        case STATE_HOLD_START:
            if (current == _released)
            {
                _state = STATE_HOLD_END_DEBOUNCE;
                _time_up = now;
            }
            break;
        case STATE_HOLD_END_DEBOUNCE:
            if (current == _pressed)
            {
                // Bouncing, go back to HOLD DOWN state (bouncing)
                _state = STATE_HOLD_START;
            }
            else
            {
                if (now - _time_up > _debounce_time)
                {
                    result = BUTTON_EVENT_HOLD_END | ((now - _time_down) << 4);
                    // restart
                    _state = STATE_UP;
                }
            }
            break;
    }
    return result;
}

