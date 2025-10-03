// Walkie-Talkie Transmitter
// Reads audio from an INMP441 I2S microphone and sends it via ESP-NOW.
#include <esp_now.h>
#include <WiFi.h>
#include "driver/i2s.h"
#include "esp_wifi.h"

// =======================================================================
// CONFIGURATION
// =======================================================================

// -- I2S Microphone Pins --
// The pins your INMP441 is connected to.
#define I2S_MIC_WS_PIN    22
#define I2S_MIC_SCK_PIN   26
#define I2S_MIC_SD_PIN    21
#define I2S_PORT_NUMBER   I2S_NUM_0

// -- Audio Settings --
// NOTE: These must match the receiver's settings.
const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;

// -- ESP-NOW Settings --
// The MAC address of the receiver ESP32.
// =======================================================================
// !! IMPORTANT !!
// 1. Upload the `receiver.ino` sketch to your receiver ESP32.
// 2. Open the Serial Monitor for the receiver.
// 3. Copy the MAC address printed in the Serial Monitor.
// 4. Paste it here to replace the placeholder address below.
// =======================================================================
uint8_t receiver_mac_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // <-- REPLACE THIS

// The size of the audio buffer sent in each packet.
// Max is 250 bytes. A smaller size may reduce latency.
const int ESP_NOW_PAYLOAD_SIZE = 250;
uint8_t audio_buffer[ESP_NOW_PAYLOAD_SIZE];

// =======================================================================
// I2S & ESP-NOW SETUP
// =======================================================================

// Callback for when data is sent
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // This is a good place for debugging.
  // You can add Serial.print statements here to check the send status.
}

void setup_i2s_microphone() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = (i2s_bits_per_sample_t)BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 1024,
      .use_apll = false};
  i2s_driver_install(I2S_PORT_NUMBER, &i2s_config, 0, NULL);

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_MIC_SCK_PIN,
      .ws_io_num = I2S_MIC_WS_PIN,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_MIC_SD_PIN};
  i2s_set_pin(I2S_PORT_NUMBER, &pin_config);
  Serial.println("I2S Microphone setup complete.");
}

void setup_esp_now() {
  WiFi.mode(WIFI_STA);

  // Set a specific Wi-Fi channel to ensure both devices match.
  // Both transmitter and receiver must be on the same channel.
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(on_data_sent);

  esp_now_peer_info_t peer_info = {};
  memcpy(peer_info.peer_addr, receiver_mac_address, 6);
  peer_info.channel = 1; // Match the channel
  peer_info.encrypt = false;
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("ESP-NOW setup complete.");
}

void setup() {
  Serial.begin(115200);

  // Use GPIO 23 to select the Right Channel on the INMP441.
  // Connect the L/R pin of the microphone to GPIO 23.
  const int MIC_L_R_SELECT_PIN = 23;
  pinMode(MIC_L_R_SELECT_PIN, OUTPUT);
  digitalWrite(MIC_L_R_SELECT_PIN, HIGH);

  delay(500);
  Serial.println("ESP32 Walkie-Talkie Transmitter");
  setup_i2s_microphone();
  setup_esp_now();
}

// =======================================================================
// MAIN LOOP
// =======================================================================

void loop() {
  size_t bytes_read = 0;
  // Read audio data from the I2S microphone
  esp_err_t result = i2s_read(I2S_PORT_NUMBER, &audio_buffer, ESP_NOW_PAYLOAD_SIZE, &bytes_read, portMAX_DELAY);

  if (result == ESP_OK && bytes_read > 0) {
    // Send the audio data via ESP-NOW to the receiver
    esp_now_send(receiver_mac_address, audio_buffer, bytes_read);
  } else {
    // This can happen if the microphone isn't connected properly.
    Serial.printf("I2S read error: %d\n", result);
  }
}
