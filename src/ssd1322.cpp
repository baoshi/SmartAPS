#include <Arduino.h>
#include "ssd1322.h"



#define SSD1322_WIDTH   256
#define SSD1322_HEIGHT  64
#define SSD1322_BUF_BYTE_WIDTH (SSD1322_WIDTH / 2)
#define SSD1322_BUF_LEN (SSD1322_BUF_BYTE_WIDTH * SSD1322_HEIGHT)
#define SSD1322_FREQ    20000000



SSD1322::SSD1322(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc, int8_t rst) : 
    Adafruit_GFX(SSD1322_WIDTH, SSD1322_HEIGHT),
    _mosi(mosi),
    _sclk(sclk),
    _cs(cs),
    _dc(dc),
    _rst(rst)
{
    _buf = NULL;
}


SSD1322::~SSD1322(void)
{
    if (_buf)
    {
        free(_buf);
        _buf = NULL;
    }
}


void SSD1322::_command(uint8_t cmd)
{
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    SPI.write(cmd);
    digitalWrite(_cs, HIGH);
}


void SSD1322::_data(uint8_t data)
{
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.write(data);
    digitalWrite(_cs, HIGH);
}

void SSD1322::_data(uint8_t* buf, uint32_t len)
{
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.writeBytes(buf, len);
    digitalWrite(_cs, HIGH);
}


bool SSD1322::begin()
{
    if (!_buf)
    {
        _buf = (uint8_t *)malloc(SSD1322_BUF_LEN);
        if (!_buf)
            return false;
    }
    
    SPI.begin(_sclk, -1, _mosi);
    SPI.setFrequency(SSD1322_FREQ);
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    pinMode(_dc, OUTPUT);
    if (_rst != -1)
    {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(1);   // Datasheet 8.9. Set wait time at least 1ms for internal VDD become stable
        digitalWrite(_rst, LOW);
        delay(1);   // Datasheet 8.9, RES# pin LOW for at least 100us
        digitalWrite(_rst, HIGH);
    }
    // Begin initialization sequence
    _command(0xfd); _data(0x12);    // Unlock MCU interface
    _command(0xae);                 // Turn off display
    _command(0xb3); _data(0x91);    // Clock divider
    _command(0xca); _data(0x3f);    // Multiplex (64 rows)
    _command(0xa2); _data(0x00);    // Display offset (0)
    _command(0xa1); _data(0x00);    // Display start line (0)
    _command(0xab); _data(0x01);    // Use internal VDD
    _command(0xa0); _data(0x14); _data(0x11);   // Remap / Dual COM mode
    _command(0xc7); _data(0x0f);    // Master current control
    _command(0xc1); _data(0x7f);    // Contrast
    _command(0xb1); _data(0xe2);    // Phase 1 2x2 DCLKS, Phase 2 14 DCLKS (default 0x74)
    _command(0xbb); _data(0x1f);    // Phase 2 precharge voltage (0.6xVCC)
    _command(0xb6); _data(0x08);    // 2nd precharge period 8 DLCKS
    _command(0xb4); _data(0xa0); _data(0xfd);   // Display enhancement
    _command(0xd1); _data(0xa2); _data(0x20);   // Display enhancement B
    _command(0xbe); _data(0x07);    // Vcomh (0.86xVCC, default 0x04)
    _command(0xb9);                 // Use default linear gray scale
    _command(0xa6);                 // Normal display
    _command(0xa9);                 // Exit partial display mode
    _command(0xaf);                 // Display on
    return true;
}


void SSD1322::display(void)
{
    _command(0x15); _data(0x1c); _data(0x5B);   // Column address
    _command(0x75); _data(0x00); _data(0x3F);   // Row address
    _command(0x5c); // Write RAM
    _data(_buf, SSD1322_BUF_LEN);
}


void SSD1322::clearDisplay(uint16_t color)
{
    uint8_t c;
    c = color & 0x000F;
    c = c | (c << 4);
    if (_buf)
    {
        memset(_buf, c, SSD1322_BUF_LEN);
    }
}


void SSD1322::setContrast(uint8_t contrast)
{
	_command(0xc1);
	_data(contrast);
}


void SSD1322::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= SSD1322_WIDTH) || (y < 0) || (y >= SSD1322_HEIGHT))
        return;
    uint8_t *cell = _buf + (x >> 1) + (y * SSD1322_BUF_BYTE_WIDTH);
    uint8_t value = *cell;
    if (x % 2) // odd column, set low nibble
    {
        value &= 0xF0;
        value |= color;
    }
    else // even column, set high nibble
    {
        value &= 0x0F;
        value |= (color << 4);
    }
    *cell = value;
}


