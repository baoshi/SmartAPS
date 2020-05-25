#ifndef SHELL_DETAIL_H
#define SHELL_DETAIL_H

#include "shell.h"

typedef enum 
{
    CHANNEL_USB,
    CHANNEL_PORT_A,
    CHANNEL_PORT_B
} channel_t;

typedef enum
{
    FPS_100,
    FPS_10,
    FPS_1
} fps_t;


class DetailShell : public Shell 
{
public:
    DetailShell(SmartAPS* sa);
    virtual ~DetailShell();
    void init(void) override;
    void enter(unsigned long now) override;
    void leave(unsigned long now) override;
    Shell* loop(unsigned long now) override;
private:
    channel_t _channel;
    bool _channel_enabled;
    fps_t _fps;
    unsigned long _timestamp_per_5000ms;
    int16_t _port_a_s, _port_a_b;
    int16_t _port_b_s, _port_b_b;
    int16_t _usb_s, _usb_b;
    float _ah, _wh;
    void start_sampling(void);
    void stop_sampling(void);
    void draw_ui();
friend void sampling_fn(void *);
};


#endif