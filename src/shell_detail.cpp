#include "esphome.h"
#include "smartaps.h"
#include "font_symbol_10.h"
#include "font_dotmatrix_7.h"
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
 * Not publishing (causes unstable WiFi connection)
 */

#define BACK_TO_OVERVIEW_TIMER    300000UL        // 5*60*1000, 5 minutes, go back to overview shell

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
                    break;
                case CHANNEL_PORT_A:
                    ds->_sa->ina226_port_a.read(s, b);
                    break;
                case CHANNEL_PORT_B:
                    ds->_sa->ina226_port_b.read(s, b);
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
    ESP_LOGI(TAG, "Sampling task ended");
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


DetailShell::DetailShell(SmartAPS* sa) :
    Shell(sa),
    _waveform_s(_waveform_length),
    _waveform_b(_waveform_length)
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
    switch (_channel)
    {
    case CHANNEL_USB:
        _channel_enabled = id(out_usb).state;
        _sa->ina226_port_a.reset();
        _sa->ina226_port_b.reset();
        _sa->ina226_usb.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;        
    case CHANNEL_PORT_A:
        _channel_enabled = id(out_port_a).state;
        _sa->ina226_port_b.reset();
        _sa->ina226_usb.reset();
        _sa->ina226_port_a.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;
    case CHANNEL_PORT_B:
        _channel_enabled = id(out_port_b).state;
        _sa->ina226_port_a.reset();
        _sa->ina226_usb.reset();
        _sa->ina226_port_b.start(SAMPLE_MODE_10MS_CONTINUOUS);
        break;
    }
    _wh = 0; _ah = 0;
    _waveform_s.reset();
    _waveform_b.reset();
    _fps = FPS_100;
    sampling_timer = timerBegin(0, 80, true);    // 1us timer, count up
    timerAttachInterrupt(sampling_timer, on_sampling_timer, true);
    timerAlarmWrite(sampling_timer, 10000, true);    // 10ms
    _timestamp_back_to_overview = now;
    _count = 0; _max_count = 0;
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
    ESP_LOGD(TAG, "Leave Detail Shell channel %d", _channel);
}


Shell* DetailShell::loop(unsigned long now)
{
    bool redraw = false;
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
        redraw = true;
        _timestamp_back_to_overview = now;
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
        redraw = true;
        _timestamp_back_to_overview = now;
    }
    if (now - _timestamp_back_to_overview > BACK_TO_OVERVIEW_TIMER)
    {
        // Go back to overview shell
        _channel = CHANNEL_USB;   // next time start from USB
        return (&(_sa->shell_overview));
    }

    // Process buffer
     if (sample_count > 0)
     {
        redraw = true;
        _count = sample_count;
        if (_count > _max_count) _max_count = _count;
     }
    portENTER_CRITICAL(&sample_buffer_mux);
    for (int i = 0; i < sample_count; ++i)
    {
        int16_t s = sample_buffer_s[i];
        int16_t b = sample_buffer_b[i];
        float c, v;
        switch (_channel)
        {
        case CHANNEL_USB:
            // accumulate WH, AH
            c = sample_buffer_s[i] / 10000.0f;    // ( * 2.5u / 0.025 )
            if (c < 0.0f) c = 0.0f;
            v = sample_buffer_b[i] * 0.00125f;    // 1.25mV/lsb
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
            break;
        case CHANNEL_PORT_A:
        case CHANNEL_PORT_B:
            c = sample_buffer_s[i] / 4000.0f;     // ( * 2.5u / 0.01 )
            if (c < 0.0f) c = 0.0f;
            v = sample_buffer_b[i] * 0.00125f;    // 1.25mV/lsb
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
            break;
        }
        // insert data into waveform buffer
        _waveform_s.put(s);
        _waveform_b.put(b);
    }
    sample_count = 0;
    portEXIT_CRITICAL(&sample_buffer_mux);
    if (redraw)
        draw_ui();
    return this;
}


void DetailShell::start_sampling(void)
{
    xTaskCreatePinnedToCore(sampling_fn, "SAMPLING", 4096, this, uxTaskPriorityGet(NULL) + 1, &sampling_task_handle, CONFIG_ARDUINO_RUNNING_CORE);
    //_highfreq.start();
    timerAlarmEnable(sampling_timer);
}


void DetailShell::stop_sampling(void)
{
    timerAlarmDisable(sampling_timer);
    if (sampling_task_handle != NULL)
    {
        xTaskNotify(sampling_task_handle, STOP_BIT, eSetBits);
        //_highfreq.stop();
        delay(20);  // TODO: Better way to wait task to join?
    }
}


