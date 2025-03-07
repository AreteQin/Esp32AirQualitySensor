#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Wire.h>
#include "font.h"

#define OLED_ADDRESS 0x3C // I2C address for the SSD1306 OLED
#define SDA_PIN 21        // ESP32 SDA pin
#define SCL_PIN 22        // ESP32 SCL pin
#define OLED_CMD 0        // Command mode
#define OLED_DATA 1       // Data mode
uint8_t OLED_GRAM[128][8];

// WiFi credentials – replace with your network info
const char *ssid = ".";
const char *password = ".";

// UDP broadcast settings
WiFiUDP udp;
const char *udpAddress = "255.255.255.255"; // adjust if needed for your network
const int udpPort = 4210;

// Create an HTTP server on port 80
WebServer server(80);

// Global variable to store the latest sensor data as a string
String sensorOutput = "No data available";

// Global variable to store the history of sensor data
int co2_history[100];
int pm25_history[100];
int pm10_history[100];
int ch2o_history[100];
int tvoc_history[100];
int temp_history[100];
int hum_history[100];

// HardwareSerial instance for sensor communication on UART2
HardwareSerial sensorSerial(2);

// HTTP handler: serve the HTML dashboard with multiple sensor plots
void handleRoot()
{
  // Build JavaScript arrays for each sensor history
  String co2Data = "[";
  String pm25Data = "[";
  String pm10Data = "[";
  String ch2oData = "[";
  String tvocData = "[";
  String tempData = "[";
  String humData = "[";
  for (int i = 0; i < 100; i++)
  {
    co2Data += String(co2_history[i]);
    pm25Data += String(pm25_history[i]);
    pm10Data += String(pm10_history[i]);
    ch2oData += String(ch2o_history[i]);
    tvocData += String(tvoc_history[i]);
    tempData += String(temp_history[i]);
    humData += String(hum_history[i]);
    if (i < 99)
    {
      co2Data += ",";
      pm25Data += ",";
      pm10Data += ",";
      ch2oData += ",";
      tvocData += ",";
      tempData += ",";
      humData += ",";
    }
  }
  co2Data += "]";
  pm25Data += "]";
  pm10Data += "]";
  ch2oData += "]";
  tvocData += "]";
  tempData += "]";
  humData += "]";

  // Construct the full HTML page with the embedded sensor data,
  // including the latest sensor reading on the main page.
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Air Quality Sensor Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: black; color: white; }
    nav { background-color: #333; padding: 10px; }
    nav a { color: white; text-decoration: none; margin: 10px; padding: 10px; display: inline-block; }
    nav a:hover { background-color: #555; }
    .page { display: none; padding: 20px; }
    .active { display: block; }
    svg { border: 1px solid #888; background-color: #222; }
    text { font-size: 14px; fill: white; font-family: Arial, sans-serif; }
  </style>
</head>
<body>
  <nav>
    <a href="#" onclick="showPage('home')">Home</a>
    <a href="#" onclick="showPage('co2')">CO2</a>
    <a href="#" onclick="showPage('pm25')">PM2.5</a>
    <a href="#" onclick="showPage('pm10')">PM10</a>
    <a href="#" onclick="showPage('ch2o')">CH2O</a>
    <a href="#" onclick="showPage('tvoc')">TVOC</a>
    <a href="#" onclick="showPage('temperature')">Temperature</a>
    <a href="#" onclick="showPage('humidity')">Humidity</a>
    <a href="#" onclick="showPage('about')">About</a>
    <a href="#" onclick="showPage('contact')">Contact</a>
  </nav>
  <div id="home" class="page active">
    <h1>Welcome to Air Quality Sensor</h1>
    <p>This is the main page of the website.</p>
    <p id="sensorData">Latest Sensor Data: )rawliteral" +
                sensorOutput + R"rawliteral(</p>
  </div>
  <div id="co2" class="page">
    <h1>CO2</h1>
    <svg id="plot-co2" width="500" height="300"></svg>
  </div>
  <div id="pm25" class="page">
    <h1>PM2.5</h1>
    <svg id="plot-pm25" width="500" height="300"></svg>
  </div>
  <div id="pm10" class="page">
    <h1>PM10</h1>
    <svg id="plot-pm10" width="500" height="300"></svg>
  </div>
  <div id="ch2o" class="page">
    <h1>CH2O</h1>
    <svg id="plot-ch2o" width="500" height="300"></svg>
  </div>
  <div id="tvoc" class="page">
    <h1>TVOC</h1>
    <svg id="plot-tvoc" width="500" height="300"></svg>
  </div>
  <div id="temperature" class="page">
    <h1>Temperature</h1>
    <svg id="plot-temperature" width="500" height="300"></svg>
  </div>
  <div id="humidity" class="page">
    <h1>Humidity</h1>
    <svg id="plot-humidity" width="500" height="300"></svg>
  </div>
  <div id="about" class="page">
    <h1>About Us</h1>
    <p>Learn more about our products.</p>
  </div>
  <div id="contact" class="page">
    <h1>Contact Us</h1>
    <p>Reach out to us through email:</p>
    <p>qinqiaomeng@outlook.com</p>
  </div>
  <script>
    function showPage(pageId) {
      var pages = document.querySelectorAll(".page");
      pages.forEach(page => page.classList.remove("active"));
      document.getElementById(pageId).classList.add("active");
    }
    function drawPlot(sensorData, elementId) {
      let svg = document.getElementById(elementId);
      svg.innerHTML = "";
      let offsetX = 50, offsetY = 250, width = 400, height = 200;
      let validValues = sensorData.filter(v => v > 0);
      let maxValue = validValues.length > 0 ? Math.max(...validValues) : 1;
      let scaleX = width / (sensorData.length - 1);
      let scaleY = maxValue > 0 ? height / maxValue : 1;
      svg.innerHTML += `
        <line x1="50" y1="250" x2="450" y2="250" stroke="gray" stroke-width="2"></line>
        <line x1="50" y1="50" x2="50" y2="250" stroke="gray" stroke-width="2"></line>
        <text x="20" y="50">${maxValue}</text>
        <text x="20" y="250">0</text>
      `;
      let points = "";
      for (let i = 0; i < sensorData.length; i++) {
        if (sensorData[i] > 0) {
          let x = offsetX + i * scaleX;
          let y = offsetY - sensorData[i] * scaleY;
          points += `<circle cx="${x}" cy="${y}" r="3" fill="red"></circle>`;
        }
      }
      svg.innerHTML += points;
    }
    // Sensor data arrays dynamically injected from ESP32
    let co2Data = )rawliteral" +
                co2Data + R"rawliteral(;
    let pm25Data = )rawliteral" +
                pm25Data + R"rawliteral(;
    let pm10Data = )rawliteral" +
                pm10Data + R"rawliteral(;
    let ch2oData = )rawliteral" +
                ch2oData + R"rawliteral(;
    let tvocData = )rawliteral" +
                tvocData + R"rawliteral(;
    let tempData = )rawliteral" +
                tempData + R"rawliteral(;
    let humData = )rawliteral" +
                humData + R"rawliteral(;
    // Draw the plots for each sensor page
    drawPlot(co2Data, "plot-co2");
    drawPlot(pm25Data, "plot-pm25");
    drawPlot(pm10Data, "plot-pm10");
    drawPlot(ch2oData, "plot-ch2o");
    drawPlot(tvocData, "plot-tvoc");
    drawPlot(tempData, "plot-temperature");
    drawPlot(humData, "plot-humidity");
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(" connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(" connection failed!");
  }

  // Start UDP on the chosen port
  udp.begin(udpPort);

  // Setup HTTP server route
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize sensor UART (assumed 9600, RX=16, TX=17)
  sensorSerial.begin(9600, SERIAL_8N1, 16, 17);
  sensorSerial.setTimeout(10);

  Serial.println("7-in-1 Sensor");
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with ESP32 pins
  OLED_Init();
  OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
}

