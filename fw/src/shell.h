#ifndef SHELL_H
#define SHELL_H

class SmartAPS;

class Shell
{
public:
    Shell(SmartAPS* sa) : _sa(sa) {};
    virtual ~Shell() {};
    virtual void init(void) = 0;
    virtual void enter(unsigned long now) = 0;
    virtual void leave(unsigned long now) = 0;
    virtual Shell* loop(unsigned long now) = 0;

protected:
    SmartAPS* _sa;
};


#endif