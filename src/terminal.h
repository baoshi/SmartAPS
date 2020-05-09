#ifndef TERMINAL_H
#define TERMINAL_H


#include "ssd1322.h"


class Terminal : public Print {
public:
    Terminal();
    void begin(SSD1322* display);
    void home(void);
    size_t write(uint8_t v) override;
    size_t write(const uint8_t *buffer, size_t size) override;

private:
    int16_t  _tx, _ty;       // current position for the print command
    SSD1322* _display;
};



#endif