void DetailShell::draw_ui()
{
    const int buf_len = 16;
    char buf[buf_len + 1];
    int16_t x1, y1;
    uint16_t w, h;
    int16_t high_s = 0, low_s = 0;
    float high_c = 0.0f, low_c = 0.0f;
    int16_t s = 0, b = 0;
    float c = 0.0f, v = 0.0f;
    
    // Nothing to draw if not enough data
    if (_waveform_s.size() < 2)
        return;
    // find limits
    int16_t max_s, max_b;
    int16_t min_s, min_b;
    max_s = max_b = INT16_MIN;
    min_s = min_b = INT16_MAX;
    for (int i = 0; i < _waveform_s.size(); ++i)
    {
        s = _waveform_s[i]; b = _waveform_b[i];
        if (max_s < s) max_s = s;
        if (min_s > s) min_s = s;
        if (max_b < b) max_b = b;
        if (min_b > b) min_b = b;
    }
    // s and b now holding latest sample value
    // Start to draw    
    _sa->oled.clearDisplay();
    // Bottom banner
    _sa->oled.fillRect(0, 54, 256, 10, 0x02);
    _sa->oled.setFont(&Dotmatrix_7);
    _sa->oled.setTextColor(0x0F);
    _sa->oled.setCursor(140, 61);
    // SPS
    switch (_fps)
    {
    case FPS_100:
        _sa->oled.print("Sampling: 100 SPS");
        break;
    case FPS_10:
        _sa->oled.print("Sampling: 10 SPS");
        break;
    case FPS_1:
        _sa->oled.print("Sampling: 1 SPS");
        break;
    }

    switch (_channel)
    {
    case CHANNEL_USB:
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(0, 10);
        _sa->oled.print("\x87");    // TYPE C
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(6, 61);
        if (id(out_usb).state)
        {
            _sa->oled.print("Output: ON");
        }
        else
        {
            _sa->oled.print("Output: OFF");
        }
        c = s / 10000.0f;
        v = b * 0.00125f;
        // USB, range round to 0.1A (1000lsb)
        low_s = min_s / 1000 * 1000;
        high_s = (max_s + 1000) / 1000 * 1000;
        low_c = low_s / 10000.0f;
        high_c = high_s / 10000.0f;
        break;
    case CHANNEL_PORT_A:
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(0, 10);
        _sa->oled.print("\x88");    // PORT A
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(6, 61);
        if (id(out_port_a).state)
        {
            _sa->oled.print("Output: ON");
        }
        else
        {
            _sa->oled.print("Output: OFF");
        }
        c = s / 4000.0f;
        v = b * 0.00125f;
        // Port A, range round to 0.1A (400lsb)
        low_s = min_s / 400 * 400;
        high_s = (max_s + 400) / 400 * 400;
        low_c = low_s / 4000.0f;
        high_c = high_s / 4000.0f;
        break;
    case CHANNEL_PORT_B:
        _sa->oled.setFont(&Symbol_10);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(0, 10);
        _sa->oled.print("\x89");    // PORT B
        _sa->oled.setFont(&Dotmatrix_7);
        _sa->oled.setTextColor(0x0F);
        _sa->oled.setCursor(6, 61);
        if (id(out_port_a).state)
        {
            _sa->oled.print("Output: ON");
        }
        else
        {
            _sa->oled.print("Output: OFF");
        }
        c = s / 4000.0f;
        v = b * 0.00125f;
        // Port B, range round to 0.1A (400lsb)
        low_s = min_s / 400 * 400;
        high_s = (max_s + 400) / 400 * 400;
        low_c = low_s / 4000.0f;
        high_c = high_s / 4000.0f;
    }
    // Voltage
    snprintf(buf, buf_len, "%.3fV", v);
    _sa->oled.getTextBounds(buf, 0, 22, &x1, &y1, &w, &h);
    _sa->oled.setCursor(48 - w, 22);
    _sa->oled.print(buf);
    // Current
    snprintf(buf, buf_len, "%.3fA", c);
    _sa->oled.getTextBounds(buf, 0, 31, &x1, &y1, &w, &h);
    _sa->oled.setCursor(48 - w, 31);
    _sa->oled.print(buf);
    // WH
    snprintf(buf, buf_len, "%.3fWh", _wh);
    _sa->oled.getTextBounds(buf, 0, 40, &x1, &y1, &w, &h);
    _sa->oled.setCursor(48 - w, 40);
    _sa->oled.print(buf);
    // AH
    if (_ah < 1.0f)
        snprintf(buf, buf_len, "%dmAh", (int)(_ah * 1000));
    else
        snprintf(buf, buf_len, "%.1fAh", _ah * 1000);
    _sa->oled.getTextBounds(buf, 0, 49, &x1, &y1, &w, &h);
    _sa->oled.setCursor(48 - w, 49);
    _sa->oled.print(buf);
    // Waveform
    _sa->oled.drawRect(54, 0, 202, 52, 0x0F);
    int y0 = map(_waveform_s[0], low_s, high_s, 50, 1);
    for (int i = 1; i < _waveform_s.size(); ++i)
    {
        int y = map(_waveform_s[i], low_s, high_s, 50, 1);
        _sa->oled.drawLine(54 + i, y0, 55 + i, y, 0x04);
        y0 = y;
    }
    // Current labels
    _sa->oled.setCursor(56, 8);
    _sa->oled.printf("%0.1fA", high_c);
    _sa->oled.setCursor(56, 49);
    _sa->oled.printf("%0.1fA", low_c);
    // Draw!
    _sa->oled.display();
}
