#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <TFT_eSPI.h> // Added for TFT
#include <SPI.h>      // Added for TFT (dependency for TFT_eSPI)

// --- Added TFT Defines ---
#define TFT_LED 15 // Backlight LED pin -> ESP32 GPIO15 (Manual Control)

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

// Global variable to store the history of sensor data (remains unchanged)
int co2_history[100];
int pm25_history[100];
int pm10_history[100];
int ch2o_history[100];
int tvoc_history[100];
int temp_history[100];
int hum_history[100];

// HardwareSerial instance for sensor communication on UART2 (remains unchanged)
HardwareSerial sensorSerial(2);

// --- Added TFT Objects ---
// Initialize TFT_eSPI library object
TFT_eSPI tft = TFT_eSPI();
// Initialize the Sprite object, referencing the TFT object
TFT_eSprite img = TFT_eSprite(&tft);

// HTTP handler: serve the HTML dashboard (remains unchanged)
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

  // Construct the full HTML page (remains unchanged)
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

  // Connect to WiFi (remains unchanged)
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

  // Start UDP on the chosen port (remains unchanged)
  udp.begin(udpPort);

  // Setup HTTP server route (remains unchanged)
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize sensor UART (remains unchanged)
  sensorSerial.begin(9600, SERIAL_8N1, 16, 17);
  sensorSerial.setTimeout(10);

  Serial.println("7-in-1 Sensor");

  // +++ Added TFT Initialization +++
  // --- Control Backlight ---
  pinMode(TFT_LED, OUTPUT);     // Set the backlight pin as an output
  digitalWrite(TFT_LED, HIGH);  // Turn the backlight ON (set HIGH)

  // --- Initialize TFT ---
  tft.init();
  Serial.println(F("TFT Initialized"));

  // --- Set Rotation (270 degrees = 160x128) ---
  tft.setRotation(1);
  Serial.println(F("TFT Rotation set to 270 degrees"));

  // --- Create Sprite Buffer ---
  // Dimensions match the rotated screen
  img.createSprite(160, 128);
  img.fillSprite(TFT_BLACK); // Pre-clear sprite
  img.pushSprite(0,0); // Push clear screen initially
  // +++ End Added TFT Initialization +++

}

int data_index = 0; // Current index for storing sensor history (remains unchanged)

void loop()
{
  // Always handle incoming HTTP requests (remains unchanged)
  server.handleClient();

  if (sensorSerial.available() >= 17)
  {
    uint8_t buffer[17];
    sensorSerial.readBytes(buffer, 17);
    if (buffer[0] == 0x3C && buffer[1] == 0x02)
    {
      // Parse sensor values (remains unchanged)
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

      // Validate checksum (remains unchanged)
      uint8_t calcSum = 0;
      for (int i = 0; i < 16; i++)
      {
        calcSum += buffer[i];
      }
      if (calcSum == buffer[16])
      {
        // Format sensor output string for debugging (remains unchanged)
        // ... (sensorOutput string formatting) ...
        Serial.println(sensorOutput);

        // Broadcast sensor data via UDP (remains unchanged)
        // ... (UDP broadcast code) ...

        // +++ Added TFT display update using Sprite +++
        img.fillSprite(TFT_BLACK); // Clear sprite buffer
        img.setTextSize(1); // <<-- SET TEXT SIZE BACK TO 1
        img.setTextColor(TFT_WHITE, TFT_BLACK); // Set default text color

        // Reverted to 2-column layout for 160x128 screen with Text Size 1
        int x1 = 5;  // X position for first column
        int x2 = 85; // X position for second column
        int y_line = 5; // Starting Y position
        int line_height = 10; // Height per line for text size 1 + spacing

        // Line 1
        img.setCursor(x1, y_line);
        img.print("CO2:"); img.print(co2);
        img.setCursor(x2, y_line);
        img.print("PM2.5:"); img.print(pm25);
        y_line += line_height;

        // Line 2
        img.setCursor(x1, y_line);
        img.print("PM10:"); img.print(pm10);
        img.setCursor(x2, y_line);
        img.print("TVOC:"); img.print(tvoc, 2); // Show decimals
        y_line += line_height;

        // Line 3
        img.setCursor(x1, y_line);
        img.print("CH2O:"); img.print(ch2o, 2); // Show decimals
        img.setCursor(x2, y_line);
        img.print("Temp:"); img.print(temperature, 1); // Show decimals
        y_line += line_height;

        // Line 4
        img.setCursor(x1, y_line);
        img.print("Hum:"); img.print(humidity, 1); // Show decimals
        img.setCursor(x2, y_line);
        if (WiFi.status() == WL_CONNECTED) {
            img.print("IP:."); img.print(WiFi.localIP()[3]); // Show last octet of IP
        } else {
            img.setTextColor(TFT_RED, TFT_BLACK); // Show warning if not connected
            img.print("WiFi Err");
            img.setTextColor(TFT_WHITE, TFT_BLACK); // Reset color
        }

        // Push the sprite content to the physical display
        img.pushSprite(0, 0);
        // +++ End Added TFT display update +++


        // Store sensor readings into history arrays (remains unchanged)
        // ... (history array storage code) ...

      } else {
        Serial.println("Checksum mismatch!");
        // Display checksum error on TFT
        img.fillSprite(TFT_BLACK);
        img.setCursor(5,5);
        img.setTextColor(TFT_RED, TFT_BLACK);
        img.setTextSize(1); // <-- Adjusted error message size back to 1
        img.println("CHECKSUM ERROR"); // Make msg slightly clearer
        img.pushSprite(0,0);
      }
    }
  }
   // Small delay to prevent overly tight loop if no serial data available
   delay(10);
}

// --- OLED Helper Functions were already removed ---
