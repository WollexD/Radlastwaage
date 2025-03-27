#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

esp_now_peer_info_t peerInfo;

typedef struct data {
  int waagenNummer;
  int gewicht;
  long timestamp;
} data;

data myMessage;

void messageSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Error");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init success");
  } else {
    Serial.println("ESPNow Init fail");
    return;
  }

  esp_now_register_send_cb(messageSent);

  memcpy(peerInfo.peer_addr, MasterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  myMessage.waagenNummer = 0;
  myMessage.gewicht = 5000;
  myMessage.timestamp = millis();
  
  
  esp_err_t result = esp_now_send(MasterAddress, (uint8_t *)&myMessage, sizeof(myMessage));
  if (result != ESP_OK) {
    Serial.println("Sending error");
  }
  delay(3000);
}