#ifndef SSD1322_H
#define SSD1322_H


#include <Adafruit_GFX.h>
#include <SPI.h>


class SSD1322 : public Adafruit_GFX {
public:
    SSD1322();
    ~SSD1322(void);
    bool init(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc, int8_t rst = -1);
    void display(void);
    void clearDisplay(uint16_t color = 0);
    void setContrast(uint8_t contrast);
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void scrollUp(int16_t lines);
    void reset(void);
    void on(void);
    void off(void);

private:
    void _command(uint8_t cmd);
    void _data(uint8_t data);
    void _data(uint8_t* buf, uint32_t len);
    uint8_t _cs, _dc, _rst;
    uint8_t *_buf;
};


#endif