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
// Initialize the Sprite object, referencing the TFT object
TFT_eSprite img = TFT_eSprite(&tft);

// Initialize DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

void setup(void) {
  Serial.begin(115200); // Initialize serial communication for debugging

  // --- Control Backlight (Manual) ---
  pinMode(TFT_LED, OUTPUT);     // Set the backlight pin as an output
  digitalWrite(TFT_LED, HIGH);  // Turn the backlight ON (set HIGH)
  // --- End Backlight Control ---

  Serial.println(F("ESP32 TFT_eSPI Sprite & DHT22 Test"));

  // Initialize TFT_eSPI display driver
  tft.init();
  Serial.println(F("TFT Initialized"));

  // Set Display Rotation (0=0, 1=90, 2=180, 3=270 degrees)
  // Rotation 3 makes the 128x160 display behave as 160x128
  tft.setRotation(3); // Set rotation on the physical display
  Serial.println(F("Rotation set to 270 degrees"));

  // Create the sprite with dimensions matching the ROTATED screen
  img.createSprite(160, 128); // Width=160, Height=128 for rotation 1 or 3

  // Initialize DHT sensor
  dht.begin();
  Serial.println(F("DHT22 Initialized"));

  // --- Draw initial static content to SPRITE ---
  img.fillSprite(TFT_BLACK); // Clear sprite background (use fillSprite for full clear)
  img.setCursor(5, 5);       // Coordinates are relative to the sprite
  img.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color (FG, BG) for sprite
  img.setTextSize(1);
  img.println("DHT22 Sensor:");
  // --- End drawing to sprite ---

  // Push the initial sprite content to the screen
  img.pushSprite(0, 0); // Push sprite to TFT at coordinate 0,0
  Serial.println("Initial sprite pushed.");

  delay(500); // Small delay before first reading
}

void loop() {
  // Wait a few seconds between measurements. DHT22 needs >= 2000ms.
  delay(2000);

  // Read temperature and humidity
  float humidity = dht.readHumidity();
  float temperature_c = dht.readTemperature();

  // --- Start Drawing to Sprite ---
  // Clear the entire sprite buffer first (simpler than calculating specific areas)
  img.fillSprite(TFT_BLACK);

  // Re-draw static title text each time since we cleared the sprite
  img.setCursor(5, 5);
  img.setTextColor(TFT_WHITE, TFT_BLACK);
  img.setTextSize(1);
  img.println("DHT22 Sensor:");

  // Check if any reads failed and display error on SPRITE
  if (isnan(humidity) || isnan(temperature_c)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    img.setCursor(5, 20); // Position error message on sprite
    img.setTextColor(TFT_RED, TFT_BLACK); // <-- Use img object, set FG/BG color
    img.setTextSize(2);
    img.println("Read Fail!"); // <-- Use img object

    // Push the sprite content (including the error) to the screen
    img.pushSprite(0, 0); // <-- ADDED Push sprite here for error case

    return; // Exit loop early if read failed
  }

  // --- Display Sensor Data on SPRITE ---

  // Display Temperature (Coordinates relative to sprite)
  img.setCursor(5, 25); // <-- Use img object
  img.setTextColor(TFT_YELLOW, TFT_BLACK); // <-- Use img object
  img.setTextSize(2);
  img.print(F("T:")); // <-- Use img object
  img.print(temperature_c, 1); // <-- Use img object
  img.println(" C"); // <-- Use img object

  // Display Humidity (Coordinates relative to sprite)
  img.setCursor(5, 50); // <-- Use img object
  img.setTextColor(TFT_CYAN, TFT_BLACK); // <-- Use img object
  img.setTextSize(2);
  img.print(F("H:")); // <-- Use img object
  img.print(humidity, 1); // <-- Use img object
  img.println(" %"); // <-- Use img object

  // --- End Display Data ---

  // Push the completed sprite buffer to the physical screen at coordinates (0,0)
  img.pushSprite(0, 0); // <-- ADDED Push sprite here for success case

  // Print data to Serial Monitor as well (remains unchanged)
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature_c);
  Serial.println(F("°C"));
}
