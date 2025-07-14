// 1. Start - Calibration durchführen und in EEPROM speichern + Startflag setzen
// n > 1. Starts Wage Starten und mit Werten aus EEPROM Initialisieren und auf null (Tara) ausführen / Sollte dabei Taster während Start gedrückt sein Tara überspringen

// Taster kurz => Tara
// Taster > 10sec => Calibration Flag zurücksetzen => beim nächsten Start neue Calibrierung

// Auswahl der Waagennummer nach MAC



//Nachrichten aus Calibration Funktion in Zahlen übersetzen und im Master hinterlegen

#include <esp_now.h>
//#include <esp_system.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <HX711.h>
#include <Preferences.h>
#include <TasterControl.h>
#include "config.h"

#define RW_MODE false
#define RO_MODE true

TasterControl oneButton;

Preferences EEPROMDATA;
esp_now_peer_info_t peerInfo;

typedef struct data {
  int waagenNummer;
  long gewicht;
  int statusFlag;
  long timestamp;
} data;

//StatusFlag Roadmap

//0 alles OK (kalibriert und tara)
//100 Kalibrierung nötig
//110 Kalibrierung Start

DeviceIndex myRole = RH;  // z. B. LV, LH, RV, RH, MASTER


data myMessage;
HX711 myscale;
//  adjust pins if needed.
uint8_t dataPin = 17;
uint8_t clockPin = 16;
uint8_t tasterPin = 19;

float weightfromscale = -9999;

bool compareMACs(uint8_t* mac1, uint8_t* mac2) {
  for (int i = 0; i < 6; i++) {
    if (mac1[i] != mac2[i]) return false;
  }
  return true;
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

void messageSent(const uint8_t* macAddr, esp_now_send_status_t status) {
  // Serial.print("Send status: ");
  // if (status == ESP_NOW_SEND_SUCCESS) {
  //   Serial.println("Success");
  // } else {
  //   Serial.println("Error");
  // }
}

void calibrate() {
  Serial.println("\n\nCALIBRATION\n===========");
  Serial.println("remove all weight from the loadcell");

  myMessage.statusFlag = 110;
  sendeDaten();
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("and press enter\n");
  while (Serial.available() == 0)
    ;

  Serial.println("Determine zero weight offset");
  //  average 20 measurements.
  myscale.tare(20);
  int32_t offset = myscale.get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();


  Serial.println("place a weight on the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n') {
    if (Serial.available()) {
      char ch = Serial.read();
      if (isdigit(ch)) {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  myscale.calibrate_scale(weight, 20);
  float scale = myscale.get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale, 6);

  Serial.print("\nuse myscale.set_offset(");
  Serial.print(offset);
  Serial.print("); and myscale.set_scale(");
  Serial.print(scale, 6);
  Serial.print(");\n");
  Serial.println("in the setup of your project");
  EEPROMDATA.putFloat("offset", offset);  // Wert speichern
  EEPROMDATA.putFloat("scale", scale);    // Wert speichern
  Serial.println("\n\n");
}

//Methode zum Senden der Daten!
void sendeDaten() {  // Sende Daten an den Empfänger
  esp_now_send(MasterAddress, (uint8_t*)&myMessage, sizeof(myMessage));
}

void setup() {
  //Input Pinmode
  oneButton.begin(tasterPin);  // Kein Pullup, da externer Pulldown verwendet wird

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

  Serial.print("Vergleiche mit erwartetem Gerät (");
  switch (myRole) {
    case LV: Serial.print("LV"); break;
    case LH: Serial.print("LH"); break;
    case RV: Serial.print("RV"); break;
    case RH: Serial.print("RH"); break;
    case MASTER: Serial.print("MASTER"); break;
  }
  Serial.println(")");

  Serial.print("Erwartete MAC: ");
  printMAC(deviceAddresses[myRole]);

  if (compareMACs(actualMAC, deviceAddresses[myRole])) {
    Serial.println("✅ MAC passt zur Rolle!");
  } else {
    Serial.println("❌ MAC passt NICHT zur Rolle!");
  }


  esp_now_register_send_cb(messageSent);

  memcpy(peerInfo.peer_addr, MasterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  myscale.begin(dataPin, clockPin);

  EEPROMDATA.begin("savedSettings", RO_MODE);  // Open our namespace (or create it
                                               //  if it doesn't exist) in RO mode.

  bool sclInit = EEPROMDATA.isKey("scaleInit");  // Test for the existence
                                                 // of the "already initialized" key.

  //Initialisieren der Waage beim ersten Start bzw. nach Rücksetzen des Init Flags
  if (sclInit == false) {
    EEPROMDATA.end();                            // close the namespace in RO mode and...
    EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
    myMessage.statusFlag = 100;
    sendeDaten();
    calibrate();
    EEPROMDATA.putBool("scaleInit", true);
  } else {
    float offset = EEPROMDATA.getFloat("offset", 0);  // Wert lesen
    float scale = EEPROMDATA.getFloat("scale", 0);    // Wert lesen
    myscale.set_offset(offset);
    myscale.set_scale(scale);
  }


  // if (!digitalRead(tasterPin)) {  //Wenn Taster während Start gedrückt wird Waage nicht genullt.
  //   myscale.tare();
  //   delay(3000);
  // }
}

void loop() {
  TasterEvent event = oneButton.update();


  switch (event) {
    case KURZER_DRUCK:
      myscale.tare();
      Serial.println("Kurzer Druck");
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
      EEPROMDATA.end();                            // close the namespace in RO mode and...
      EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
      EEPROMDATA.remove("scaleInit");
      break;
    default:
      break;
  }

  if (myscale.is_ready()) {
    weightfromscale = myscale.get_units();
  }
  // Serial.print("UNITS: ");
  // Serial.println(weightfromscale);

  myMessage.waagenNummer = 0;
  myMessage.gewicht = weightfromscale;
  myMessage.timestamp = millis();
  myMessage.statusFlag = 0;
  sendeDaten();
  //delay(3000);
}