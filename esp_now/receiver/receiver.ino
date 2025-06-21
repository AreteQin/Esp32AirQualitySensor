#include <WiFi.h>
#include <esp_now.h>

// data structure matching the sender’s
typedef struct struct_message {
  int number;
} struct_message;

struct_message incomingData;

// This is the correct callback signature for ESP-NOW receive
void onDataRecv(const esp_now_recv_info_t *recv_info,
                const uint8_t *data,
                int len) {
  // ignore packets of the wrong size
  if (len != sizeof(incomingData)) return;

  // copy payload into our struct
  memcpy(&incomingData, data, sizeof(incomingData));

  // print sender MAC
  Serial.print("Received from ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", recv_info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }

  // print the number
  Serial.print(" -> ");
  Serial.println(incomingData.number);
}

void setup() {
  Serial.begin(115200);
  delay(100);  // give the USB-serial a moment

  // set up as Wi-Fi AP (no password)
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_Slave_AP");

  Serial.print("SoftAP MAC: ");
  Serial.println(WiFi.softAPmacAddress());

  // init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error: ESP-NOW init failed");
    while (true) { delay(1000); }
  }

  // register the receive callback
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // nothing needed here; incoming packets trigger onDataRecv()
  delay(1000);
}
