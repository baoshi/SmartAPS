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
    unsigned long _timestamp_per_1s;    
};


#endif