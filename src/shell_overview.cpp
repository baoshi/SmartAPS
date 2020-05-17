
#include "esphome.h"
#include "smartaps.h"
#include "shell_overview.h"

using namespace esphome;

extern wifi::WiFiComponent *wifi_wificomponent;
extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_a;
extern gpio::GPIOSwitch *out_b;
extern gpio::GPIOSwitch *out_usb;


static const char *TAG = "ovshell";


/*
 * Overview shell
 * Actions:
 * s2 switches on/off port A
 * s3 switches on/off port B
 * s1 switches to next shell (detailshell_usb)
 *
 * Display: 
 * V/A for all channels
 * Time
 * WiFi signal strength
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
    _sa->oled.clearDisplay();
    _sa->oled.setTextColor(0x0F);
    _sa->terminal.home();
    _sa->oled.display();
    _timestamp_per_1s = now;
}


void OverviewShell::leave(unsigned long now)
{
    _sa->beeper.stop();
    ESP_LOGD(TAG, "Leave Overview Shell");
}


Shell* OverviewShell::loop(unsigned long now)
{
    uint32_t e1 = _sa->sw1.update(now);
    uint32_t event_id = BUTTON_EVENT_ID(e1);
    uint32_t event_param = BUTTON_EVENT_PARAM(e1);
    if (event_id == BUTTON_EVENT_CLICK)
    {
        return (&(_sa->shell_detail));
    }
    if (now - _timestamp_per_1s > 1000)
    {
        if (id(sntp_time).now().is_valid())
        {
            char buffer[64];
            size_t ret = id(sntp_time).now().strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S");
            _sa->terminal.println(buffer);
        }
        else
        {
            _sa->terminal.println("No time yet");
        }
        _sa->oled.display();
        _timestamp_per_1s = now;
    }
    _sa->beeper.loop();
    return this;
}