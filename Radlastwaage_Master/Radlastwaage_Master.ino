#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "config.h"  //MAC Adressen
#include <Wire.h>
#include <TasterControl.h>
#include <Scale.h>
#include <DisplayControl.h>

typedef struct data {
  DeviceIndex waagenNummer;
  long gewicht;
  StatusFlags statusFlag;
  long timestamp;
} data;
data waagenMsg;

DisplayControl display;

TasterControl oneButton;
uint8_t tasterPin = 15;

Scale* waagen[4];

uint8_t myAddress[6];

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

  memcpy(&waagenMsg, incomingData, sizeof(waagenMsg));

  // Weiche Sicherung, falls waagenNummer außerhalb des Arrays liegt
  if (waagenMsg.waagenNummer < 0 || waagenMsg.waagenNummer >= 5) {
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

  waagen[waagenMsg.waagenNummer]->updateScale(waagenMsg.gewicht, waagenMsg.statusFlag, waagenMsg.timestamp);
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
  memcpy(myAddress, MasterAddress, sizeof(myAddress));
  Serial.begin(115200);
  delay(1000);

  for (DeviceIndex i = DeviceIndex::LV; i < DeviceIndex::MASTER; ++i) {
    waagen[static_cast<int>(i)] = new Scale(i);
    waagen[static_cast<int>(i)]->addListener(&display);
    waagen[static_cast<int>(i)]->addMeToAllScallesList();
  }



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


  display.clear();
  display.setStandardansicht();


  waagen[LV]->updateScale(5000.0, Default, 0);
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
      display.nextAnsicht();
      Serial.println("Kurzer Druck");
      break;
    case DOPPELKLICK:
      Serial.println("Doppelklick");
      break;
    case LANGER_DRUCK:
      Serial.println("Langer Druck");
      waagen[LH]->updateScale(0.0f, ErrorCalibration, millis());
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

  if (!oneButton.isBlocking()) {
    display.updateScreen();
  }

  for (Scale* w : waagen){
    bool alive = w->scaleAlive();
  }

  // delay(500);
  if (millis() - lastChange > 200) {
    lastChange = millis();
    gew = gew + 1000;
    waagen[LV]->updateScale(gew, Default, millis());
  }

  if (millis() - lastChange2 > 100) {
    lastChange2 = millis();
    gew2 = gew2 + 1000;
    waagen[RH]->updateScale(gew2, Default, millis());
  }
}