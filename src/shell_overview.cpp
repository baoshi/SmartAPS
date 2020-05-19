
#include "esphome.h"
#include "smartaps.h"
#include "font_seg7_23.h"
#include "font_symbol_7.h"
#include "font_dotmatrix_7.h"
#include "shell_overview.h"

using namespace esphome;

extern wifi::WiFiComponent *wifi_wificomponent;
extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_port_a;
extern gpio::GPIOSwitch *out_port_b;
extern gpio::GPIOSwitch *out_usb;

#define LOWLIGHT_THRESHOLD      5       // below this value is considered dark room
#define AUTO_OFF_TIMER          30000   // turn off display 30 seconds after room go dark

static const char *TAG = "ovshell";


/*
 * Overview shell
 * Actions:
 * s2 switches on/off port A
 * s3 switches on/off port B
 * s1 switches to next shell (detailshell_usb)
 *
 * Display every 500ms: 
 * V/A for all channels
 * Time
 * WiFi signal strength
 * 
 * Turn off display if all output is off (current low)
 * 
 * Publishes:
 * V/A of all channels every  5 second
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
    _sa->oled.reset();  // This force display to on
    _display_on = true;
    _sa->sw1.begin(true);
    _sa->sw2.begin(true);
    _sa->sw3.begin(true);
    _sa->ina226_port_a.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_port_b.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _sa->ina226_usb.start(SAMPLE_MODE_300MS_CONTINUOUS);
    _timestamp_per_500ms = 0;
    _timestamp_per_5000ms = 0;
    _timestamp_light_off = 0;
}


void OverviewShell::leave(unsigned long now)
{
    _sa->beeper.stop();
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
    bool button_clicked = false;
    if (BUTTON_EVENT_ID(e2) == BUTTON_EVENT_CLICK)
    {
        button_clicked = true;
        // If button pressed while display is off,
        // only turn on display, don't toggle output.
        if (_display_on)
            out_port_a->toggle();
    }
    if (BUTTON_EVENT_ID(e3) == BUTTON_EVENT_CLICK)
    {
        button_clicked = true;
        if (_display_on)
            out_port_b->toggle();
    }
    if (button_clicked)
    {
        // Turn on display if button pressed
        // Also reset auto-off timer
        if (!_display_on)
        {
            ESP_LOGI(TAG, "Turning on OLED");
            _sa->oled.on();
            _display_on = true;
        }
        _timestamp_light_off = 0;
    }
    else
    {
        // no button click, determine whether turn off display by
        // measure ambient light
        float lx = _sa->temt6000.read();
        if (lx > LOWLIGHT_THRESHOLD)
        {
            if (!_display_on)
            {
                ESP_LOGD(TAG, "Light = %d lux", (int)lx);
                ESP_LOGI(TAG, "Turning on OLED");
                _sa->oled.on();
                _display_on = true;
                _timestamp_light_off = 0;
            }
        }
        else
        {
            // Low ambient light
            if (_display_on)
            {
                if (_timestamp_light_off == 0)
                    _timestamp_light_off = now;
                if (now - _timestamp_light_off > AUTO_OFF_TIMER)
                {
                    ESP_LOGD(TAG, "Light = %d lux", (int)lx);
                    ESP_LOGI(TAG, "Turning off OLED");
                    _sa->oled.off();
                    _display_on = false;
                }
            }
        }
    }
    // Update UI if button pressed or 2fps due
    if (button_clicked || (now - _timestamp_per_500ms > 500))
    {
        _sa->ina226_port_a.read(_port_a_s, _port_a_b);
        _sa->ina226_port_b.read(_port_b_s, _port_b_b);
        _sa->ina226_usb.read(_usb_s, _usb_b);
        if (_display_on)
            draw_ui();
        _timestamp_per_500ms = now;
    }
    // Publish reading every 5 seconds
    if (now - _timestamp_per_5000ms > 5000)
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
        _timestamp_per_5000ms = now;
    }
    _sa->beeper.loop();
    return this;
}


void OverviewShell::draw_ui(void)
{
    const int buf_len = 64;
    char buf[buf_len - 1];
    int16_t  x1, y1;
    uint16_t w, h;
    float v, c;
    
    _sa->oled.clearDisplay();
    _sa->oled.drawFastHLine(0, 8, 256, 0x0F);
    // USB V/A
    if (id(out_usb).state)
    {
        _sa->oled.setFont(&Symbol_7);
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
        _sa->oled.setFont(&Symbol_7);
        _sa->oled.setCursor(0, 6);
        _sa->oled.setTextColor(0x02);
        _sa->oled.print("\x81");  // USB symbol    
    }
    // WiFi Symbol
    if (id(wifi_wificomponent).is_connected())
    {
        _sa->oled.setFont(&Symbol_7);
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
        _sa->oled.setFont(&Symbol_7);
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