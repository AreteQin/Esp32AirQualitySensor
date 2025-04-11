#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <DHT.h>             // Include DHT sensor library

// Define ESP32 Pins connected to the ST7735 display
#define TFT_CS        5  // Chip select pin -> ESP32 GPIO5
#define TFT_RST       4  // Reset pin -> ESP32 GPIO4
#define TFT_DC        2  // Data/Command (AO/RS) pin -> ESP32 GPIO2
#define TFT_LED      15  // Backlight LED pin -> ESP32 GPIO15

// Define DHT Sensor Pin and Type
#define DHTPIN       27  // Data pin of DHT22 connected to GPIO27
#define DHTTYPE      DHT22 // Define sensor type as DHT22

// Initialize Adafruit ST7735 library using hardware SPI
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Initialize DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

void setup(void) {
  Serial.begin(115200); // Initialize serial communication for debugging

  // --- Control Backlight ---
  pinMode(TFT_LED, OUTPUT);     // Set the backlight pin as an output
  digitalWrite(TFT_LED, HIGH);  // Turn the backlight ON (set HIGH)
  // --- End Backlight Control ---

  Serial.println(F("ESP32 ST7735 & DHT22 Test"));

  // Initialize ST7735 display driver
  tft.initR(INITR_BLACKTAB);
  Serial.println(F("TFT Initialized"));

  // --- Set Display Rotation --- <<-- ADDED THIS LINE
  tft.setRotation(1); // Rotate display 90 degrees clockwise
  // --- End Rotation ---

  // Initialize DHT sensor
  dht.begin();
  Serial.println(F("DHT22 Initialized"));

  // Prepare initial display screen
  tft.fillScreen(ST77XX_BLACK); // Clear screen
  tft.setCursor(5, 5); // Note: Coordinates are relative to the NEW rotation
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("DHT22 Sensor:"); // Adjusted label slightly
  delay(500); // Small delay before first reading
}

void loop() {
  // Wait a few seconds between measurements. DHT22 needs >= 2 seconds.
  delay(2000);

  // Read temperature and humidity
  float humidity = dht.readHumidity();
  float temperature_c = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature_c)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    // Display error on TFT (Adjust coordinates if needed for rotation)
    tft.fillRect(5, 20, tft.width() - 10, 40, ST77XX_BLACK); // Clear previous data area
    tft.setCursor(5, 20);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.println("Read Fail!");
    return; // Exit loop early if read failed
  }

  // --- Display Data on TFT ---

  // Clear the area where sensor data will be displayed
  // Note: tft.width() and tft.height() swap values after rotation!
  // Adjust coordinates and dimensions as needed for the rotated layout.
  tft.fillRect(5, 20, tft.width() - 10, 60, ST77XX_BLACK);

  // Display Temperature (Adjust coordinates for rotated layout)
  tft.setCursor(5, 25);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.print(F("T:")); // Shortened label for rotated view
  tft.print(temperature_c, 1);
  tft.print((char)247); // Degree symbol
  tft.println("C");

  // Display Humidity (Adjust coordinates for rotated layout)
  tft.setCursor(5, 50);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.print(F("H:")); // Shortened label
  tft.print(humidity, 1);
  tft.println(" %");

  // --- End Display Data ---

  // Print data to Serial Monitor as well
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature_c);
  Serial.println(F("°C"));
}
