#include <esp_now.h>
#include <WiFi.h>
#include "config.h"  //MAC Adressen
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//Anzeige eines Confi screens wenn eine Waage noch ein anderes Status/Confi Flag sendet
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

typedef struct data {
  DeviceIndex waagenNummer;
  long gewicht;
  int statusFlag;
  long timestamp;
} data;

uint8_t myAddress[6];

data waggenMsg;
data waagenDaten[4] = { LV, LH, RV, RH };

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



  waagenDaten[waggenMsg.waagenNummer].waagenNummer = waggenMsg.waagenNummer;
  waagenDaten[waggenMsg.waagenNummer].gewicht = waggenMsg.gewicht;
  waagenDaten[waggenMsg.waagenNummer].statusFlag = waggenMsg.statusFlag;
  waagenDaten[waggenMsg.waagenNummer].timestamp = waggenMsg.timestamp;
}

void setup() {
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

  esp_now_register_recv_cb(messageReceived);
  lcd.init();  // initialize the lcd
  lcd.backlight();
  lcd.clear();
}

void loop() {
  lcd.clear();
  for (int i = 0; i < 4; i++) {
    Serial.print("Waage Possition :");
    Serial.println(waagenDaten[i].waagenNummer);
    Serial.print("Gewicht in g: ");
    Serial.println(waagenDaten[i].gewicht);
    Serial.print("Zeitstempel: ");
    Serial.println(waagenDaten[i].timestamp);
    Serial.println();
    lcd.setCursor(0, i);
    lcd.print("Gewicht in g: ");
    lcd.print(waagenDaten[i].gewicht);
  }
  Serial.println();
  delay(1000);
}