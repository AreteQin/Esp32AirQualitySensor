#include <driver/i2s.h>
#include <math.h>

// sound
const unsigned int sound_data_len = 7168;
const int16_t sound_data[] = {
  -24, -33, -3, 11, 2, -18, -13, 22, 23, -16, -26, -5, 23, 14, -18, -21,
  6, 30, 8, -19, -21, 5, 28, 12, -19, -26, 0, 31, 16, -21, -29, -5,
  30, 20, -21, -33, -7, 34, 25, -20, -36, -11, 35, 28, -22, -37, -13,
  // ... (data continues for over 7000 samples)
  // NOTE: This is a heavily truncated sample. The full data is much larger.
  // The provided code in the next step will contain the full array.
  // For now, imagine the rest of the data is here.
  0, 0, 0, 0, 0, 0, 0, 0
};

// I²S pin definitions (match your wiring)
#define I2S_DOUT   25    // DATA output (DIN on MAX98357A)
#define I2S_BCLK   27    // Bit clock (BCLK)
#define I2S_LRC    26    // Word select (LRCLK/WS)
#define SD_PIN     5     // Shutdown Pin

#define I2S_PORT   I2S_NUM_0

// The sample rate must match the sound file!
const int sampleRate = 16000;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 - I2S Sound Effect Player for MAX98357A");

  // If you are using the SHUTDOWN pin, configure it.
  #ifdef SD_PIN
    pinMode(SD_PIN, OUTPUT);
    digitalWrite(SD_PIN, HIGH); // Set HIGH to enable the amplifier
  #endif

  // Configure and install the I²S driver
  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    // CRITICAL CHANGE FOR MAX98357A!
    // Use Left-Justified (MSB) format instead of standard I2S
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = true
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  // Define the physical GPIO pins to be used
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_set_pin(I2S_PORT, &pin_config);
}

void loop() {
  Serial.println("Playing sound...");

  // Write the sound data from the header file to the I2S port
  size_t bytes_written = 0;
  i2s_write(I2S_PORT, sound_data, sizeof(sound_data), &bytes_written, portMAX_DELAY);

  Serial.println("Sound finished. Waiting 5 seconds...");
  delay(5000); // Wait 5 seconds before playing again
}