void SSD1322::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Boundary check
    if (y < 0 || y >= SSD1322_HEIGHT)
    {
		return;
	}
	if (x < 0) 
    {
		w += x;
		x = 0;
	}
	if ((x + w) > SSD1322_WIDTH) 
    {
		w = (SSD1322_WIDTH - x);
	}
	if (w <= 0) {
		return;
	}

    uint8_t *cell = _buf + (x >> 1) + (y * SSD1322_BUF_BYTE_WIDTH);
	uint8_t oddval = color;
	uint8_t evenval = (color << 4);
	uint8_t fullval = (color << 4) | color;
	uint8_t total = w / 2;

    //
    // If x starts at even column and w is even number
    //
    // |---  x  ---|--- x+1 ---|   ......   |-- x+w-1 --|--- x+w ---|
    // |__|__|__|__|__|__|__|__|   ......   |__|__|__|__|__|__|__|__|
    //
	if (((x % 2) == 0) && ((w % 2) == 0)) 
	{
		while (total)
		{
			*cell = fullval;
            ++cell;
            --total;
		}
		return;
	}

    //
    // If x starts at odd column and w is odd number
    //
    // |           |---  x  ---|   ......   |-- x+w-1 --|--- x+w ---|
    // |__|__|__|__|__|__|__|__|   ......   |__|__|__|__|__|__|__|__|
    //
	if ((x % 2) && (w % 2)) 
	{
		uint8_t t = (*cell) & 0xF0;
		*cell = t | oddval;
        ++cell; // next cell
		while (total)
		{
			*cell = fullval;
            ++cell;
            --total;
		}
		return;
	}

    //
    // If x starts at even column and w is odd number
    //
    // |---  x  ---|--- x+1 ---|   ......   |--- x+w ---|           |
    // |__|__|__|__|__|__|__|__|   ......   |__|__|__|__|__|__|__|__|
    //
	if (((x % 2) == 0) && (w % 2))
	{
		while (total)
		{
			*cell = fullval;
            ++cell;
            --total;
		}
		uint8_t t = (*cell) & 0x0F;
		*cell = t | evenval;
		return;
	}

    //
    // If x starts at odd column and w is even number
    //
    // |           |---  x  ---|   ......   |--- x+w ---|           |
    // |__|__|__|__|__|__|__|__|   ......   |__|__|__|__|__|__|__|__|
    //
	if ((x % 2) && ((w % 2) == 0))
	{
		uint8_t t = (*cell) & 0xF0;
		*cell = t | oddval;
        ++cell;
        --total;    // remove 2 pixels from total length
		while (total)
		{
			*cell = fullval;
            ++cell;
            --total;
		}
		t = (*cell) & 0x0F;
		*cell++ = t | evenval;
		return;
	}
}


void SSD1322::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Boundary check
	if (x < 0 || x >= SSD1322_WIDTH) 
    {
		return;
	}
	if (y < 0) 
    {
		h += y;
		y = 0;
	}
	if ((y + h) > SSD1322_HEIGHT) 
    {
		h = (SSD1322_HEIGHT - y);
	}
	if (h <= 0) 
    {
		return;
	}
    uint8_t *cell = _buf + (x >> 1) + (y * SSD1322_BUF_BYTE_WIDTH);
    uint8_t mask, setval;
    if (x % 2)
    {
        mask = 0xF0;
        setval = color;
    }
    else
    {
        mask = 0x0F;
        setval = color << 4;
    }
	while (h--)
	{
		uint8_t t = *cell;
        t &= mask;
        t |= setval;
		*cell = t;
        cell += SSD1322_BUF_BYTE_WIDTH;
	};
}


void SSD1322::scrollUp(int16_t lines)
{
    // Move frame buffer up by lines
    uint8_t *src = _buf + lines * SSD1322_BUF_BYTE_WIDTH;
    memcpy(_buf, src, (SSD1322_HEIGHT - lines) * SSD1322_BUF_BYTE_WIDTH);
    src = _buf + (SSD1322_HEIGHT - lines) * SSD1322_BUF_BYTE_WIDTH;
    memset(src, 0, lines * SSD1322_BUF_BYTE_WIDTH);
}