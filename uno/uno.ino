#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library

// TFT_eSPI will pick up TFT_CS, TFT_DC, TFT_RST from your User_Setup.h
TFT_eSPI tft = TFT_eSPI();            

void setup() {
  tft.init();            // Initialize display
  tft.setRotation(1);    // Landscape
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(1);
  tft.setTextFont(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Hello, TFT_eSPI!", 0, 0);
}

void loop() {
  static int x = 0;
  tft.drawPixel(x, 30, TFT_RED);
  delay(50);
  tft.drawPixel(x, 30, TFT_BLACK);
  x = (x + 1) % tft.width();
}
