#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Wire.h>
#include "font.h"

#define OLED_ADDRESS 0x3C  // I2C address for the SSD1306 OLED
#define SDA_PIN 21          // ESP32 SDA pin
#define SCL_PIN 22          // ESP32 SCL pin
#define OLED_CMD  0  // Command mode
#define OLED_DATA 1  // Data mode
uint8_t OLED_GRAM[128][8];

// WiFi credentials – replace with your network info
const char* ssid     = "QY_Home";
const char* password = "qinyao1805.";

// UDP broadcast settings
WiFiUDP udp;
const char* udpAddress = "255.255.255.255"; // adjust if needed for your network
const int udpPort = 4210;

// Create an HTTP server on port 80
WebServer server(80);

// Global variable to store the latest sensor data as a string
String sensorOutput = "No data available";

// HardwareSerial instance for sensor communication on UART2
HardwareSerial sensorSerial(2);

// Timing variables for sensor reading
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 1000;  // read sensor every 1000ms

// HTTP handler: serve a simple HTML page that auto-refreshes every 5 seconds
void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='5'>";
  html += "<title>Sensor Data</title></head><body><h1>Sensor Data</h1>";
  html += "<p>" + sensorOutput + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start UDP on the chosen port
  udp.begin(udpPort);

  // Setup HTTP server routes
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize sensor UART (assumed 9600, RX=16, TX=17)
  sensorSerial.begin(9600, SERIAL_8N1, 16, 17);
  // Optionally, set a shorter timeout to reduce blocking
  sensorSerial.setTimeout(10);
  
  Serial.println("7-in-1 Sensor");
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with ESP32 pins
  OLED_Init();
  OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
}

void loop() {
  // Always handle incoming HTTP requests
  server.handleClient();

  // Process sensor data only at specified intervals (e.g., once per second)
  unsigned long now = millis();
  if (now - lastSensorRead >= sensorInterval) {
    lastSensorRead = now;
    
    // Only proceed if at least 17 bytes are available
    if (sensorSerial.available() >= 17) {
      uint8_t buffer[17];
      // readBytes() will now wait at most for the set timeout
      sensorSerial.readBytes(buffer, 17);

      // Check for header: 0x3C, 0x02
      if (buffer[0] == 0x3C && buffer[1] == 0x02) {
        // Parse CO2 (1 ppm)
        uint16_t co2 = (buffer[2] << 8) | buffer[3];

        // Parse CH2O (0.01 mg/m³)
        float ch2o = ((buffer[4] << 8) | buffer[5]) / 100.0;

        // Parse TVOC (0.01 mg/m³)
        float tvoc = ((buffer[6] << 8) | buffer[7]) / 100.0;

        // Parse PM2.5 (µg/m³)
        uint16_t pm25 = (buffer[8] << 8) | buffer[9];

        // Parse PM10 (µg/m³)
        uint16_t pm10 = (buffer[10] << 8) | buffer[11];

        // Parse Temperature:
        // B13: bit7 indicates sign (1 for negative), bits 6..0 the integer part
        // B14: decimal part in hundredths
        uint8_t tempInt = buffer[12]; // B13
        uint8_t tempDec = buffer[13]; // B14

        bool isNegative = (tempInt & 0x80) != 0; // check if sign bit is set
        tempInt &= 0x7F;  // clear the sign bit

        float temperature = tempInt + (tempDec / 100.0);
        if (isNegative) {
          temperature = -temperature;
        }

        // Parse Humidity:
        // B15 = integer part, B16 = decimal part
        uint8_t humInt = buffer[14]; // B15
        uint8_t humDec = buffer[15]; // B16
        float humidity = humInt + (humDec / 100.0);

        // Validate checksum: sum of bytes 0..15 must equal byte 16
        uint8_t calcSum = 0;
        for (int i = 0; i < 16; i++) {
          calcSum += buffer[i];
        }
        uint8_t sensorSum = buffer[16];

        if (calcSum == sensorSum) {
          // Format the sensor data as a string
          sensorOutput = "CO2: " + String(co2) + " ppm, ";
          sensorOutput += "CH2O: " + String(ch2o, 2) + " mg/m^3, ";
          sensorOutput += "TVOC: " + String(tvoc, 2) + " mg/m^3, ";
          sensorOutput += "PM2.5: " + String(pm25) + " µg/m^3, ";
          sensorOutput += "PM10: " + String(pm10) + " µg/m^3, ";
          sensorOutput += "Temp: " + String(temperature, 2) + " °C, ";
          sensorOutput += "Humidity: " + String(humidity, 2) + " %";

          // Print to Serial Monitor
          Serial.println(sensorOutput);

          // Broadcast the data via UDP
          udp.beginPacket(udpAddress, udpPort);
          udp.print(sensorOutput);
          udp.endPacket();
          OLED_Clear();
          OLED_ShowString(1, 1, "pm2 ", 16);
          OLED_ShowNum(33, 1, pm25, 3, 16);
          OLED_ShowString(57, 1, " ", 16);
          OLED_ShowString(65, 1, "ch2 ", 16);
          OLED_ShowNum(97, 1, ch2o, 2, 16);
            OLED_ShowString(1, 17, "pmX ", 16);
            OLED_ShowNum(33, 17, pm10, 3, 16);
            OLED_ShowString(57, 17, " ", 16);
            OLED_ShowString(65, 17, "tvo ", 16);
            OLED_ShowNum(97, 17, tvoc, 2, 16);
            OLED_ShowString(1, 33, "tem ", 16);
            OLED_ShowNum(33, 33, temperature, 3, 16);
            OLED_ShowString(57, 33, " ", 16);
            OLED_ShowString(65, 33, "hum ", 16);
            OLED_ShowNum(97, 33, humidity, 2, 16);
            OLED_ShowString(1, 49, "co2 ", 16);
            OLED_ShowNum(33, 49, co2, 4, 16);
          OLED_Refresh();
        }
        else {
          Serial.println("Checksum mismatch!");
        }
      }
    }
  }
}

