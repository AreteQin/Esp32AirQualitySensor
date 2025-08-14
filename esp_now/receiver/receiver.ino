// Walkie-Talkie Receiver (with Debugging)
// Receives audio via ESP-NOW and plays it on a MAX98357A I2S amplifier.
// Board: ESP32 with MAX98357A

#include <esp_now.h>
#include <WiFi.h>
#include "driver/i2s.h"

// =================================================================================
// CONFIGURATION
// =================================================================================

// == I2S/Amplifier Configuration ==
#define I2S_PORT_NUMBER I2S_NUM_0
#define I2S_SPEAKER_BCLK_PIN  27
#define I2S_SPEAKER_LRC_PIN   26
#define I2S_SPEAKER_DIN_PIN   25

// == Audio Configuration ==
const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;
// =================================================================================

// Callback function executed when data is received via ESP-NOW
void on_data_received(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  // [DEBUG] Print a dot for every packet received. If you don't see this,
  // the receiver isn't getting any data.
  Serial.print(".");
  
  size_t bytes_written = 0;
  esp_err_t result = i2s_write(I2S_PORT_NUMBER, data, len, &bytes_written, portMAX_DELAY);
  
  if (result != ESP_OK) {
    Serial.printf("\nI2S write error: %d\n", result);
  }
}

void setup_i2s_speaker() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SPEAKER_BCLK_PIN,
    .ws_io_num = I2S_SPEAKER_LRC_PIN,
    .data_out_num = I2S_SPEAKER_DIN_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_PORT_NUMBER, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT_NUMBER, &pin_config);
  Serial.println("I2S Speaker setup complete.");
}

void setup_esp_now() {
  WiFi.mode(WIFI_AP);
  
  // [DEBUG] Set a specific Wi-Fi channel to ensure both devices match.
  WiFi.softAP("ESP32_Receiver_AP", nullptr, 1);
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Set Wi-Fi channel to 1");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(on_data_received);
  Serial.println("ESP-NOW setup complete. Waiting for audio...");
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 Walkie-Talkie Receiver (Debugging Mode)");
  setup_i2s_speaker();
  setup_esp_now();
}

void loop() {
  // Main work happens in the on_data_received callback.
  delay(2000); // We can delay longer here during debug.
}
