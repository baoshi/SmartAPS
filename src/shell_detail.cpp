#include "esphome.h"
#include "smartaps.h"
#include "shell_detail.h"

using namespace esphome;

extern sntp::SNTPComponent *sntp_time;
extern gpio::GPIOSwitch *out_port_a;
extern gpio::GPIOSwitch *out_port_b;
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

static TaskHandle_t sampling_task_handle = NULL;
hw_timer_t *sampling_timer = NULL;

portMUX_TYPE sample_buffer_mux = portMUX_INITIALIZER_UNLOCKED;
const static int sample_buffer_length = 64;
int sample_count = 0;
int16_t sample_buffer_b[sample_buffer_length];
int16_t sample_buffer_s[sample_buffer_length];


void sampling_fn(void * param)
{
    DetailShell* ds = reinterpret_cast<DetailShell*>(param);
    uint32_t notify;
    int frame = 0;
    long accu_s = 0, accu_b = 0;
    int accu_count = 0;
    ESP_LOGI(TAG, "Sampling task started");
    sample_count = 0;
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
                case CHANNEL_USB:
                    ds->_sa->ina226_usb.read(s, b);
                    ds->_usb_s = s;
                    ds->_usb_b = b;
                    // for every 5 seconds (500 samples on selected channel), we sneak in 
                    // sampling of the two other channels
                    if (++frame > 499)
                    {
                        ds->_sa->ina226_port_a.read(ds->_port_a_s, ds->_port_a_b);
                        ds->_sa->ina226_port_b.read(ds->_port_b_s, ds->_port_b_b);
                        frame = 0;
                    }
                    break;
                case CHANNEL_PORT_A:
                    ds->_sa->ina226_port_a.read(s, b);
                    ds->_port_a_s = s;
                    ds->_port_a_b = b;
                    // for every 5 seconds (500 samples on selected channel), we sneak in 
                    // sampling of the two other channels
                    if (++frame > 499)
                    {
                        ds->_sa->ina226_usb.read(ds->_usb_s, ds->_usb_b);
                        ds->_sa->ina226_port_b.read(ds->_port_b_s, ds->_port_b_b);
                        frame = 0;
                    }
                    break;
                case CHANNEL_PORT_B:
                    ds->_sa->ina226_port_b.read(s, b);
                    ds->_port_b_s = s;
                    ds->_port_b_b = b;
                    // for every 5 seconds (500 samples on selected channel), we sneak in 
                    // sampling of the two other channels
                    if (++frame > 499)
                    {
                        ds->_sa->ina226_usb.read(ds->_usb_s, ds->_usb_b);
                        ds->_sa->ina226_port_a.read(ds->_port_a_s, ds->_port_a_b);
                        frame = 0;
                    }
                    break;
                }
                // accumulating samples
                // fps == 100 -> no accumulate
                // fps == 10  -> accumulate 10 samples
                // fps == 1   -> accumulate 100 samples
                switch (ds->_fps)
                {
                case FPS_100:
                    portENTER_CRITICAL(&sample_buffer_mux);
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
                    portEXIT_CRITICAL(&sample_buffer_mux);
                    break;
                case FPS_10:
                    accu_b += b; accu_s += s; ++accu_count;
                    if (accu_count >= 10)
                    {
                        b = (int16_t)(accu_b / 10);
                        s = (int16_t)(accu_s / 10);
                        portENTER_CRITICAL(&sample_buffer_mux);
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
                        portEXIT_CRITICAL(&sample_buffer_mux);
                        accu_s = 0; accu_b = 0;
                        accu_count = 0;
                    }
                    break;
                case FPS_1:
                    accu_b += b; accu_s += s; ++accu_count;
                    if (accu_count >= 100)
                    {
                        b = (int16_t)(accu_b / 100);
                        s = (int16_t)(accu_s / 100);
                        portENTER_CRITICAL(&sample_buffer_mux);
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
                        portEXIT_CRITICAL(&sample_buffer_mux);
                        accu_s = 0; accu_b = 0;
                        accu_count = 0;
                    }
                    break;
                } // FPS
            } // SAMPLE_BIT
        } // xTaskNotify
        else
        {
            // xTaskNotify failed, shall not happen
        }
    }  // for (;;)
    ESP_LOGI(TAG, "Sampling task end");
    vTaskDelete(NULL);
    sampling_task_handle = NULL;
}


