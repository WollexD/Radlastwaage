// 1. Start - Calibration durchführen und in EEPROM speichern + Startflag setzen
// n > 1. Starts Wage Starten und mit Werten aus EEPROM Initialisieren und auf null (Tara) ausführen / Sollte dabei Taster während Start gedrückt sein Tara überspringen

// Taster kurz => Tara
// Taster > 10sec => Calibration Flag zurücksetzen => beim nächsten Start neue Calibrierung

// Auswahl der Waagennummer nach MAC



//Nachrichten aus Calibration Funktion in Zahlen übersetzen und im Master hinterlegen

#include <esp_now.h>
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
  DeviceIndex waagenNummer;
  long gewicht;  //in Gramm
  StatusFlags statusFlag;
  long timestamp;
} data;

const char* deviceIndexToString(DeviceIndex index) {
  switch (index) {
    case LV: return "LV";
    case LH: return "LH";
    case RV: return "RV";
    case RH: return "RH";
    case MASTER: return "MASTER";
    default: return "UNKNOWN";
  }
}

DeviceIndex getDeviceRole(uint8_t* actualMAC) {
  for (int i = 0; i < sizeof(deviceAddresses) / sizeof(deviceAddresses[0]); i++) {
    bool match = true;
    for (int j = 0; j < 6; j++) {
      if (actualMAC[j] != deviceAddresses[i][j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return static_cast<DeviceIndex>(i);
    }
  }

  // Wenn keine Übereinstimmung gefunden wurde
  return static_cast<DeviceIndex>(-1);  // oder z. B. ein spezielles UNKNOWN
}




DeviceIndex myRole;  // LV, LH, RV, RH, MASTER
data myMessage;
HX711 myscale;
//  adjust pins if needed.
uint8_t dataPin = 17;
uint8_t clockPin = 16;
uint8_t tasterPin = 19;

const int MEDIAN_SIZE = 9;
float medianBuffer[MEDIAN_SIZE] = { 0 };
int medianIndex = 0;
int valuesCollected = 0;
float aktuellesGewicht = 0;

long currentTime = millis();
long lastTransmitTime = currentTime;

int currentStatus = 0;

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

void messageSent(const wifi_tx_info_t* macAddr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Error");
  }
}

void calibrate() {
  Serial.println("\n\n\n=======CALIBRATION Start ==========\n");

  myMessage.statusFlag = CalibrationWaitRemoveAllWeight;  //Remove all weigt from Scale + Press Button to Continue
  sendeDaten();
  while (!digitalRead(tasterPin)) yield();

  myMessage.statusFlag = CalibrationWorkingZeroing;  //Waage wird genullt
  sendeDaten();
  delay(1000);
  //  average 20 measurements.
  myscale.tare(20);
  int32_t offset = myscale.get_offset();
  Serial.print("OFFSET: ");
  Serial.println(offset);
  delay(1000);

  myMessage.statusFlag = CalibrationWaitAfterZeroing;  //Waage ist genullt + Press Button to Continue
  sendeDaten();
  while (!digitalRead(tasterPin)) yield();

  myMessage.statusFlag = CalibrationWaitPlaceWeight;  //Kalibrierungsgewicht plazieren + Press Button to Continue
  sendeDaten();
  delay(2000);
  while (!digitalRead(tasterPin)) yield();

  myMessage.statusFlag = CalibrationWorkingInProgress;  //Waage wird Kalibriert
  sendeDaten();
  delay(1000);

  float weight = 640;  //<--- Hier Kalibrierungsgewicht anpassen
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  myscale.calibrate_scale(weight, 20);
  float scale = myscale.get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale, 6);


  EEPROMDATA.putFloat("offset", offset);  // Wert speichern
  EEPROMDATA.putFloat("scale", scale);    // Wert speichern

  myMessage.statusFlag = CalibrationCompleted;  //Waage ist Kalibriert + Press Button um Vorgang zu beenden!
  sendeDaten();
  while (!digitalRead(tasterPin)) yield();

  Serial.println("\n=======CALIBRATION ENDE ===========");
  currentStatus = 0;
}

//Methode zum Senden der Daten!
void sendeDaten() {  // Sende Daten an den Empfänger
  esp_now_send(MasterAddress, (uint8_t*)&myMessage, sizeof(myMessage));
}

void setup() {
  //Input Pinmode
  oneButton.begin(tasterPin);

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

  myRole = getDeviceRole(actualMAC);
  Serial.print("Erkannte Rolle: ");
  Serial.println(deviceIndexToString(myRole));

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
                                                 //------Default Nachricht bauen-------
  myMessage.waagenNummer = myRole;
  myMessage.gewicht = 0;
  myMessage.timestamp = millis();
  myMessage.statusFlag = currentStatus;

  //Initialisieren der Waage beim ersten Start bzw. nach Rücksetzen des Init Flags
  if (sclInit == false) {
    EEPROMDATA.end();                            // close the namespace in RO mode and...
    EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
    currentStatus = 100;
    myMessage.statusFlag = currentStatus;
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
  //-------Auswertung Taster-----------
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
      currentStatus = 100;
      EEPROMDATA.end();                            // close the namespace in RO mode and...
      EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
      EEPROMDATA.remove("scaleInit");
      break;
    default:
      currentStatus = Default;
      break;
  }

  //-------Gewicht Auslesen------------
  //-----+Median über Werte bilden---
  if (myscale.is_ready()) {
    float gewicht = myscale.get_units();
    if (isfinite(gewicht)) {
      // Neuer Wert in Ringpuffer
      medianBuffer[medianIndex] = gewicht;
      medianIndex = (medianIndex + 1) % MEDIAN_SIZE;

      // Mitzählen wie viele Werte gesammelt wurden (max. MEDIAN_SIZE)
      if (valuesCollected < MEDIAN_SIZE) {
        valuesCollected++;
      }

      // Kopie der gültigen Werte zum Sortieren
      float sorted[MEDIAN_SIZE];
      memcpy(sorted, medianBuffer, sizeof(sorted));

      // Sortieren (nur so viele wie gesammelt wurden)
      for (int i = 0; i < valuesCollected - 1; i++) {
        for (int j = i + 1; j < valuesCollected; j++) {
          if (sorted[j] < sorted[i]) {
            float temp = sorted[i];
            sorted[i] = sorted[j];
            sorted[j] = temp;
          }
        }
      }

      // Median bestimmen
      if (valuesCollected % 2 == 1) {
        aktuellesGewicht = sorted[valuesCollected / 2];
      } else {
        aktuellesGewicht = (sorted[valuesCollected / 2 - 1] + sorted[valuesCollected / 2]) / 2.0;
      }
    }
  }

  //-------Nachricht "zusammenbauen"---
  myMessage.waagenNummer = myRole;
  myMessage.gewicht = aktuellesGewicht;
  myMessage.timestamp = millis();
  myMessage.statusFlag = currentStatus;

  //-------Nachricht Senden------------
  currentTime = millis();
  if (currentTime > lastTransmitTime + 100) {
    sendeDaten();
    lastTransmitTime = currentTime;
  }
}