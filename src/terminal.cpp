#include "terminal.h"

#define FONT_WIDTH  5    // Adafruit default 5x7 font
#define FONT_HEIGHT 7
#define CHAR_SPACE  1
#define LINE_SPACE  8
#define TEXT_COLOR  0x0F
#define BACK_COLOR  0x00

Terminal::Terminal() :
    _tx(0),
    _ty(0),
    _display(NULL)
{
}


void Terminal::begin(SSD1322* display)
{
    _display = display;
    _display->clearDisplay();
    _display->setFont(NULL);    // Use adafruit default font
    home();
}


void Terminal::home()
{
    _tx = 0; _ty = 0;
    _display->setCursor(0, 0);
}


size_t Terminal::write(uint8_t c)
{
    if (c == '\n')
    {
        _tx = 0;
        _ty += LINE_SPACE;
    }
    else if (c == '\r')
    {
        _tx = 0;
    }
    else
    {
        if (_ty > (_display->height() - FONT_HEIGHT))
        {
            _display->scrollUp(LINE_SPACE);
            _ty -= LINE_SPACE;
        }
        _display->drawChar(_tx, _ty, c, TEXT_COLOR, BACK_COLOR, 1);
        _tx += FONT_WIDTH + CHAR_SPACE;
    }
    return 1;
}


size_t Terminal::write(const uint8_t *buffer, size_t size) 
{
    size_t cnt = 0;
    while (size > 0)
    {
        cnt += write(*buffer++);
        size--;
    }
    return cnt;
}
