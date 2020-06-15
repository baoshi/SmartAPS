#include "esphome.h"
#include "smartaps.h"
#include "font_dotmatrix_7.h"
#include "shell_screensaver.h"

using namespace esphome;

extern gpio::GPIOSwitch *out_port_a;
extern gpio::GPIOSwitch *out_port_b;
extern gpio::GPIOSwitch *out_usb;

#define LOWLIGHT_THRESHOLD      1       // below this value is considered dark room
#define LIGHT_OFF_TIMER         30000   // turn off display 30 seconds after room go dark
#define FRAMES_PER_ITEM         125     // We display at 25fps. 125 frames is about 5 seconds

static const char *TAG = "screensaver";


/*
 * Screensaver shell
 * Actions:
 * Back to overshell shell if
 * - sw1/sw2/sw3 press
 * - output toggle externally
 * - output current change by 200mA
 *
 * Display bouncing short form of channel status
 * 
 * Turn off display if low light
 * 
 * Publishes:
 * V/A of all channels every  5 second
 */


ScreenSaverShell::ScreenSaverShell(SmartAPS* sa) : Shell(sa)
{
}


ScreenSaverShell::~ScreenSaverShell()
{
}


void ScreenSaverShell::init(void)
{
}


void ScreenSaverShell::enter(unsigned long now)
{
    ESP_LOGD(TAG, "Enter ScreenSaver Shell");
    _port_a_state = id(out_port_a).state;
    _port_b_state = id(out_port_b).state;
    _usb_state = id(out_usb).state;
    _sa->oled.clearDisplay();
    _sa->oled.display();
    _sa->oled.reset();  // force display on
    _display_on = true;
    _timestamp_light_off = 0;
    _sa->sw1.begin();
    _sa->sw2.begin();
    _sa->sw3.begin();
    _sa->ina226_port_a.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_port_b.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_usb.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _port_a_b = _sa->ina226_port_a.cache_bus;
    _port_a_s = _sa->ina226_port_a.cache_shunt;
    _port_b_b = _sa->ina226_port_b.cache_bus;
    _port_b_s = _sa->ina226_port_b.cache_shunt;
    _usb_b = _sa->ina226_usb.cache_bus;
    _usb_s = _sa->ina226_usb.cache_shunt;
    _timestamp_per_40ms = now;
    _timestamp_per_1s = now;
    _timestamp_publish_ha = now;
    _frame = 0; _item = 0;
    _x = _sa->oled.width() / 2;
    _y = _sa->oled.height() / 2;
}


void ScreenSaverShell::leave(unsigned long now)
{
    _sa->ina226_port_a.reset();
    _sa->ina226_port_b.reset();
    _sa->ina226_usb.reset();
    ESP_LOGD(TAG, "Leave ScreenSaver Shell");
}


