#ifndef SHELL_DETAIL_H
#define SHELL_DETAIL_H

#include "shell.h"

typedef enum 
{
    CHANNEL_PORT_A,
    CHANNEL_PORT_B,
    CHANNEL_USB
} channel_t;

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
};


#endif