#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SoftwareSerial.h>

#define TFT_CS     10
#define TFT_DC      9
#define TFT_RST     8
#define TFT_LED     7

#define SENSOR_RX   2   // UNO reads sensor TX
#define SENSOR_TX   3   // UNO drives sensor RX

SoftwareSerial sensorSerial(SENSOR_RX, SENSOR_TX);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// layout constants
const uint8_t FONT_SIZE  = 2;
const uint8_t LINE_HEIGHT = 8 * FONT_SIZE;  // 8px font × scale
const uint8_t MARGIN_X   = 1;               // 1px left indent

void setup() {
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextSize(FONT_SIZE);

  sensorSerial.begin(9600);
}

void loop() {
  if (sensorSerial.available() >= 17) {
    uint8_t buf[17];
    sensorSerial.readBytes(buf, 17);

    // validate header
    if (buf[0] == 0x3C && buf[1] == 0x02) {
      // checksum
      uint8_t sum = 0;
      for (int i = 0; i < 16; i++) sum += buf[i];

      // clear with white background
      tft.fillScreen(ST77XX_WHITE);

      if (sum == buf[16]) {
        // parse values
        uint16_t co2   = (buf[2]<<8)  | buf[3];
        uint16_t pm25  = (buf[8]<<8)  | buf[9];
        uint16_t pm10  = (buf[10]<<8) | buf[11];
        float   ch2o   = ((buf[4]<<8)  | buf[5])  / 100.0;
        float   tvoc   = ((buf[6]<<8)  | buf[7])  / 100.0;
        bool    neg    = buf[12] & 0x80;
        uint8_t tI     = buf[12] & 0x7F, tD = buf[13];
        float   temp   = (neg ? -1 : 1)*(tI + tD/100.0);
        float   hum    = buf[14] + buf[15]/100.0;

        // print each line with 1px indent
        int16_t y = 20;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("CO2:  "));
        tft.setTextColor(ST77XX_RED);
        tft.print(co2);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("PM2.5:"));
        tft.setTextColor(ST77XX_RED);
        tft.print(pm25);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("PM10: "));
        tft.setTextColor(ST77XX_RED);
        tft.print(pm10);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("TVOC: "));
        tft.setTextColor(ST77XX_RED);
        tft.print(tvoc, 2);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("CH2O: "));
        tft.setTextColor(ST77XX_RED);
        tft.print(ch2o, 2);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("Temp: "));
        tft.setTextColor(ST77XX_RED);
        tft.print(temp, 1);
        y += LINE_HEIGHT;

        tft.setCursor(MARGIN_X, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(F("Hum:  "));
        tft.setTextColor(ST77XX_RED);
        tft.print(hum, 1);

      } else {
        // checksum error
        tft.setCursor(MARGIN_X, 0);
        tft.setTextColor(ST77XX_RED);
        tft.print(F("BAD CHECKSUM"));
      }
    }
  }

  delay(10);
}
