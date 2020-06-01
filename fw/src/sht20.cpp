#include "sht20.h"


using namespace esphome;
using namespace sequencer;

static const char *TAG = "sht20";


#define SHT20_ADDRESS  0x40


std::vector<action_t> _operations =
{
    { 1,    20, false },  // soft reset, take 15ms
    { 2,   100, false },  // Start measure temperature and wait (14bit temperature measurement max 85ms)
    { 3,    40, false },  // Read temperature, start measure humidity and wait (12it RH measurement max 29ms)
    { 4, 60000, false}    // Read humidity, wait for next cycle (1 minute delay)
};


SHT20::SHT20() :
    temperature(NAN),
    humidity(NAN),
    _bus(nullptr)
{
}


bool SHT20::execute(const action_t *action)
{
    switch (action->opcode)
    {
    case 1:
    {
        ESP_LOGI(TAG, "Reset");
        _bus->beginTransmission(SHT20_ADDRESS);
        _bus->write(0xFE);  // Reset
        _bus->endTransmission();
    }
    break;
    case 2:
    {
        ESP_LOGI(TAG, "Measure temperature");
        _bus->beginTransmission(SHT20_ADDRESS);
        _bus->write(0xF3);
        _bus->endTransmission();
    }
    break;
    case 3:
    {
        ESP_LOGI(TAG, "Read temperature");
        _bus->requestFrom(SHT20_ADDRESS, 3);
        uint16_t reading = (_bus->read() & 0xFF);
        reading = (reading << 8) | (_bus->read() & 0xFF);
        uint8_t sum = _bus->read();
        if (_verify_crc(reading, sum) == 0)
        {
            reading = (reading & 0xFFFC); // last two bits are status
            temperature = (reading * (175.72 / 65536.0) - 46.85);
        }
        else
        {
            ESP_LOGW(TAG, "Temperature checksum failed");
            temperature = NAN;
        }
        ESP_LOGI(TAG, "Measure humidity");
        _bus->beginTransmission(SHT20_ADDRESS);
        _bus->write(0xF5);
        _bus->endTransmission();
    }
    break;
    case 4:
    {
        ESP_LOGI(TAG, "Read humidity");
        _bus->requestFrom(SHT20_ADDRESS, 3);
        uint16_t reading = (_bus->read() & 0xFF);
        reading = (reading << 8) | (_bus->read() & 0xFF);
        uint8_t sum = _bus->read();
        if (_verify_crc(reading, sum) == 0)
        {
            reading = (reading & 0xFFFC); // last two bits are status
            humidity = reading * (125.0 / 65536.0) - 6.0;
        }
        else
        {
            ESP_LOGW(TAG, "Humidity checksum failed");
            humidity = NAN;
        }
    }
    break;
    }
    return true;
}


// Ported and refactored from Sparkfun Arduino HTU21D Library: https://github.com/sparkfun/HTU21D_Breakout
// returns 0 if crc is correct
bool SHT20::_verify_crc(uint16_t message, uint8_t crc)
{
    uint32_t remainder = (uint32_t)message << 8;
    remainder |= crc;
    // POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1
    // divsor = 0x988000 is the 0x0131 polynomial shifted to farthest left of three bytes
    uint32_t divsor = (uint32_t)0x988000;
    for (int i = 0 ; i < 16 ; i++)
    {
        if (remainder & (uint32_t)1 << (23 - i))
        {
            remainder ^= divsor;
        }
        divsor >>= 1;
    }
    return (uint8_t)remainder;
}


void SHT20::begin(TwoWire* bus)
{
    _bus = bus;
    start(&_operations, 0);
}


void SHT20::loop()
{
    Sequencer::loop();
}