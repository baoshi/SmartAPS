#ifndef SHELL_SCREENSAVER_H
#define SHELL_SCREENSAVER_H

#include "shell.h"


class ScreenSaverShell : public Shell 
{
public:
    ScreenSaverShell(SmartAPS* sa);
    virtual ~ScreenSaverShell();
    void init(void) override;
    void enter(unsigned long now) override;
    void leave(unsigned long now) override;
    Shell* loop(unsigned long now) override;

private:
    unsigned long _timestamp_per_40ms, _timestamp_per_500ms, _timestamp_per_5000ms;
    bool _port_a_state, _port_b_state, _usb_state;
    bool _display_on;
    unsigned long _timestamp_light_off;
    unsigned long _timestamp_screensaver_off;
    int16_t _port_a_s, _port_a_b;
    int16_t _port_b_s, _port_b_b;
    int16_t _usb_s, _usb_b;
    int _frame, _item;
    int16_t _x, _y, _vx, _vy;
    uint16_t _w, _h;
    const static int _str_len = 20;
    char _str[_str_len + 1];
    void _draw_ui(void);
};


#endif