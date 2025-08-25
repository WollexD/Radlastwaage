#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "config.h"  //MAC Adressen
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TasterControl.h>
//Anzeige eines Confi screens wenn eine Waage noch ein anderes Status/Confi Flag sendet
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
TasterControl oneButton;

uint8_t tasterPin = 15;
int ansichtCounter = 0;
int backGroundCounterStandard = 60;
int backGroundCounter = 0;

typedef struct data {
  DeviceIndex waagenNummer;
  long gewicht;
  int statusFlag;
  long timestamp;
} data;

uint8_t myAddress[6];

byte heart[8] = {
  B00000,
  B00000,
  B01010,
  B10101,
  B10001,
  B01010,
  B00100,
  B00000
};

byte leftUP[8] = {
  B00000,
  B00000,
  B00111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte leftdown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00111,
  B00000,
  B00000
};

byte rightUP[8] = {
  B00000,
  B00000,
  B11100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte rightdown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11100,
  B00000,
  B00000
};

byte upper[8] = {
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte lower[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000
};


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

void calibrierungsText(int status) {
  lcd.clear();

  //"--------------------"
  //"
  switch (status) {
    case 100:
      lcd.setCursor(1, 1);  //Spalte , Zeile
      lcd.print("Calibration REQ.");
      break;
    case 110:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Remove all weigt");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("from Scale + Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 111:
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Waage wird genullt");
      break;
    case 112:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage ist genullt");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 113:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Kalibrierungsgewicht");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("plazieren + Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 114:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage  ");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("wird");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Kalibriert");
      break;
    case 115:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage ist Kalibriert");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Press Button to");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("End Calibration!");
      break;
    default:
      break;
  }
}

void HintergrundWaage() {
  lcd.setCursor(0, 0);
  lcd.print("VL");
  lcd.setCursor(7, 0);
  lcd.print("kg|Gesamt:");
  lcd.setCursor(0, 1);
  lcd.print("VR");
  lcd.setCursor(7, 1);
  lcd.print("kg|");
  lcd.setCursor(18, 1);
  lcd.print("kg");
  lcd.setCursor(0, 2);
  lcd.print("HL");
  lcd.setCursor(7, 2);
  lcd.print("kg|VtlVA");
  lcd.setCursor(19, 2);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("HR");
  lcd.setCursor(7, 3);
  lcd.print("kg|VtlHA");
  lcd.setCursor(19, 3);
  lcd.print("%");
}

void ProzentAnzeigen() {
  float gewichtVorne = waagenDaten[0].gewicht + waagenDaten[1].gewicht;
  float gewichtHinten = waagenDaten[2].gewicht + waagenDaten[3].gewicht;
  float gewichtGesamt = gewichtVorne + gewichtHinten;
  float VtlVA = (gewichtVorne / gewichtGesamt) * 100;
  float VtlHA = (gewichtHinten / gewichtGesamt) * 100;

  lcd.setCursor(15, 2);
  lcd.print("    ");
  lcd.setCursor(15, 2);
  char ausgabe4A[5];
  formatVtl4(VtlVA, ausgabe4A);
  lcd.print(ausgabe4A);

  lcd.setCursor(15, 3);
  lcd.print("    ");
  lcd.setCursor(15, 3);
  char ausgabe4B[5];
  formatVtl4(VtlHA, ausgabe4B);
  lcd.print(ausgabe4B);
}

void Standardansicht() {
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(2, i);
    lcd.print("     ");
    lcd.setCursor(2, i);
    char ausgabe5[6];
    formatWeight5(waagenDaten[i].gewicht, ausgabe5);
    lcd.print(ausgabe5);
  }
  lcd.setCursor(12, 1);
  lcd.print("      ");
  lcd.setCursor(12, 1);
  char ausgabe6[7];
  formatWeight6(waagenDaten[0].gewicht + waagenDaten[1].gewicht + waagenDaten[2].gewicht + waagenDaten[3].gewicht, ausgabe6);
  lcd.print(ausgabe6);
  ProzentAnzeigen();

  backGroundCounter--;
  if (backGroundCounter < 0) {
    HintergrundWaage();
    backGroundCounter = backGroundCounterStandard;
  }
}

