/*
  ESP32 I2S AI-Generated Song Player
  FIXED: Actively outputs silence during rests to eliminate noise.
*/
#include "Arduino.h"
#include "driver/i2s.h"
#include "math.h"

// I2S Connections
#define I2S_DOUT      22
#define I2S_BCLK      26
#define I2S_LRC       25

// I2S Settings
#define I2S_PORT      I2S_NUM_0
#define SAMPLE_RATE   44100
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT

// --- Define the Song Structure ---
struct Note {
  int frequency;
  int duration;
};

// --- Note and Rhythm Definitions ---
#define NOTE_REST 0
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

#define TEMPO 120
const int QUARTER_NOTE = 60000 / TEMPO;
const int HALF_NOTE = QUARTER_NOTE * 2;
const int EIGHTH_NOTE = QUARTER_NOTE / 2;
const int SIXTEENTH_NOTE = QUARTER_NOTE / 4;

// --- The AI-Generated Song ---
Note aiSong[] = {
  {NOTE_E4, EIGHTH_NOTE},
  {NOTE_B4, SIXTEENTH_NOTE},
  {NOTE_C5, QUARTER_NOTE},
  {NOTE_A4, EIGHTH_NOTE},
  {NOTE_REST, SIXTEENTH_NOTE}, // This rest will now be silent
  {NOTE_A4, SIXTEENTH_NOTE},
  {NOTE_G4, EIGHTH_NOTE},
  {NOTE_E4, EIGHTH_NOTE},
  {NOTE_G4, EIGHTH_NOTE},
  {NOTE_C5, HALF_NOTE},
  {NOTE_REST, QUARTER_NOTE} // This rest will also be silent
};


// Function to generate and play a single note or rest
void playNote(int frequency, int duration_ms) {
    // --- FIX: Handle rests by sending silence ---
    if (frequency == 0) {
        // Clear the I2S buffer to ensure silence is played
        i2s_zero_dma_buffer(I2S_PORT);
        delay(duration_ms);
        return;
    }
  
    int sample_rate = SAMPLE_RATE;
    int num_samples = duration_ms * (sample_rate / 1000);
    int16_t *samples = (int16_t *)malloc(num_samples * sizeof(int16_t) * 2);

    if (!samples) {
        Serial.println("Error: Failed to allocate memory for samples");
        return;
    }

    double sin_value;
    int16_t sample_value;
    for (int i = 0; i < num_samples; i++) {
        sin_value = sin(2 * PI * frequency * i / sample_rate);
        sample_value = (int16_t)(sin_value * INT16_MAX * 0.1);
        samples[i * 2] = sample_value;
        samples[i * 2 + 1] = sample_value;
    }

    size_t bytes_written = 0;
    i2s_write(I2S_PORT, samples, num_samples * sizeof(int16_t) * 2, &bytes_written, portMAX_DELAY);
    free(samples);
}


void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 I2S Song Player - Noise Fixed");

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
}

void loop() {
    Serial.println("Playing song...");
    int numNotes = sizeof(aiSong) / sizeof(Note);

    for (int i = 0; i < numNotes; i++) {
        Serial.printf("Playing note %d: Freq=%d, Dur=%d\n", i + 1, aiSong[i].frequency, aiSong[i].duration);
        playNote(aiSong[i].frequency, aiSong[i].duration);
    }
    
    Serial.println("Song finished. Repeating in 2 seconds...");
    
    // --- FIX: Clear the I2S buffer before the long pause ---
    i2s_zero_dma_buffer(I2S_PORT);
    delay(2000); 
}