int data_index = 0; // Current index for storing sensor history

void loop()
{
  // Always handle incoming HTTP requests
  server.handleClient();

  if (sensorSerial.available() >= 17)
  {
    uint8_t buffer[17];
    sensorSerial.readBytes(buffer, 17);
    if (buffer[0] == 0x3C && buffer[1] == 0x02)
    {
      // Parse sensor values
      uint16_t co2 = (buffer[2] << 8) | buffer[3];
      float ch2o = ((buffer[4] << 8) | buffer[5]) / 100.0;
      float tvoc = ((buffer[6] << 8) | buffer[7]) / 100.0;
      uint16_t pm25 = (buffer[8] << 8) | buffer[9];
      uint16_t pm10 = (buffer[10] << 8) | buffer[11];
      uint8_t tempInt = buffer[12]; // Temperature integer part
      uint8_t tempDec = buffer[13]; // Temperature decimal part
      bool isNegative = (tempInt & 0x80) != 0;
      tempInt &= 0x7F;
      float temperature = tempInt + (tempDec / 100.0);
      if (isNegative)
        temperature = -temperature;
      uint8_t humInt = buffer[14];
      uint8_t humDec = buffer[15];
      float humidity = humInt + (humDec / 100.0);

      // Validate checksum: sum of bytes 0..15 must equal byte 16
      uint8_t calcSum = 0;
      for (int i = 0; i < 16; i++)
      {
        calcSum += buffer[i];
      }
      if (calcSum == buffer[16])
      {
        // Format sensor output string for debugging
        sensorOutput = "CO2: " + String(co2) + " ppm, ";
        sensorOutput += "CH2O: " + String(ch2o, 2) + " mg/m^3, ";
        sensorOutput += "TVOC: " + String(tvoc, 2) + " mg/m^3, ";
        sensorOutput += "PM2.5: " + String(pm25) + " µg/m^3, ";
        sensorOutput += "PM10: " + String(pm10) + " µg/m^3, ";
        sensorOutput += "Temp: " + String(temperature, 2) + " °C, ";
        sensorOutput += "Humidity: " + String(humidity, 2) + " %";
        Serial.println(sensorOutput);
        // Broadcast sensor data via UDP
        udp.beginPacket(udpAddress, udpPort);
        udp.print(sensorOutput);
        udp.endPacket();
        // Update OLED display (functions defined below)
        OLED_Clear();
        if (WiFi.status() == WL_CONNECTED)
        {
          OLED_ShowString(73, 1, "IP ", 16);
          OLED_ShowNum(97, 1, WiFi.localIP()[3], 3, 16);
        }
        OLED_ShowString(1, 1, "co2 ", 16);
        OLED_ShowNum(33, 1, co2, 4, 16);
        OLED_ShowString(1, 17, "pm2 ", 16);
        OLED_ShowNum(33, 17, pm25, 3, 16);
        OLED_ShowString(65, 17, "ch2 ", 16);
        OLED_ShowNum(97, 17, ch2o, 2, 16);
        OLED_ShowString(1, 33, "pmX ", 16);
        OLED_ShowNum(33, 33, pm10, 3, 16);
        OLED_ShowString(65, 33, "tvo ", 16);
        OLED_ShowNum(97, 33, tvoc, 2, 16);
        OLED_ShowString(1, 49, "tem ", 16);
        OLED_ShowNum(33, 49, temperature, 3, 16);
        OLED_ShowString(65, 49, "hum ", 16);
        OLED_ShowNum(97, 49, humidity, 2, 16);
        OLED_Refresh();
        // Store sensor readings into history arrays using current index (adjust as needed)
        co2_history[data_index] = co2;
        pm25_history[data_index] = pm25;
        pm10_history[data_index] = pm10;
        ch2o_history[data_index] = ch2o;
        tvoc_history[data_index] = tvoc;
        temp_history[data_index] = temperature;
        hum_history[data_index] = humidity;
        if (++data_index >= 100)
        {
          data_index = 0; // Wrap around to the beginning
          for (int i = 0; i < 100; i++)
          {
            co2_history[i] = 0;
            pm25_history[i] = 0;
            pm10_history[i] = 0;
            ch2o_history[i] = 0;
            tvoc_history[i] = 0;
            temp_history[i] = 0;
            hum_history[i] = 0;
          }
        }
      }
      else
      {
        Serial.println("Checksum mismatch!");
      }
    }
  }
}

