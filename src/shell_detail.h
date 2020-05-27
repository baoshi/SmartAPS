#ifndef SHELL_DETAIL_H
#define SHELL_DETAIL_H

#include "shell.h"
#include "ringbuf.h"

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
    unsigned long _timestamp_back_to_overview;
    int16_t _port_a_s, _port_a_b;
    int16_t _port_b_s, _port_b_b;
    int16_t _usb_s, _usb_b;
    float _ah, _wh;
    static const int _waveform_length = 200;
    RingBuffer<int16_t> _waveform_s;
    RingBuffer<int16_t> _waveform_b;
    HighFrequencyLoopRequester _highfreq;
    void start_sampling(void);
    void stop_sampling(void);
    void draw_ui();
friend void sampling_fn(void *);
    int _count, _max_count;
};


#endif