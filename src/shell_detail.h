#ifndef SHELL_DETAIL_H
#define SHELL_DETAIL_H

#include "shell.h"

typedef enum 
{
    CHANNEL_PORT_A,
    CHANNEL_PORT_B,
    CHANNEL_USB
} channel_t;


void sampling_fn(void *);

class DetailShell : public Shell 
{
public:
    DetailShell(SmartAPS* sa);
    virtual ~DetailShell();
    void init(void) override;
    void enter(unsigned long now) override;
    void leave(unsigned long now) override;
    Shell* loop(unsigned long now);
private:
    channel_t _channel;
    unsigned long _timestamp_light_off;
    int16_t _port_a_s, _port_a_b;
    int16_t _port_b_s, _port_b_b;
    int16_t _usb_s, _usb_b;
    void draw_ui();
friend void sampling_fn(void *);
};


#endif