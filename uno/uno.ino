/*
 * Air-quality monitor
 * – incremental redraw, then total shut-down after 1 minute
 * Board : Arduino Uno
 * Display: ST7735 (Adafruit_ST7735 / ST7789 library)
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SoftwareSerial.h>
#include <math.h>              // fabsf()

// ───────── pin map ─────────
#define TFT_CS     10
#define TFT_DC      9
#define TFT_RST     8           // we will pull this low at shut-down
#define TFT_LED     7           // HIGH = on  •  LOW = off

#define SENSOR_RX   2
#define SENSOR_TX   3

SoftwareSerial  sensorSerial(SENSOR_RX, SENSOR_TX);
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// ───────── layout ──────────
const uint8_t FONT_SIZE   = 2;
const uint8_t LINE_HEIGHT = 8 * FONT_SIZE;
const uint8_t MARGIN_X    = 1;
const uint8_t VALUE_X     = 64;

// ───────── timer ───────────
const unsigned long DISPLAY_ON_MS = 60000UL;   // 1 minute
unsigned long bootMillis;

// ───────── display state ───
bool displayActive = true;                     // becomes false after shut-down
bool firstPaint    = true;

// cached values
uint16_t prevCO2  = 0xFFFF, prevPM25 = 0xFFFF, prevPM10 = 0xFFFF;
float    prevTVOC = NAN,    prevCH2O = NAN,    prevTemp = NAN, prevHum = NAN;

// ───────── helper routines ─
void drawFieldUInt(int16_t y, const __FlashStringHelper *label,
                   uint16_t value, uint16_t &cache)
{
  if (firstPaint || value != cache) {
    tft.fillRect(VALUE_X, y, 128 - VALUE_X, LINE_HEIGHT, ST77XX_WHITE);

    tft.setCursor(MARGIN_X, y);
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);  tft.print(label);

    tft.setCursor(VALUE_X, y);
    tft.setTextColor(ST77XX_RED,   ST77XX_WHITE);  tft.print(value);

    cache = value;
  }
}

void drawFieldFloat(int16_t y, const __FlashStringHelper *label,
                    float value, float &cache, uint8_t digits)
{
  float step = (digits == 0) ? 1.0f : 1.0f / powf(10.0f, digits);
  if (firstPaint || fabsf(value - cache) >= step) {
    tft.fillRect(VALUE_X, y, 128 - VALUE_X, LINE_HEIGHT, ST77XX_WHITE);

    tft.setCursor(MARGIN_X, y);
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);  tft.print(label);

    tft.setCursor(VALUE_X, y);
    tft.setTextColor(ST77XX_RED,   ST77XX_WHITE);  tft.print(value, digits);

    cache = value;
  }
}

// ───────── hard-power-off ───
void shutdownDisplay()
{
  if (!displayActive) return;          // already done

  // 1. blank GRAM so a faint image can't persist
  tft.fillScreen(ST77XX_BLACK);

  // 2. panel off sequence: DISPOFF (0x28) → SLPIN (0x10)
  tft.writeCommand(ST77XX_DISPOFF);
  delay(10);
  tft.writeCommand(ST77XX_SLPIN);
  delay(120);                          // spec: >120 ms for internal circuits to power-down

  // 3. back-light off
  digitalWrite(TFT_LED, LOW);

  // 4. hold display in hardware reset (optional but ensures µA sleep)
  digitalWrite(TFT_RST, LOW);

  displayActive = false;
}

// ───────── Arduino setup ──
void setup()
{
  pinMode(TFT_LED, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);         // release reset
  digitalWrite(TFT_LED, HIGH);         // back-light on

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.setTextSize(FONT_SIZE);
  tft.fillScreen(ST77XX_WHITE);

  sensorSerial.begin(9600);
  bootMillis = millis();
}

// ───────── main loop ──────
void loop()
{
  // ░░ 1. shut down after one minute ░░
  if (displayActive && (millis() - bootMillis >= DISPLAY_ON_MS))
    shutdownDisplay();

  // once powered down, drop into low-duty idle
  if (!displayActive) {
    delay(100);
    return;
  }

  // ░░ 2. normal incremental update ░░
  if (sensorSerial.available() >= 17) {
    uint8_t buf[17];
    sensorSerial.readBytes(buf, 17);

    if (buf[0] == 0x3C && buf[1] == 0x02) {         // header OK
      uint8_t sum = 0;  for (int i = 0; i < 16; ++i) sum += buf[i];
      if (sum == buf[16]) {                         // checksum OK
        uint16_t co2   = (buf[2]  << 8) | buf[3];
        uint16_t pm25  = (buf[8]  << 8) | buf[9];
        uint16_t pm10  = (buf[10] << 8) | buf[11];
        float   ch2o   = ((buf[4] << 8) | buf[5]) / 100.0f;
        float   tvoc   = ((buf[6] << 8) | buf[7]) / 100.0f;
        bool    neg    = buf[12] & 0x80;
        uint8_t tI     = buf[12] & 0x7F, tD = buf[13];
        float   temp   = (neg ? -1.0f : 1.0f) * (tI + tD / 100.0f);
        float   hum    = buf[14] + buf[15] / 100.0f;

        int16_t y = 20;
        drawFieldUInt (y, F("CO2:  "), co2,  prevCO2);  y += LINE_HEIGHT;
        drawFieldUInt (y, F("PM2.5:"), pm25, prevPM25); y += LINE_HEIGHT;
        drawFieldUInt (y, F("PM10: "), pm10, prevPM10); y += LINE_HEIGHT;
        drawFieldFloat(y, F("TVOC: "), tvoc, prevTVOC, 2); y += LINE_HEIGHT;
        drawFieldFloat(y, F("CH2O: "), ch2o, prevCH2O, 2); y += LINE_HEIGHT;
        drawFieldFloat(y, F("Temp: "), temp, prevTemp, 1); y += LINE_HEIGHT;
        drawFieldFloat(y, F("Hum:  "), hum,  prevHum,  1);

        firstPaint = false;
      }
    }
  }

  delay(10);                            // small breathing space
}