void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(mode ? 0x40 : 0x00);
  Wire.write(dat);
  Wire.endTransmission();
}

void OLED_ColorTurn(uint8_t i)
{
  OLED_WR_Byte(i ? 0xA7 : 0xA6, OLED_CMD);
}

void OLED_DisplayTurn(uint8_t i)
{
  OLED_WR_Byte(i ? 0xC0 : 0xC8, OLED_CMD);
  OLED_WR_Byte(i ? 0xA0 : 0xA1, OLED_CMD);
}

void OLED_Refresh()
{
  for (uint8_t i = 0; i < 8; i++)
  {
    OLED_WR_Byte(0xB0 + i, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    for (uint8_t n = 0; n < 128; n++)
    {
      OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    }
  }
}

void OLED_Clear()
{
  memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
  OLED_Refresh();
}

void OLED_ShowNum(uint8_t x, uint8_t y, int num, uint8_t len, uint8_t size1)
{
  char str[10];
  snprintf(str, sizeof(str), "%*d", len, num);
  OLED_ShowString(x, y, str, size1);
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size1)
{
  while (*chr)
  {
    OLED_ShowChar(x, y, *chr, size1);
    x += size1 / 2;
    if (x > 128 - size1 / 2)
    {
      x = 0;
      y += size1;
    }
    chr++;
  }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size1)
{
  uint8_t temp, y0 = y;
  uint8_t size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
  uint8_t chr1 = chr - ' ';
  for (uint8_t i = 0; i < size2; i++)
  {
    temp = pgm_read_byte(&asc2_1608[chr1][i]);
    for (uint8_t m = 0; m < 8; m++)
    {
      if (temp & 0x80)
        OLED_DrawPoint(x, y);
      temp <<= 1;
      y++;
      if ((y - y0) == size1)
      {
        y = y0;
        x++;
        break;
      }
    }
  }
}

void OLED_DrawPoint(uint8_t x, uint8_t y)
{
  OLED_GRAM[x][y / 8] |= (1 << (y % 8));
}

void OLED_Init()
{
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(0x00);
  Wire.write(0xAE); // Display OFF
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
  Wire.write(0xAF); // Display ON
  Wire.endTransmission();
  OLED_Clear();
}
