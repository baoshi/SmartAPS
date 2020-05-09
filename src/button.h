#ifndef BUTTON_H
#define BUTTON_H


/* Usage
 *
 * Button s;
 * s.init(...)
 * s.begin(...)
 * 
 * uint32_t e;
 * e = s.update();
 * uint32_t event_id = BUTTON_EVENT_ID(e);
 * uint32_t event_param = BUTTON_EVENT_PARAM(e);
 * 
 *
 * Event Parameters:
 * BUTTON_EVENT_DOWN: Time between button 1st down to deboucne end (ms)
 * BUTTON_EVENT_HOLD_START: Time between button down to current (ms)
 * BUTTON_EVENT_CLICK: Time between button down to click registered (ms)
 * BUTTON_EVENT_HOLD_END: Total holding time (ms)
 */

#define BUTTON_EVENT_NONE           0x00000000UL
#define BUTTON_EVENT_DOWN           0x00000001UL
#define BUTTON_EVENT_CLICK          0x00000002UL
#define BUTTON_EVENT_HOLD_START     0x00000003UL
#define BUTTON_EVENT_HOLD_END       0x00000004UL

#define BUTTON_EVENT_ID(X)          ((uint32_t)(X & 0x0000000FUL))
#define BUTTON_EVENT_PARAM(X)       ((uint32_t)(X >> 4))



class Button
{
public:
    Button();
    virtual ~Button();
    void init(uint8_t pin, uint8_t active = LOW, uint8_t mode = INPUT_PULLUP);
    void begin(bool wait_for_release = false);
    uint32_t update(unsigned long now = 0);
private:
    uint8_t _pin;
    int _pressed, _released;
    int _state;
    int _debounce_time;
    int _min_hold_time;
    unsigned long _time_down;
    unsigned long _time_up;
};


#endif