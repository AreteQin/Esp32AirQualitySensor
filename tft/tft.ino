#include <TFT_eSPI.h>        // Include TFT_eSPI library
#include <SPI.h>
#include <DHT.h>             // Include DHT sensor library

// TFT pins (like CS, DC, RST) are defined in TFT_eSPI's User_Setup.h now!
// We only need to define pins not directly controlled by TFT_eSPI library here.
#define TFT_LED      15  // Backlight LED pin -> ESP32 GPIO15 (Manual Control)

// Define DHT Sensor Pin and Type
#define DHTPIN       27  // Data pin of DHT22 connected to GPIO27
#define DHTTYPE      DHT22 // Define sensor type as DHT22

// Initialize TFT_eSPI library object
TFT_eSPI tft = TFT_eSPI();

// Initialize DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

void setup(void) {
  Serial.begin(115200); // Initialize serial communication for debugging

  // --- Control Backlight (Manual) ---
  pinMode(TFT_LED, OUTPUT);     // Set the backlight pin as an output
  digitalWrite(TFT_LED, HIGH);  // Turn the backlight ON (set HIGH)
  // --- End Backlight Control ---

  Serial.println(F("ESP32 TFT_eSPI & DHT22 Test"));

  // Initialize TFT_eSPI display driver
  tft.init();
  Serial.println(F("TFT Initialized"));

  // Set Display Rotation (0=0, 1=90, 2=180, 3=270 degrees)
  tft.setRotation(3); // Rotate display 180 degrees <-- CHANGED
  Serial.println(F("Rotation set to 180 degrees"));

  // Initialize DHT sensor
  dht.begin();
  Serial.println(F("DHT22 Initialized"));

  // Prepare initial display screen
  tft.fillScreen(TFT_BLACK); // Clear screen
  tft.setCursor(5, 5); // Note: Coordinates are relative to the NEW rotation (top-left is now bottom-right)
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println("DHT22 Sensor:");
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
    tft.fillRect(5, 20, tft.width() - 10, 40, TFT_BLACK); // Clear area
    tft.setCursor(5, 20);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.println("Read Fail!");
    return; // Exit loop early if read failed
  }

  // --- Display Data on TFT ---

  // Clear the area where sensor data will be displayed
  // Note: For 180 degree rotation, width/height remain the same as 0 degrees
  // but the origin (0,0) is now effectively at the bottom-right corner
  // of the original orientation.
  tft.fillRect(5, 20, tft.width() - 10, 60, TFT_BLACK);

  // Display Temperature (Coordinates might need adjusting for desired 180deg layout)
  tft.setCursor(5, 25);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.print(F("T:"));
  tft.print(temperature_c, 1);
  tft.println(" C");

  // Display Humidity (Coordinates might need adjusting for desired 180deg layout)
  tft.setCursor(5, 50);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.print(F("H:"));
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
