#include "rgb_led.h"

#if __has_include(<esp_intr_alloc.h>)
#include <esp_intr_alloc.h>
#endif

#ifndef ESP_INTR_FLAG_IRAM
#define ESP_INTR_FLAG_IRAM 0
#endif

#ifndef RMT_MEM_ITEM_NUM
#define RMT_MEM_ITEM_NUM 64
#endif

#ifndef RGB_LED_MAX_RMT_BLOCKS
#define RGB_LED_MAX_RMT_BLOCKS 8
#endif

RGBLed::RGBLed(uint8_t dataPin, rmt_channel_t rmtChannel)
    : dataPin(dataPin), rmtChannel(rmtChannel), ledMutex(nullptr), memBlockCount(1) {}

RGBLed::~RGBLed()
{
    if (ledMutex != nullptr)
    {
        vSemaphoreDelete(ledMutex);
    }
}

void RGBLed::begin(uint16_t numLeds)
{
    if (ledMutex == nullptr)
    {
        ledMutex = xSemaphoreCreateMutex();
    }

    leds.resize(numLeds, RGB(0, 0, 0));
    txItems.resize(numLeds * 24 + 1);
    setupRMT();
}

void RGBLed::setLED(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (ledMutex != nullptr)
    {
        xSemaphoreTake(ledMutex, portMAX_DELAY);
    }

    if (index < leds.size())
    {
        leds[index] = RGB(red, green, blue);
    }

    if (ledMutex != nullptr)
    {
        xSemaphoreGive(ledMutex);
    }
}

void RGBLed::setLED(uint16_t index, const RGB &color)
{
    if (ledMutex != nullptr)
    {
        xSemaphoreTake(ledMutex, portMAX_DELAY);
    }

    if (index < leds.size())
    {
        leds[index] = color;
    }

    if (ledMutex != nullptr)
    {
        xSemaphoreGive(ledMutex);
    }
}

void RGBLed::update()
{
    sendRGB();
}

void RGBLed::setupRMT()
{
    const uint32_t totalItems = static_cast<uint32_t>(leds.size()) * 24U + 1U;
    uint32_t blocksNeeded = (totalItems + RMT_MEM_ITEM_NUM - 1U) / RMT_MEM_ITEM_NUM;

    if (blocksNeeded == 0)
    {
        blocksNeeded = 1;
    }
    if (blocksNeeded > RGB_LED_MAX_RMT_BLOCKS)
    {
        blocksNeeded = RGB_LED_MAX_RMT_BLOCKS;
    }

    memBlockCount = static_cast<uint8_t>(blocksNeeded);

    rmt_config_t config = {};
    config.rmt_mode = RMT_MODE_TX;
    config.channel = rmtChannel;
    config.gpio_num = (gpio_num_t)dataPin;
    config.clk_div = 2;
    config.mem_block_num = memBlockCount;
    config.flags = 0;

    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    config.tx_config.idle_output_en = true;

    rmt_config(&config);
    rmt_driver_install(rmtChannel, 0, ESP_INTR_FLAG_IRAM);
}

void RGBLed::sendRGB()
{
    const uint32_t T0H = 14; // 0.35us for '0' bit high time
    const uint32_t T0L = 32; // 0.80us for '0' bit low time
    const uint32_t T1H = 28; // 0.70us for '1' bit high time
    const uint32_t T1L = 24; // 0.60us for '1' bit low time

    if (ledMutex != nullptr)
    {
        xSemaphoreTake(ledMutex, portMAX_DELAY);
    }

    uint16_t numLeds = leds.size();
    if (txItems.size() != static_cast<size_t>(numLeds) * 24U + 1U)
    {
        txItems.resize(numLeds * 24 + 1);
    }

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
                    txItems[itemIdx].level0 = 1;
                    txItems[itemIdx].duration0 = T1H;
                    txItems[itemIdx].level1 = 0;
                    txItems[itemIdx].duration1 = T1L;
                }
                else
                {
                    txItems[itemIdx].level0 = 1;
                    txItems[itemIdx].duration0 = T0H;
                    txItems[itemIdx].level1 = 0;
                    txItems[itemIdx].duration1 = T0L;
                }
                itemIdx++;
            }
        }
    }

    txItems[itemIdx].level0 = 0;
    txItems[itemIdx].duration0 = 2000; // 50us reset
    txItems[itemIdx].level1 = 0;
    txItems[itemIdx].duration1 = 0;
    itemIdx++;

    rmt_wait_tx_done(rmtChannel, portMAX_DELAY);
    rmt_write_items(rmtChannel, txItems.data(), itemIdx, true);

    if (ledMutex != nullptr)
    {
        xSemaphoreGive(ledMutex);
    }
}