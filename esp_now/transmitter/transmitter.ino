#include <WiFi.h>
#include <esp_now.h>

// Structure to send data
typedef struct struct_message {
  int number;
} struct_message;

struct_message outgoingData;

// ← Replace with your receiver’s MAC (six 0x-prefixed bytes)
uint8_t broadcastAddress[6] = {0x78, 0x1C, 0x3C, 0xB8, 0x26, 0xB1};

// Callback when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet to: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(onDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  static int counter = 0;
  outgoingData.number = counter++;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&outgoingData, sizeof(outgoingData));
  if (result != ESP_OK) {
    Serial.println("Error sending the data");
  }

  delay(1000);  // send every second
}
