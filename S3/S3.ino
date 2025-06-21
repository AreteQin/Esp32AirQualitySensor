#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "NavLab";
const char* password = "forflighttest.";

// On-board LED pin (usually GPIO 13 for ESP32-S3 dev boards; adjust if yours differs)
const int LED_PIN = 13;

void setup() {
  // Initialize serial for debug output
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("ESP32-S3 Board Test");

  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Start Wi-Fi connection
  Serial.printf("Connecting to Wi-Fi network: %s\n", ssid);
  WiFi.begin(ssid, password);

  // Wait until connected or timeout after 20 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && 
        (millis() - startAttemptTime) < 20000) {
    blinkLED(100);  // fast blink while trying
    Serial.print(".");
  }
  Serial.println();

  // Check status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ Wi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // slow blinking to indicate success
    blinkLED(500);
  } else {
    Serial.println("❌ Failed to connect to Wi-Fi");
    // rapid blinking to indicate failure
    for (int i = 0; i < 10; i++) {
      blinkLED(100);
      delay(50);
    }
  }
}

void loop() {
  // In the loop, blink the LED once every second
  blinkLED(250);
  delay(750);
}

// Helper to blink the onboard LED once for the given duration
void blinkLED(int ms) {
  digitalWrite(LED_PIN, HIGH);
  delay(ms);
  digitalWrite(LED_PIN, LOW);
}