void OLED_WR_Byte(uint8_t dat, uint8_t mode) {
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(mode ? 0x40 : 0x00);
    Wire.write(dat);
    Wire.endTransmission();
}

void OLED_ColorTurn(uint8_t i) {
    OLED_WR_Byte(i ? 0xA7 : 0xA6, OLED_CMD);
}

void OLED_DisplayTurn(uint8_t i) {
    OLED_WR_Byte(i ? 0xC0 : 0xC8, OLED_CMD);
    OLED_WR_Byte(i ? 0xA0 : 0xA1, OLED_CMD);
}

void OLED_Refresh() {
    for (uint8_t i = 0; i < 8; i++) {
        OLED_WR_Byte(0xB0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);
        for (uint8_t n = 0; n < 128; n++) {
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
        }
    }
}

void OLED_Clear() {
    memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
    OLED_Refresh();
}

void OLED_ShowNum(uint8_t x, uint8_t y, int num, uint8_t len, uint8_t size1) {
    char str[10];
    snprintf(str, sizeof(str), "%*d", len, num);
    OLED_ShowString(x, y, str, size1);
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size1) {
    while (*chr) {
        OLED_ShowChar(x, y, *chr, size1);
        x += size1 / 2;
        if (x > 128 - size1 / 2) {
            x = 0;
            y += size1;
        }
        chr++;
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size1) {
    uint8_t temp, y0 = y;
    uint8_t size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
    uint8_t chr1 = chr - ' ';
    for (uint8_t i = 0; i < size2; i++) {
        temp = pgm_read_byte(&asc2_1608[chr1][i]);
        for (uint8_t m = 0; m < 8; m++) {
            if (temp & 0x80) OLED_DrawPoint(x, y);
            temp <<= 1;
            y++;
            if ((y - y0) == size1) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void OLED_DrawPoint(uint8_t x, uint8_t y) {
    OLED_GRAM[x][y / 8] |= (1 << (y % 8));
}

void OLED_Init() {
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(0x00);
    Wire.write(0xAE);  // Display OFF
    Wire.write(0xD5);
    Wire.write(0x80);
    Wire.write(0xA8);
    Wire.write(0x3F);
    Wire.write(0xD3);
    Wire.write(0x00);
    Wire.write(0x40);
    Wire.write(0xA1);
    Wire.write(0xC8);
    Wire.write(0xDA);
    Wire.write(0x12);
    Wire.write(0x81);
    Wire.write(0x7F);
    Wire.write(0xA4);
    Wire.write(0xA6);
    Wire.write(0xD9);
    Wire.write(0xF1);
    Wire.write(0xDB);
    Wire.write(0x40);
    Wire.write(0x8D);
    Wire.write(0x14);
    Wire.write(0xAF);  // Display ON
    Wire.endTransmission();
    OLED_Clear();
}
