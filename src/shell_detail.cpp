#include "esphome.h"
#include "smartaps.h"
#include "shell_detail.h"

using namespace esphome;

extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_a;
extern gpio::GPIOSwitch *out_b;
extern gpio::GPIOSwitch *out_usb;

static const char *TAG = "dtshell";

/*
 * Detail shell
 * Actions:
 * s2 switches on/off channel
 * s3 toggle samples/second
 * s1 switches to next shell (next channel, until last channel then go back to overview)
 *
 * Display: 
 * V/A for selected channel, graph
 * Time
 * 
 * Publishes:
 * V/A of all channels every  5? second
 */


DetailShell::DetailShell(SmartAPS* sa) : Shell(sa)
{
}


DetailShell::~DetailShell()
{

}


void DetailShell::init(void)
{
    _channel = CHANNEL_PORT_A;
}


void DetailShell::enter(unsigned long now)
{
    ESP_LOGD(TAG, "Enter Detail Shell channel %d", _channel);
    _sa->oled.clearDisplay();
    _sa->oled.setTextColor(0x0F);
    _sa->oled.setFont(NULL);
    _sa->terminal.home();
    switch (_channel)
    {
    case CHANNEL_PORT_A:
        _sa->terminal.println("Port A");
        break;
    case CHANNEL_PORT_B:
        _sa->terminal.println("Port B");
        break;
    case CHANNEL_USB:
        _sa->terminal.println("USB");
        break;
    }
    _sa->oled.display();
}


void DetailShell::leave(unsigned long now)
{
    _sa->beeper.stop();
    ESP_LOGD(TAG, "Leave Detail Shell channel %d", _channel);
}


Shell* DetailShell::loop(unsigned long now)
{
    uint32_t e1 = _sa->sw1.update(now);
    uint32_t event_id = BUTTON_EVENT_ID(e1);
    uint32_t event_param = BUTTON_EVENT_PARAM(e1);
    if (event_id == BUTTON_EVENT_CLICK)
    {
        // sw1 click
        switch (_channel)
        {
        case CHANNEL_PORT_A:
            leave(now);
            _channel = CHANNEL_PORT_B;
            enter(now);
            break;
        case CHANNEL_PORT_B:
            leave(now);
            _channel = CHANNEL_USB;
            enter(now);
            break;
        case CHANNEL_USB:
            _channel = CHANNEL_PORT_A;   // next time start from port A
            return (&(_sa->shell_overview));
            break;
        }
    }
    return this;
}