void AutoHintergrund() {
  lcd.setCursor(8,0);
  lcd.write(byte(2));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(4));
  lcd.setCursor(8, 1);
  lcd.print("|  |");
  lcd.setCursor(8, 2);
  lcd.print("|  |");
  lcd.setCursor(8, 3);
  lcd.write(byte(3));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(5));
}

// Funktion: Float -> "000,0"-String
void formatVtl4(float Vtl, char* buffer) {
  // Überlaufbehandlung
  if (Vtl < 0) {
    strcpy(buffer, "XX,X");
    return;
  }
  if (Vtl > 99.9) {
    strcpy(buffer, "XX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int Vtl_int = (int)(Vtl * 10.0 + (Vtl >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(Vtl_int / 10);
  int hinten = abs(Vtl_int % 10);

  sprintf(buffer, "%2d,%1d", vorn, hinten);  // z. B. " 12,3"
}
void formatWeight5(float weight, char* buffer) {
  weight = weight / 1000;  //Umrechnung von Gramm in kg
  // Überlaufbehandlung
  if (weight < -99.9) {
    strcpy(buffer, "-XX,X");
    return;
  }
  if (weight > 999.9) {
    strcpy(buffer, "XXX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int gewicht_int = (int)(weight * 10.0 + (weight >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(gewicht_int / 10);
  int hinten = abs(gewicht_int % 10);

  // Formatieren abhängig von Vorzeichen
  if (weight < 0) {
    // Platz für '-' berücksichtigen (max. -99,9)
    sprintf(buffer, "-%2d,%1d", vorn, hinten);  // z. B. -12,3 → "-12,3"
  } else {
    // Rechtsbündig mit Leerzeichen für positive Werte
    sprintf(buffer, "%3d,%1d", vorn, hinten);  // z. B. " 12,3"
  }
}
void formatWeight6(float weight, char* buffer) {
  weight = weight / 1000;  //Umrechnung von Gramm in kg
  // Überlaufbehandlung
  if (weight < -999.9) {
    strcpy(buffer, "-XXX,X");
    return;
  }
  if (weight > 9999.9) {
    strcpy(buffer, "XXXX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int gewicht_int = (int)(weight * 10.0 + (weight >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(gewicht_int / 10);
  int hinten = abs(gewicht_int % 10);

  // Formatieren abhängig von Vorzeichen
  if (weight < 0) {
    // Platz für '-' berücksichtigen (max. -99,9)
    sprintf(buffer, "-%3d,%1d", vorn, hinten);  // z. B. -12,3 → "-12,3"
  } else {
    // Rechtsbündig mit Leerzeichen für positive Werte
    sprintf(buffer, "%4d,%1d", vorn, hinten);  // z. B. " 12,3"
  }
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

  lcd.init();  // initialize the lcd
  lcd.createChar(0, heart);
  lcd.createChar(1, upper);
  lcd.createChar(2, leftUP);
  lcd.createChar(3, leftdown);
  lcd.createChar(4, rightUP);
  lcd.createChar(5, rightdown);
  lcd.createChar(6, lower);

  lcd.backlight();
  lcd.clear();
}

void loop() {
  //-------Auswertung Taster-----------
  TasterEvent event = oneButton.update();
  switch (event) {
    case KURZER_DRUCK:
      Serial.println("Kurzer Druck");
      ansichtCounter++;
      ansichtCounter = (ansichtCounter == 2) ? 0 : ansichtCounter;
      break;
    case DOPPELKLICK:
      Serial.println("Doppelklick");
      break;
    case LANGER_DRUCK:
      Serial.println("Langer Druck");
      ansichtCounter++;
      ansichtCounter = (ansichtCounter == 2) ? 0 : ansichtCounter;
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

  //Bearbeitung
  waagenDaten[0].gewicht = 5000;
  if (waagenDaten[3].statusFlag > 0) {
    calibrierungsText(waagenDaten[3].statusFlag);
    lcd.clear();
  } else if (ansichtCounter == 0) {
    Standardansicht();
  } else if (ansichtCounter == 1) {
    lcd.clear();
    AutoHintergrund();
  }
}