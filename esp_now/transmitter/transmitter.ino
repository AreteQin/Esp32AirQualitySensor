// Walkie-Talkie Transmitter (with Debugging)
// Reads audio from an INMP441 I2S microphone and sends it via ESP-NOW.
// Board: ESP32 with INMP441

#include <esp_now.h>
#include <WiFi.h>
#include "driver/i2s.h"
#include "esp_wifi.h" // Needed for esp_wifi_set_channel

// =================================================================================
// CONFIGURATION
// =================================================================================

// == I2S/Microphone Configuration ==
#define I2S_PORT_NUMBER I2S_NUM_0
#define I2S_MIC_WS_PIN    22
#define I2S_MIC_SCK_PIN   26
#define I2S_MIC_SD_PIN    21

// == Audio Configuration ==
const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;

// == ESP-NOW Configuration ==
const int ESP_NOW_PAYLOAD_SIZE = 250;
uint8_t audio_buffer[ESP_NOW_PAYLOAD_SIZE] = {0};
// MAC Address of the receiver board.
// IMPORTANT: Double-check this matches the receiver's output.
uint8_t receiver_mac_address[] = {0x34,0x98,0x7A,0xB7,0xD7,0x35};
// =================================================================================

// Callback function executed when data is sent
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // [DEBUG] This will tell us if packets are being sent successfully.
  // Look for "Success". If you see "Fail", check the receiver's MAC address.
  Serial.print("Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup_i2s_microphone() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_MIC_SCK_PIN,
    .ws_io_num = I2S_MIC_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD_PIN
  };
  i2s_driver_install(I2S_PORT_NUMBER, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT_NUMBER, &pin_config);
  Serial.println("I2S Microphone setup complete.");
}

void setup_esp_now() {
  WiFi.mode(WIFI_STA);

  // [DEBUG] Set a specific Wi-Fi channel to ensure both devices match.
  // Both transmitter and receiver must be on the same channel.
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  Serial.println("Set Wi-Fi channel to 1");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(on_data_sent);
  esp_now_peer_info_t peer_info = {};
  memcpy(peer_info.peer_addr, receiver_mac_address, 6);
  peer_info.channel = 1; // Set channel for peer
  peer_info.encrypt = false;
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("ESP-NOW setup complete.");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 Walkie-Talkie Transmitter (Debugging Mode)");
  setup_i2s_microphone();
  setup_esp_now();
}

void loop() {
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(I2S_PORT_NUMBER, &audio_buffer, ESP_NOW_PAYLOAD_SIZE, &bytes_read, portMAX_DELAY);

  if (result == ESP_OK && bytes_read > 0) {
    // [DEBUG] Check if the microphone is sending silence.
    // If the sample value is always 0 or very low, check mic wiring/power.
    // int16_t first_sample = (audio_buffer[1] << 8) | audio_buffer[0];
    // Serial.printf("First audio sample: %d\n", first_sample);
    
    esp_now_send(receiver_mac_address, audio_buffer, bytes_read);
  } else {
    Serial.printf("I2S read error: %d\n", result);
  }
}