Shell* ScreenSaverShell::loop(unsigned long now)
{
    uint32_t e1 = _sa->sw1.update(now);
    uint32_t e2 = _sa->sw2.update(now);
    uint32_t e3 = _sa->sw3.update(now);
    // Any key press causes return to overview shell
    if ((BUTTON_EVENT_ID(e1) == BUTTON_EVENT_CLICK)
        ||
        (BUTTON_EVENT_ID(e2) == BUTTON_EVENT_CLICK)
        ||
        (BUTTON_EVENT_ID(e3) == BUTTON_EVENT_CLICK))
    {
        return (&(_sa->shell_overview));
    }
    // Any output toggle causes return to overview shell
    if ((id(out_port_a).state != _port_a_state)
        ||
        (id(out_port_b).state != _port_b_state)
        ||
        (id(out_usb).state != _usb_state))
    {
        return (&(_sa->shell_overview));
    }
    // Measure ambient light
    float lx = _sa->temt6000.read();
    if (lx > LOWLIGHT_THRESHOLD)
    {
        // Room is bright
        if (!_display_on)
        {
            // turn on display
            ESP_LOGD(TAG, "Light = %d lux", (int)lx);
            ESP_LOGI(TAG, "Turning on OLED");
            _sa->oled.on();
            _display_on = true;
            _timestamp_light_off = 0;  // not counting low light
        }
    }
    else
    {
        // Room is dark
        if (_display_on)
        {
            if (_timestamp_light_off == 0)
            {
                _timestamp_light_off = now;     
            }
            else
            {
                if (now - _timestamp_light_off > LIGHT_OFF_TIMER)
                {
                    ESP_LOGD(TAG, "Light = %d lux", (int)lx);
                    ESP_LOGI(TAG, "Turning off OLED");
                    _sa->oled.off();
                    _display_on = false;
                }
            }
        }
    }
    // Update UI animation every 40ms
    if (now - _timestamp_per_40ms > 40)
    {
        if (_display_on)
            _draw_ui();
        _timestamp_per_40ms = now;
    }
    // Measurement every 1s
    if (now - _timestamp_per_1s > 1000)
    {
        // collect samples, 
        int16_t s, b, d_u, d_a, d_b;
        _sa->ina226_usb.read(s, b);
        d_u = s - _usb_s; d_u = d_u > 0 ? d_u : -d_u;
        _usb_s = s; _usb_b = b;
        _sa->ina226_port_a.read(s, b);
        d_a = s - _port_a_s; d_a = d_a > 0 ? d_a : -d_a;
        _port_a_s = s; _port_a_b = b;
        _sa->ina226_port_b.read(s, b);
        d_b = s - _port_b_s; d_b = d_b > 0 ? d_b : -d_b;
        _port_b_s = s; _port_b_b = b;
        // accumulate AH/WH
        float v, c;
        v = _usb_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _usb_s / 10000.0f;  // ( * 2.5u / 0.025 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_usb += c / 3600.0f;
        _sa->wh_usb += (c * v) / 3600.0f;
        v = _port_a_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_port_a += c / 3600.0f;
        _sa->wh_port_a += (c * v) / 3600.0f;
        v = _port_b_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_port_b += c / 3600.0f;
        _sa->wh_port_b += (c * v) / 3600.0f;
        // Compare current, if changed (200mA), exit screen saver
        if (d_u > 2000)
        {
            ESP_LOGI(TAG, "USB current changed");
            return (&(_sa->shell_overview));
        }
        if (d_a > 800)
        {
            ESP_LOGI(TAG, "Port A current changed");
            return (&(_sa->shell_overview));
        }
        
        if (d_b > 800)
        {
            ESP_LOGI(TAG, "Port B current changed");
            return (&(_sa->shell_overview));
        }
        _timestamp_per_1s = now;
    }
    // Publish reading every 60 seconds
    if (now - _timestamp_publish_ha > 60000)
    {
        float v, c;
        // USB data
        v = _usb_b * 0.00125f;  // 1.25mV/lsb
        c = _usb_s / 10000.0f;  // ( * 2.5u / 0.025 )
        _sa->sensor_usb_v->publish_state(v);
        _sa->sensor_usb_c->publish_state(c);
        // Port A data
        v = _port_a_b * 0.00125f;  // 1.25mV/lsb
        c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
        _sa->sensor_port_a_v->publish_state(v);
        _sa->sensor_port_a_c->publish_state(c);
        // Port B data
        v = _port_b_b * 0.00125f;  // 1.25mV/lsb
        c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
        _sa->sensor_port_b_v->publish_state(v);
        _sa->sensor_port_b_c->publish_state(c);
        _timestamp_publish_ha = now;
    }
    return this;
}


void ScreenSaverShell::_draw_ui(void)
{
    float v, c;
    if (_frame == 0)
    {
        // initial frame, prepare text, velocity, etc
        switch (_item)
        {
        case 0:
            v = _usb_b * 0.00125f; // 1.25mV/lsb
            c = _usb_s / 10000.0f; // ( * 2.5u / 0.025 )
            if (c < 0)
                c = 0.0f;
            snprintf(_str, _str_len, "U:%.1fV %.1fA", v, c);
            break;
        case 1:
            v = _port_a_b * 0.00125f; // 1.25mV/lsb
            c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
            if (c < 0)
                c = 0.0f;
            snprintf(_str, _str_len, "A:%.1fV %.1fA", v, c);
            break;
        case 2:
            v = _port_b_b * 0.00125f; // 1.25mV/lsb
            c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
            if (c < 0)
                c = 0.0f;
            snprintf(_str, _str_len, "B:%.1fV %.1fA", v, c);
            break;
        }
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        int16_t x1, y1;
        _sa->oled.getTextBounds(_str, 0, 0, &x1, &y1, &_w, &_h);
        do { _vx = random(-3, 4); } while (_vx == 0);
        do { _vy = random(-2, 3); } while (_vy == 0);
        ++_frame;
    }
    _x += _vx; _y += _vy;
    if ((_x < 0) || (_x >= _sa->oled.width() - _w))
    {
        _vx = -_vx;
        while ((_x < 0) || (_x >= _sa->oled.width() - _w))
            _x += _vx;
    }
    if ((_y < _h ) || (_y >= _sa->oled.height()))
    {
        _vy = -_vy;
        while ((_y < _h ) || (_y >= _sa->oled.height()))
            _y += _vy;
    }
    _sa->oled.clearDisplay();
    _sa->oled.setCursor(_x, _y);
    _sa->oled.print(_str);
    _sa->oled.display();
    ++_frame;
    if (_frame > FRAMES_PER_ITEM)
    {
        _frame = 0;
        ++_item;
        if (_item > 2)
            _item = 0;
    }
}