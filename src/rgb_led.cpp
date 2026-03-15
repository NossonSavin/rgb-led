#include "rgb_led.h"

RGBLed::RGBLed(uint8_t dataPin, rmt_channel_t rmtChannel)
    : dataPin(dataPin), rmtChannel(rmtChannel) {}

void RGBLed::begin(uint16_t numLeds)
{
    leds.resize(numLeds, RGB(0, 0, 0));
    setupRMT();
}

void RGBLed::setLED(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (index < leds.size())
    {
        leds[index] = RGB(red, green, blue);
    }
}

void RGBLed::setLED(uint16_t index, const RGB &color)
{
    if (index < leds.size())
    {
        leds[index] = color;
    }
}

void RGBLed::update()
{
    sendRGB();
}

void RGBLed::setupRMT()
{
    rmt_config_t config = {};
    config.rmt_mode = RMT_MODE_TX;
    config.channel = rmtChannel;
    config.gpio_num = (gpio_num_t)dataPin;
    config.clk_div = 2;
    config.mem_block_num = 1;
    config.flags = 0;

    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    config.tx_config.idle_output_en = true;

    rmt_config(&config);
    rmt_driver_install(rmtChannel, 0, 0);
}

void RGBLed::sendRGB()
{
    const uint32_t T0H = 14; // 0.35us for '0' bit high time
    const uint32_t T0L = 32; // 0.80us for '0' bit low time
    const uint32_t T1H = 28; // 0.70us for '1' bit high time
    const uint32_t T1L = 24; // 0.60us for '1' bit low time

    uint16_t numLeds = leds.size();
    std::vector<rmt_item32_t> items(numLeds * 24 + 1);
    int itemIdx = 0;

    for (uint16_t led = 0; led < numLeds; led++)
    {
        uint8_t data[3] = {leds[led].g, leds[led].r, leds[led].b};

        for (int byteIdx = 0; byteIdx < 3; byteIdx++)
        {
            for (int bit = 7; bit >= 0; bit--)
            {
                if (data[byteIdx] & (1 << bit))
                {
                    items[itemIdx].level0 = 1;
                    items[itemIdx].duration0 = T1H;
                    items[itemIdx].level1 = 0;
                    items[itemIdx].duration1 = T1L;
                }
                else
                {
                    items[itemIdx].level0 = 1;
                    items[itemIdx].duration0 = T0H;
                    items[itemIdx].level1 = 0;
                    items[itemIdx].duration1 = T0L;
                }
                itemIdx++;
            }
        }
    }

    items[itemIdx].level0 = 0;
    items[itemIdx].duration0 = 2000; // 50us reset
    items[itemIdx].level1 = 0;
    items[itemIdx].duration1 = 0;
    itemIdx++;

    rmt_write_items(rmtChannel, items.data(), itemIdx, true);
}