#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "config.h"  //MAC Adressen
#include <Wire.h>
#include <TasterControl.h>
#include <DisplayControl.h>

DisplayControl display;
TasterControl oneButton;

uint8_t tasterPin = 15;
int ansichtCounter = 0;

typedef struct data {
  DeviceIndex waagenNummer;
  long gewicht;
  int statusFlag;
  long timestamp;
} data;

uint8_t myAddress[6];

data waggenMsg;
// data waagenDaten[4] = { LV, LH, RV, RH };

void messageReceived(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
  // Prüfen, ob die MAC-Adresse in der Liste ist
  bool validSender = false;
  int senderIndex = -1;

  for (int i = 0; i < 4; i++) {  // 4 = Anzahl der gespeicherten Adressen (ohne Master)
    if (memcmp(info->src_addr, deviceAddresses[i], 6) == 0) {
      validSender = true;
      senderIndex = i;
      break;
    }
  }

  // Falls Absender nicht gültig, ignorieren
  if (!validSender) {
    Serial.println("⚠ Unbekannte MAC-Adresse! Nachricht ignoriert.");
    return;
  }

  memcpy(&waggenMsg, incomingData, sizeof(waggenMsg));

  // Weiche Sicherung, falls waagenNummer außerhalb des Arrays liegt
  if (waggenMsg.waagenNummer < 0 || waggenMsg.waagenNummer >= 5) {
    Serial.println("⚠ Ungültige Waagen-Nummer!");
    return;
  }

  // Serial.print("Vergleiche mit erwartetem Gerät (");
  // switch (myRole) {
  //   case LV: Serial.print("LV"); break;
  //   case LH: Serial.print("LH"); break;
  //   case RV: Serial.print("RV"); break;
  //   case RH: Serial.print("RH"); break;
  //   case MASTER: Serial.print("MASTER"); break;
  // }
  // Serial.println(")");



  // waagenDaten[waggenMsg.waagenNummer].waagenNummer = waggenMsg.waagenNummer;
  // waagenDaten[waggenMsg.waagenNummer].gewicht = waggenMsg.gewicht;
  // waagenDaten[waggenMsg.waagenNummer].statusFlag = waggenMsg.statusFlag;
  // waagenDaten[waggenMsg.waagenNummer].timestamp = waggenMsg.timestamp;

  display.updateWeight(waggenMsg.gewicht, waggenMsg.waagenNummer);
}

void printMAC(uint8_t* mac) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("}");
}

void setup() {
  oneButton.begin(tasterPin);
  display.begin();
  display.updateScreen();
  memcpy(myAddress, MasterAddress, sizeof(myAddress));
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init success");
  } else {
    Serial.println("ESPNow Init fail");
    return;
  }
  uint8_t actualMAC[6];
  esp_wifi_get_mac(WIFI_IF_STA, actualMAC);
  Serial.print("Aktuelle MAC: ");
  printMAC(actualMAC);

  esp_now_register_recv_cb(messageReceived);

  display.Standardansicht();
  // waagenDaten[0].gewicht = 5000;
  display.updateWeight(5000, 0);
}

long lastChange = 0;
long lastChange2 = 0;
long gew = 0;
long gew2 = 0;

void loop() {
  //-------Auswertung Taster-----------
  TasterEvent event = oneButton.update();
  switch (event) {
    case KURZER_DRUCK:
      Serial.println("Kurzer Druck");
      ansichtCounter++;
      ansichtCounter = (ansichtCounter == 2) ? 0 : ansichtCounter;
      display.changeAnsicht(ansichtCounter);
      break;
    case DOPPELKLICK:
      Serial.println("Doppelklick");
      break;
    case LANGER_DRUCK:
      Serial.println("Langer Druck");
      break;
    case SEHR_LANGER_DRUCK:
      Serial.println("Sehr langer Druck");
      break;
    case EXTRA_LANGER_DRUCK:
      Serial.println("Extra langer Druck");
      break;
    default:
      break;
  }

  display.updateScreen();


  if (millis() - lastChange > 2000) {
    lastChange = millis();
    gew = gew + 1000;
    display.updateWeight(gew, LV);
  }

  if (millis() - lastChange2 > 1000) {
    lastChange2 = millis();
    gew2 = gew2 + 1000;
    display.updateWeight(gew2, RH);
  }
}