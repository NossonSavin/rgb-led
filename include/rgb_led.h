#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include <driver/rmt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <vector>

struct RGB
{
    uint8_t r, g, b;
    RGB(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
        : r(red), g(green), b(blue) {}
};

class RGBLed
{
public:
    RGBLed(uint8_t dataPin, rmt_channel_t rmtChannel = RMT_CHANNEL_0);
    ~RGBLed();

    void begin(uint16_t numLeds);

    void setLED(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
    void setLED(uint16_t index, const RGB &color);

    void update();

    std::vector<RGB> &getLeds() { return leds; }

    uint16_t getNumLeds() const { return leds.size(); }

private:
    uint8_t dataPin;
    rmt_channel_t rmtChannel;
    std::vector<RGB> leds;
    std::vector<rmt_item32_t> txItems;
    SemaphoreHandle_t ledMutex;
    uint8_t memBlockCount;

    void setupRMT();
    void sendRGB();
};

#endif // RGB_LED_H