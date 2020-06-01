
#include "esphome.h"
#include "smartaps.h"
#include "font_seg7_23.h"
#include "font_symbol_10.h"
#include "font_dotmatrix_7.h"
#include "shell_overview.h"

using namespace esphome;

extern wifi::WiFiComponent *wifi_wificomponent;
extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_port_a;
extern gpio::GPIOSwitch *out_port_b;
extern gpio::GPIOSwitch *out_usb;

#define LOWLIGHT_THRESHOLD      5       // below this value is considered dark room
#define LIGHT_OFF_TIMER         30000   // turn off display 30 seconds after room go dark
#define SCREENSAVER_TIMER       300000  // turn on screen saver if screen stays on for 5 minutes


static const char *TAG = "overview";


/*
 * Overview shell
 * Actions:
 * sw2 switches on/off port A
 * sw3 switches on/off port B
 * sw1 switches to next shell (detailshell_usb)
 *
 * Display every 500ms: 
 * V/A for all channels
 * Time
 * WiFi signal strength
 * 
 * Turn off display if low light
 * Turn on display if
 * - sw1/sw2/sw3 press
 * - output toggle externally
 * - output current change by 200mA
 * 
 * Publishes:
 * V/A of all channels every minute
 */

OverviewShell::OverviewShell(SmartAPS* sa) : Shell(sa)
{
}


OverviewShell::~OverviewShell()
{
}


void OverviewShell::init(void)
{
}


void OverviewShell::enter(unsigned long now)
{
    ESP_LOGD(TAG, "Enter Overview Shell");
    _port_a_state = id(out_port_a).state;
    _port_b_state = id(out_port_b).state;
    _usb_state = id(out_usb).state;
    _sa->oled.clearDisplay();
    _sa->oled.display();
    _sa->oled.reset();  // force display on
    _display_on = true;
    _timestamp_exit_screensaver = now;   // start to count for screen saver
    _timestamp_light_off = 0;
    _sa->sw1.begin(true);
    _sa->sw2.begin(true);
    _sa->sw3.begin(true);
    _sa->ina226_port_a.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_port_b.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_usb.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _port_a_b = _sa->ina226_port_a.cache_bus;
    _port_a_s = _sa->ina226_port_a.cache_shunt;
    _port_b_b = _sa->ina226_port_b.cache_bus;
    _port_b_s = _sa->ina226_port_b.cache_shunt;
    _usb_b = _sa->ina226_usb.cache_bus;
    _usb_s = _sa->ina226_usb.cache_shunt;
    _timestamp_per_500ms = 0;    // The 500ms task will be called immediately when enter loop
    _timestamp_per_1s = now;
    _timestamp_publish_ha = now; // The homeassistant update interval
}


void OverviewShell::leave(unsigned long now)
{
    _sa->ina226_port_a.reset();
    _sa->ina226_port_b.reset();
    _sa->ina226_usb.reset();
    ESP_LOGD(TAG, "Leave Overview Shell");
}


