#ifndef SHELL_OVERVIEW_H
#define SHELL_OVERVIEW_H

#include "shell.h"


class OverviewShell : public Shell 
{
public:
    OverviewShell(SmartAPS* sa);
    virtual ~OverviewShell();
    void init(void) override;
    void enter(unsigned long now) override;
    void leave(unsigned long now) override;
    Shell* loop(unsigned long now);

private:
    unsigned long _timestamp_per_500ms, _timestamp_per_5000ms;
    bool _port_a_state, _port_b_state, _usb_state;
    bool _display_on;
    unsigned long _timestamp_light_off;
    unsigned long _timestamp_exit_screensaver;
    int16_t _port_a_s, _port_a_b;
    int16_t _port_b_s, _port_b_b;
    int16_t _usb_s, _usb_b;
    void draw_ui(void);
    void draw_screensaver(unsigned long now, bool startup = false);
};


#endif