# Esp32AirQualitySensor
Soldering free!

## How to connect the ESP32 to the 1.8 inch TFT display
![](./Figures/tft_1_8.jpg) ![](./Figures/ESP32-DEVKIT-V1-30pin.png)

|TFT Pin|ESP32 Pin|
|-------|---------|
|VCC    |3.3V     |
|GND    |GND      |
|SCK    |GPIO 18  |
|SDA    |GPIO 23  |
|CS     |GPIO 5   |
|AO     |GPIO 2   |
|RST    |GPIO 4   |
|LED    |GPIO 15  |

## How to connect the ESP32 to the DHT22 sensor
![](./Figures/DHT22.png)

|DHT Pin|ESP32 Pin|
|-------|---------|
|VCC    |VIN      |
|GND    |GND      |
|OUT    |GPIO 27  |

## Temperature and Humidity Sensor with TFT Display

### Dependencies
- DHT sensor library
- TFT_eSPI library

### TFT_eSPI Configuration
- Edit the `User_Setup.h` file in the TFT_eSPI library folder to match your display configuration.
```C++
#define USER_SETUP_INFO "User_Setup"

#define ST7735_DRIVER      // Define additional parameters below for this display

#define ST7735_BLACKTAB160x128

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

// ##################################################################################
//
// Section 3. Define the fonts that are to be used here
//
// ##################################################################################

// Comment out the #defines below with // to stop that font being loaded
// The ESP8366 and ESP32 have plenty of memory so commenting out fonts is not
// normally necessary. If all fonts are loaded the extra FLASH space required is
// about 17Kbytes. To save FLASH space only enable the fonts you need!

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

#define SPI_FREQUENCY  40000000

#define SPI_READ_FREQUENCY  20000000

#define SPI_TOUCH_FREQUENCY  2500000
```