Shell* OverviewShell::loop(unsigned long now)
{
    uint32_t e1 = _sa->sw1.update(now);
    uint32_t e2 = _sa->sw2.update(now);
    uint32_t e3 = _sa->sw3.update(now);
    if (BUTTON_EVENT_ID(e1) == BUTTON_EVENT_CLICK)
    {
        return (&(_sa->shell_detail));
    }
    bool switch_changed = false;
    if (BUTTON_EVENT_ID(e2) == BUTTON_EVENT_CLICK)
    {
        switch_changed = true;
        // If button pressed while display is not on,
        // only turn on display, don't toggle output.
        if (_display_on)
            out_port_a->toggle();
    }
    if (BUTTON_EVENT_ID(e3) == BUTTON_EVENT_CLICK)
    {
        switch_changed = true;
        if (_display_on)
            out_port_b->toggle();
    }
    bool port_changed = false;
    if (id(out_port_a).state != _port_a_state)
    {
        port_changed = true;
        _port_a_state = !_port_a_state;
    }
    if (id(out_port_b).state != _port_b_state)
    {
        port_changed = true;
        _port_b_state = !_port_b_state;
    }
    if (id(out_usb).state != _usb_state)
    {
        port_changed = true;
        _usb_state = !_usb_state;
    }
    if (switch_changed || port_changed)
    {
        if (!_display_on)
        {
            ESP_LOGI(TAG, "Turning on OLED");
            _sa->oled.on();
            _display_on = true;
        }
        _timestamp_light_off = 0;
        _timestamp_exit_screensaver = now;
    }
    // Measure
    if (switch_changed || port_changed || (now - _timestamp_per_500ms > 500))
    {
        // collect samples, compare, if changed (200mA), turn screen on
        int16_t s, b, d;
        _sa->ina226_port_a.read(s, b);
        d = s - _port_a_s; d = d > 0 ? d : -d;
        _port_a_s = s; _port_a_b = b;
        if (d > 800)
        {
            ESP_LOGI(TAG, "Port A current changed");
            if (!_display_on)
            {
                ESP_LOGI(TAG, "Turning on OLED");
                _sa->oled.on();
                _display_on = true;
                // reset the timer here instead outside
                // so changing of current won't keep reset
                // screensaver timer
                _timestamp_light_off = 0;
                _timestamp_exit_screensaver = now;
            }
        }
        _sa->ina226_port_b.read(s, b);
        d = s - _port_b_s; d = d > 0 ? d : -d;
        _port_b_s = s; _port_b_b = b;
        if (d > 800)
        {
            ESP_LOGI(TAG, "Port B current changed");
            if (!_display_on)
            {
                ESP_LOGI(TAG, "Turning on OLED");
                _sa->oled.on();
                _display_on = true;
                _timestamp_light_off = 0;
                _timestamp_exit_screensaver = now;
            }
        }
        _sa->ina226_usb.read(s, b);
        d = s - _usb_s; d = d > 0 ? d : -d;
        _usb_s = s; _usb_b = b;
        if (d > 2000)
        {
            ESP_LOGI(TAG, "USB current changed");
            if (!_display_on)
            {
                ESP_LOGI(TAG, "Turning on OLED");
                _sa->oled.on();
                _display_on = true;
                _timestamp_light_off = 0;
                _timestamp_exit_screensaver = now;
            }
        }
        if (_display_on)
            draw_ui();
        _timestamp_per_500ms = now;
    }
    // Ambient light measurement
    float lx = _sa->temt6000.read();
    if (lx > LOWLIGHT_THRESHOLD)
    {
        // Room is bright
        if (!_display_on)
        {
            // turn on display if it is not
            ESP_LOGD(TAG, "Light = %d lux", (int)lx);
            ESP_LOGI(TAG, "Turning on OLED");
            _sa->oled.on();
            _display_on = true;
            _timestamp_light_off = 0;  // not counting low light
            _timestamp_exit_screensaver = now;
        }
        else 
        {
            // display is already on, count for screensaver
            if (now - _timestamp_exit_screensaver > SCREENSAVER_TIMER)
            {
                return (&(_sa->shell_screensaver));
            }
        }
    }
    else
    {
        // Room is dark
        if (_display_on)
        {
            if (_timestamp_light_off == 0)
            {
                _timestamp_light_off = now; // start counting low light duration
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
    // Every 1 second accumulate AH/WH
    if (now - _timestamp_per_1s > 1000)
    {
        float v, c;
        // USB data
        v = _usb_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _usb_s / 10000.0f;  // ( * 2.5u / 0.025 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_usb += c / 3600.0f;
        _sa->wh_usb += (c * v) / 3600.0f;
        // Port A
        v = _port_a_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_port_a += c / 3600.0f;
        _sa->wh_port_a += (c * v) / 3600.0f;
        // Port B
        v = _port_b_b * 0.00125f;  // 1.25mV/lsb
        if (v < 0.0f) v = 0.0f;
        c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0.0f) c = 0.0f;
        _sa->ah_port_b += c / 3600.0f;
        _sa->wh_port_b += (c * v) / 3600.0f;
        _timestamp_per_1s = now;
    }
    // Publish reading every 60s
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


void OverviewShell::draw_ui(void)
{
    const int buf_len = 64;
    char buf[buf_len + 1];
    int16_t  x1, y1;
    uint16_t w, h;
    float v, c;
    
    _sa->oled.clearDisplay();
    _sa->oled.drawFastHLine(0, 8, 256, 0x0F);
    // USB V/A
    if (id(out_usb).state)
    {
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setCursor(0, 6);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.print("\x81");  // USB symbol
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        v = _usb_b * 0.00125f;  // 1.25mV/lsb
        c = _usb_s / 10000.0f;  // ( * 2.5u / 0.025 )
        snprintf(buf, buf_len, "%.1fV %.1fA", v, c);
        _sa->oled.setCursor(14, 6);
        _sa->oled.print(buf);
    }
    else
    {
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setCursor(0, 6);
        _sa->oled.setTextColor(0x02);
        _sa->oled.print("\x81");  // USB symbol    
    }
    // WiFi Symbol
    if (id(wifi_wificomponent).is_connected())
    {
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setCursor(249, 6);
        _sa->oled.setTextColor(0x02);
        _sa->oled.print("\x85");  // Four bar signal symbol
        int8_t rssi = WiFi.RSSI();
        if (rssi >= -50)
        {
            // 4 bars
            _sa->oled.setCursor(249, 6);
            _sa->oled.setTextColor(0x0F);
            _sa->oled.print("\x85");
        }
        else if (rssi >= -65)
        {
            // 3 bars
            _sa->oled.setCursor(249, 6);
            _sa->oled.setTextColor(0x0F);
            _sa->oled.print("\x84");
        }
        else if (rssi >= -85)
        {
            // 2 bars
            _sa->oled.setCursor(249, 6);
            _sa->oled.setTextColor(0x0F);
            _sa->oled.print("\x83");
        }
        else
        {
            // 1 bar
            _sa->oled.setCursor(249, 6);
            _sa->oled.setTextColor(0x0F);
            _sa->oled.print("\x83");
        }
    }
    else
    {
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setCursor(249, 6);
        _sa->oled.setTextColor(0x02);
        _sa->oled.print("\x86");  // Four bar signal symbol
    }
    if (id(sntp_time).now().is_valid())
    {
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        id(sntp_time).now().strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S");
        _sa->oled.getTextBounds(buf, 0, 6, &x1, &y1, &w, &h);
        _sa->oled.setCursor(246 - w, 6);
        _sa->oled.print(buf);
    }
    // Port A
    if (id(out_port_a).state)
    {
        _sa->oled.setFont(&SEG7_23);
        _sa->oled.setTextColor(0x0F);
        c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
    }
    else
    {
        _sa->oled.setFont(&SEG7_23);
        _sa->oled.setTextColor(0x02);
        c = _port_a_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0)
            c = 0.0f;   // sometime it measures -0.00025;
    }
    // Right align at x = 123
    // voltage y = 36
    // current y = 64
    v = _port_a_b * 0.00125f;  // 1.25mV/lsb
    snprintf(buf, buf_len, "%.2f#", v); // # is symbol V
    _sa->oled.getTextBounds(buf, 0, 36, &x1, &y1, &w, &h);
    _sa->oled.setCursor(123 - w, 36);
    _sa->oled.print(buf);
    if (c >= 0 && c < 10)
        snprintf(buf, buf_len, "%.3f\"", c);    // " is symbol A
    else
        snprintf(buf, buf_len, "%.2f\"", c);
    _sa->oled.getTextBounds(buf, 0, 64, &x1, &y1, &w, &h);
    _sa->oled.setCursor(123 - w, 64);
    _sa->oled.print(buf);
    // Port B
    if (id(out_port_b).state)
    {
        _sa->oled.setFont(&SEG7_23);
        _sa->oled.setTextColor(0x0F);
        c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
    }
    else
    {
        _sa->oled.setFont(&SEG7_23);
        _sa->oled.setTextColor(0x02);
        c = _port_b_s / 4000.0f;  // ( * 2.5u / 0.01 )
        if (c < 0)
            c = 0.0f;
    }
    // ch2 : Right align at x = 255
    // voltage y = 36
    // current y = 64
    v = _port_b_b * 0.00125f;  // 1.25mV/lsb
    snprintf(buf, buf_len, "%.2f#", v); // # is symbol V
    _sa->oled.getTextBounds(buf, 0, 36, &x1, &y1, &w, &h);
    _sa->oled.setCursor(255 - w, 36);
    _sa->oled.print(buf);
    if (c >= 0 && c < 10)
    snprintf(buf, buf_len, "%.3f\"", c);    // " is symbol A
    else
    snprintf(buf, buf_len, "%.2f\"", c);
    _sa->oled.getTextBounds(buf, 0, 64, &x1, &y1, &w, &h);
    _sa->oled.setCursor(255 - w, 64);
    _sa->oled.print(buf);
    // Display
    _sa->oled.display();
}



void OverviewShell::draw_screensaver(unsigned long now, bool startup)
{
    static int saver_frame = 0;   // 0 - Initialize, 1-100 -- Bounce
    static int saver_item = 0;    // 0 - USB, 1 - PortA, 2 - Port B
    static unsigned long saver_ts = now;
    const int str_len = 64;
    static char saver_str[str_len - 1];
    static int saver_x = 128, saver_y = 64, saver_vx, saver_vy;
    static uint16_t saver_w, saver_h;
    int16_t  x1, y1;
    float v, c;
    if (startup)
    {
        // About to enter screen saver, just reset values
        saver_frame = 0;
        saver_item = 0;
        saver_ts = now;
        return;
    }
    if (now - saver_ts > 40)  // about 25fps
    {
        if (saver_frame == 0)
        {
            switch (saver_item)
            {
            case 0:
                _sa->ina226_usb.read(_usb_s, _usb_b);
                v = _usb_b * 0.00125f;  // 1.25mV/lsb
                c = _usb_s / 10000.0f;  // ( * 2.5u / 0.025 )
                if (c < 0) c = 0.0f;
                snprintf(saver_str, str_len, "U:%.1fV %.1fA", v, c);
                break;
            case 1:
                _sa->ina226_port_a.read(_port_a_s, _port_a_b);
                v = _port_a_b * 0.00125f;   // 1.25mV/lsb
                c = _port_a_s / 4000.0f;    // ( * 2.5u / 0.01 )
                if (c < 0) c = 0.0f;
                snprintf(saver_str, str_len, "A:%.1fV %.1fA", v, c);
                break;
            case 2:
                _sa->ina226_port_b.read(_port_b_s, _port_b_b);
                v = _port_b_b * 0.00125f;   // 1.25mV/lsb
                c = _port_b_s / 4000.0f;    // ( * 2.5u / 0.01 )
                if (c < 0) c = 0.0f;
                snprintf(saver_str, str_len, "B:%.1fV %.1fA", v, c);
                break;
            }
            _sa->oled.setFont(&Dotmatrix_7);
            //_sa->oled.setTextSize(2);
            _sa->oled.setTextColor(0x0F);
            _sa->oled.getTextBounds(saver_str, 0, 0, &x1, &y1, &saver_w, &saver_h);
            do { saver_vx = random(-3, 4); } while (saver_vx == 0);
            do { saver_vy = random(-2, 3); } while (saver_vy == 0);
            ++saver_frame;
        }
        saver_x = saver_x + saver_vx; saver_y = saver_y + saver_vy;
        if ((saver_x < 0) || (saver_x >= _sa->oled.width() - saver_w))
        {
            saver_vx = -saver_vx;
            saver_x = saver_x + saver_vx;
        }
        if ((saver_y < saver_h ) || (saver_y >= _sa->oled.height()))
        {
            saver_vy = -saver_vy;
            saver_y = saver_y + saver_vy;
        }
        _sa->oled.clearDisplay();
        _sa->oled.setCursor(saver_x, saver_y);
        _sa->oled.print(saver_str);
        _sa->oled.display();
        ++saver_frame;
        if (saver_frame > 200)
        {
            saver_frame = 0;
            ++saver_item;
            if (saver_item > 2)
                saver_item = 0;
        }
        saver_ts= now;
    }
}
