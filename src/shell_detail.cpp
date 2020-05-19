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


#define SAMPLE_BIT      0x01
#define STOP_BIT        0x02

static TaskHandle_t sampling_task_handle;
hw_timer_t *sampling_timer;

portMUX_TYPE sample_buffer_muxtex = portMUX_INITIALIZER_UNLOCKED;
const static int sample_buffer_length = 64;
int sample_count = 0;
int16_t sample_buffer_b[sample_buffer_length];
int16_t sample_buffer_s[sample_buffer_length];


void sampling_fn(void * param)
{
    DetailShell* ds = reinterpret_cast<DetailShell*>(param);
    sample_count = 0;
    uint32_t notify;
    ESP_LOGI(TAG, "Sampling task started");
    for (;;)
    {
        if (xTaskNotifyWait(pdFALSE, ULONG_MAX, &notify, portMAX_DELAY) == pdPASS)
        {
            if ((notify & STOP_BIT) != 0)
            {
                // TODO:: Cleanup
                break;
            } // STOP_BIT
            if ((notify & SAMPLE_BIT) != 0)
            {
                int16_t s, b;
                switch (ds->_channel)
                {
                case CHANNEL_PORT_A:
                    ds->_sa->ina226_port_a.read(s, b);
                    ds->_sa->ina226_port_a.start(SAMPLE_MODE_10MS_TRIGGERED);
                    break;
                case CHANNEL_PORT_B:
                    ds->_sa->ina226_port_b.read(s, b);
                    ds->_sa->ina226_port_b.start(SAMPLE_MODE_10MS_TRIGGERED);
                    break;
                case CHANNEL_USB:
                    ds->_sa->ina226_usb.read(s, b);
                    ds->_sa->ina226_usb.start(SAMPLE_MODE_10MS_TRIGGERED);
                    break;
                }
                portENTER_CRITICAL(&sample_buffer_muxtex);
                if (sample_count < sample_buffer_length)
                {
                    sample_buffer_s[sample_count] = s;
                    sample_buffer_b[sample_count] = b;
                    ++sample_count;
                }
                else
                {
                    ESP_LOGW(TAG, "Sample buffer overflow");
                }
                portEXIT_CRITICAL(&sample_buffer_muxtex);
            } // SAMPLE_BIT
        } // xTaskNotify
        else
        {
            // xTaskNotify failed, shall not happen
        }
    }  // for (;;)
    ESP_LOGI(TAG, "Sampling task end");
    vTaskDelete(NULL);
}


void IRAM_ATTR on_sampling_timer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (sampling_task_handle != NULL)
    {
        xTaskNotifyFromISR(sampling_task_handle, SAMPLE_BIT, eSetBits, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR ();
        }
    }
}


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
    _sa->oled.reset();  // This force display to on
    switch (_channel)
    {
    case CHANNEL_PORT_A:
        break;
    case CHANNEL_PORT_B:
        break;
    case CHANNEL_USB:
        break;
    }
}


void DetailShell::leave(unsigned long now)
{
    _sa->beeper.stop();
    timerAlarmDisable(sampling_timer);
    if (sampling_task_handle != NULL)
    {
        xTaskNotify(sampling_task_handle, STOP_BIT, eSetBits);
        // TODO: Do we need to wait thread join?
        sampling_task_handle = NULL;
    }
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