void IRAM_ATTR on_sampling_timer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (sampling_task_handle != NULL)
    {
        xTaskNotifyFromISR(sampling_task_handle, SAMPLE_BIT, eSetBits, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR();
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
    _channel = CHANNEL_USB;
}


void DetailShell::enter(unsigned long now)
{
    ESP_LOGD(TAG, "Enter Detail Shell channel %d", _channel);
    _sa->oled.reset();  // This force display to on
    _sa->oled.clearDisplay();
    _sa->oled.setFont(NULL);
    _sa->oled.setTextColor(0x0F);
    _sa->terminal.home();
    switch (_channel)
    {
    case CHANNEL_USB:
        _channel_enabled = id(out_usb).state;
        _sa->ina226_port_a.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_port_b.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_usb.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;        
    case CHANNEL_PORT_A:
        _channel_enabled = id(out_port_a).state;
        _sa->ina226_port_b.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_usb.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_port_a.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;
    case CHANNEL_PORT_B:
        _channel_enabled = id(out_port_b).state;
        _sa->ina226_port_a.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_usb.start(SAMPLE_MODE_300MS_CONTINUOUS);
        _sa->ina226_port_b.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;
    }
    _port_a_b = _sa->ina226_port_a.cache_bus;
    _port_a_s = _sa->ina226_port_a.cache_shunt;
    _port_b_b = _sa->ina226_port_b.cache_bus;
    _port_b_s = _sa->ina226_port_b.cache_shunt;
    _usb_b = _sa->ina226_usb.cache_bus;
    _usb_s = _sa->ina226_usb.cache_shunt;
    _wh = 0; _ah = 0;
    _fps = FPS_100;
    sampling_timer = timerBegin(0, 80, true);    // 1us timer, count up
    timerAttachInterrupt(sampling_timer, on_sampling_timer, true);
    timerAlarmWrite(sampling_timer, 10000, true);    // 10ms
    _timestamp_per_5000ms = now;
    start_sampling();
}


void DetailShell::leave(unsigned long now)
{
    stop_sampling();
    timerEnd(sampling_timer);
    sampling_timer = NULL;
    _sa->ina226_port_a.reset();
    _sa->ina226_port_b.reset();
    _sa->ina226_usb.reset();
    _sa->beeper.stop();
    ESP_LOGD(TAG, "Leave Detail Shell channel %d", _channel);
}


Shell* DetailShell::loop(unsigned long now)
{
    uint32_t e1 = _sa->sw1.update(now);
    uint32_t e2 = _sa->sw2.update(now);
    uint32_t e3 = _sa->sw3.update(now);
    if (BUTTON_EVENT_ID(e1) == BUTTON_EVENT_CLICK)
    {
        // sw1 click, switch channel
        switch (_channel)
        {
        case CHANNEL_USB:
            leave(now);
            _channel = CHANNEL_PORT_A;
            enter(now);
            break;
        case CHANNEL_PORT_A:
            leave(now);
            _channel = CHANNEL_PORT_B;
            enter(now);
            break;
        case CHANNEL_PORT_B:
            _channel = CHANNEL_USB;   // next time start from USB
            return (&(_sa->shell_overview));
            break;
        }
    }
    if (BUTTON_EVENT_ID(e3) == BUTTON_EVENT_CLICK)
    {
        // sw3 click, cycle FPS
        stop_sampling();
        switch (_fps)
        {
        case FPS_100:
            _fps = FPS_10;
            break;
        case FPS_10:
            _fps = FPS_1;
            break;
        case FPS_1:
            _fps = FPS_100;
            break;
        }
        start_sampling();
    }
    if (BUTTON_EVENT_ID(e2) == BUTTON_EVENT_CLICK)
    {
        // sw2 click, on/off channel
        switch (_channel)
        {
        case CHANNEL_USB:
            id(out_usb).toggle();
            _channel_enabled = id(out_usb).state;
            break;
        case CHANNEL_PORT_A:
            id(out_port_a).toggle();
            _channel_enabled = id(out_port_a).state;
            break;
        case CHANNEL_PORT_B:
            id(out_port_b).toggle();
            _channel_enabled = id(out_port_b).state;
            break;
        }
    }
    // Process buffer
    switch (_channel)
    {
    case CHANNEL_USB:
        portENTER_CRITICAL(&sample_buffer_mux);
        for (int i = 0; i < sample_count; ++i)
        {
            float c = sample_buffer_s[i] / 10000.0f;    // ( * 2.5u / 0.025 )
            if (c < 0.0f) c = 0.0f;
            float v = sample_buffer_b[i] * 0.00125f;    // 1.25mV/lsb
            if (v < 0.0f) v = 0.0f;
            switch (_fps)
            {
            case FPS_100:
                _ah += c / 360000.0f;
                _wh += (c * v) / 360000.0f;
                break;
            case FPS_10:
                _ah += c / 36000.0f;
                _wh += (c * v) / 36000.0f;
                break;
            case FPS_1:
                _ah += c / 3600.0f;
                _wh += (c * v) / 3600.0f;
                break;
            }
        }
        sample_count = 0;
        portEXIT_CRITICAL(&sample_buffer_mux);
        _sa->terminal.printf("U: %.2fWH, %.3fAH\n", _wh, _ah);
        break; 
    case CHANNEL_PORT_A:
        portENTER_CRITICAL(&sample_buffer_mux);
        for (int i = 0; i < sample_count; ++i)
        {
            float c = sample_buffer_s[i] / 4000.0f;     // ( * 2.5u / 0.01 )
            if (c < 0.0f) c = 0.0f;
            float v = sample_buffer_b[i] * 0.00125f;    // 1.25mV/lsb
            if (v < 0.0f) v = 0.0f;
            switch (_fps)
            {
            case FPS_100:
                _ah += c / 360000.0f;
                _wh += (c * v) / 360000.0f;
                break;
            case FPS_10:
                _ah += c / 36000.0f;
                _wh += (c * v) / 36000.0f;
                break;
            case FPS_1:
                _ah += c / 3600.0f;
                _wh += (c * v) / 3600.0f;
                break;
            }
        }
        sample_count = 0;
        portEXIT_CRITICAL(&sample_buffer_mux);
        _sa->terminal.printf("A: %.2fWH, %.3fAH\n", _wh, _ah);
        break;
    case CHANNEL_PORT_B:
        portENTER_CRITICAL(&sample_buffer_mux);
        for (int i = 0; i < sample_count; ++i)
        {
            float c = sample_buffer_s[i] / 4000.0f;     // ( * 2.5u / 0.01 )
            if (c < 0.0f) c = 0.0f;
            float v = sample_buffer_b[i] * 0.00125f;    // 1.25mV/lsb
            if (v < 0.0f) v = 0.0f;
            switch (_fps)
            {
            case FPS_100:
                _ah += c / 360000.0f;
                _wh += (c * v) / 360000.0f;
                break;
            case FPS_10:
                _ah += c / 36000.0f;
                _wh += (c * v) / 36000.0f;
                break;
            case FPS_1:
                _ah += c / 3600.0f;
                _wh += (c * v) / 3600.0f;
                break;
            }
        }
        sample_count = 0;
        portEXIT_CRITICAL(&sample_buffer_mux);
        _sa->terminal.printf("B: %.2fWH, %.3fAH\n", _wh, _ah);
        break;
    }
    _sa->oled.display();
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


void DetailShell::start_sampling(void)
{
    xTaskCreatePinnedToCore(sampling_fn, "SAMPLING", 2000, this, uxTaskPriorityGet(NULL) + 1, &sampling_task_handle, CONFIG_ARDUINO_RUNNING_CORE);
    timerAlarmEnable(sampling_timer);
}


void DetailShell::stop_sampling(void)
{
    timerAlarmDisable(sampling_timer);
    if (sampling_task_handle != NULL)
    {
        xTaskNotify(sampling_task_handle, STOP_BIT, eSetBits);
        delay(20);  // TODO: Better way to wait task to join?
    }
}


void DetailShell::draw_ui()
{

}