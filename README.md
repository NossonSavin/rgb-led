# RGB LED Library

A PlatformIO library for controlling RGB addressable LEDs on ESP32 using the RMT peripheral.

## Features

- **WS2812B Support**: Optimized for common RGB addressable LEDs such as WS2812B
- **RMT Peripheral**: Uses ESP32's RMT (Remote Control) hardware for efficient LED control
- **No External Dependency**: Minimal dependencies, just Arduino framework
- **Easy Integration**: Simple API for LED control

## Supported Boards

- ESP32 (all variants)
- ESP32-S2
- ESP32-S3
- ESP32-C3

## Installation

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    rgb-led
```

Or install directly:

```bash
pio lib install rgb-led
```

## Usage

### Basic Example

```cpp
#include <Arduino.h>
#include <rgb_led.h>

// Create LED instance with data pin 17, using RMT_CHANNEL_0
RGBLed ledStrip(17, RMT_CHANNEL_0);

void setup() {
    // Initialize with 1 LED
    ledStrip.begin(1);
}

void loop() {
    // Set LED to red
    ledStrip.setLED(0, 255, 0, 0);  // (index, R, G, B)
    ledStrip.update();  // Send data to LED
    delay(1000);
    
    // Set LED to blue
    ledStrip.setLED(0, 0, 0, 255);
    ledStrip.update();
    delay(1000);
}
```

### Multiple LEDs Example

```cpp
RGBLed ledStrip(17, RMT_CHANNEL_0);

void setup() {
    ledStrip.begin(10);  // Initialize with 10 LEDs
}

void loop() {
    for (int i = 0; i < 10; i++) {
        ledStrip.setLED(i, i * 25, 0, 0);  // Red gradient
    }
    ledStrip.update();
    delay(100);
}
```

### Using RGB Struct

```cpp
RGBLed ledStrip(17, RMT_CHANNEL_0);

void setup() {
    ledStrip.begin(1);
}

void loop() {
    RGB color(255, 128, 0);  // Orange
    ledStrip.setLED(0, color);
    ledStrip.update();
}
```

## Configuration

Configuration is done through the constructor and `begin()` method - no file editing required!

```cpp
// Create instance with custom pin and RMT channel
RGBLed ledStrip(GPIO_PIN, RMT_CHANNEL);

// Initialize with number of LEDs
ledStrip.begin(NUM_LEDS);
```

### Constructor Parameters

- `dataPin`: GPIO pin connected to LED data (e.g., 17)
- `rmtChannel`: RMT channel to use (default: RMT_CHANNEL_0)
  - RMT_CHANNEL_0
  - RMT_CHANNEL_1
  - RMT_CHANNEL_2
  - RMT_CHANNEL_3 (for ESP32-S3)

## API Reference

### Constructor

```cpp
RGBLed(uint8_t dataPin, rmt_channel_t rmtChannel = RMT_CHANNEL_0);
```

Creates a new LED strip instance.

### Methods

#### `void begin(uint16_t numLeds)`

Initializes the LED strip with the specified number of LEDs. Must be called before using the strip.

#### `void setLED(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)`

Sets the color of an LED at the given index.

**Parameters:**
- `index`: LED index (0 to numLeds-1)
- `red`: Red value (0-255)
- `green`: Green value (0-255)
- `blue`: Blue value (0-255)

#### `void setLED(uint16_t index, const RGB& color)`

Sets the color of an LED using an RGB struct.

#### `void update()`

Sends the current LED data to the strip. Call this after setting LED colors.

#### `std::vector<RGB>& getLeds()`

Returns reference to the internal LED array for direct manipulation.

#### `uint16_t getNumLeds() const`

Returns the number of LEDs in the strip.

## Timing

The library uses the following timing for WS2812-style RGB LEDs:

- **T0H**: 0.35µs (14 RMT units at 2x clock division)
- **T0L**: 0.80µs (32 RMT units at 2x clock division)
- **T1H**: 0.70µs (28 RMT units at 2x clock division)
- **T1L**: 0.60µs (24 RMT units at 2x clock division)
- **Reset**: 50µs (2000 RMT units at 2x clock division)

## LED Order

The library uses GRB order in the data stream:
- Byte 0: Green
- Byte 1: Red
- Byte 2: Blue

## License

MIT License - See LICENSE file for details

## Author

Your Name (your.email@example.com)

## Support

For issues, questions, or contributions, please visit: https://github.com/yourusername/rgb-led
