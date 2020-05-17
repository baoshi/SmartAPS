#ifndef SHELL_H
#define SHELL_H

class SmartAPS;

class Shell
{
public:
    Shell(SmartAPS* sa) : _sa(sa) {};
    virtual ~Shell() {};
    virtual void init(void);
    virtual void enter(unsigned long now);
    virtual void leave(unsigned long now);
    virtual Shell* loop(unsigned long now);

protected:
    SmartAPS* _sa;
};


#endif