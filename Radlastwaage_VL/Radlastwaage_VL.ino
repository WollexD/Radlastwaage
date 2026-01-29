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
#include "CalibrationController.h"


#define RW_MODE false
#define RO_MODE true

TasterControl oneButton;

Preferences EEPROMDATA;
esp_now_peer_info_t peerInfo;

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

// DeviceIndex aus MAC ermitteln
DeviceIndex getDeviceRole(const uint8_t* actualMAC) {
  for (int i = 0; i < ROLE_COUNT; i++) {
    if (compareMACs(actualMAC, deviceAddresses[i])) return static_cast<DeviceIndex>(i);
  }
  return ROLE_UNKNOWN;
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

StatusFlags currentStatus = Default;


CalibrationController calibration(myscale, EEPROMDATA, tasterPin, myMessage);


bool compareMACs(const uint8_t* mac1, const uint8_t* mac2) {
  for (int i = 0; i < 6; i++) {
    if (mac1[i] != mac2[i]) return false;
  }
  return true;
}

void printMAC(const uint8_t* mac) {
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

//Methode zum Senden der Daten!
void sendeDaten() {  // Sende Daten an den Empfänger
  esp_now_send(MasterAddress, (uint8_t*)&myMessage, sizeof(myMessage));
}

void setup() {
  Serial.begin(115200);
  Serial.println("------ Startup Booting ------");

  //--------- Tasterauswertung ---------
  oneButton.begin(tasterPin);

  //-------------- WLAN ----------------
  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init success");
  } else {
    Serial.println("ESPNow Init fail");
    return;
  }

  //--------- Auslesen MAC --------------
  uint8_t actualMAC[6];
  esp_wifi_get_mac(WIFI_IF_STA, actualMAC);
  Serial.print("Aktuelle MAC: ");
  printMAC(actualMAC);

  //--------- Auslesen Rolle ------------
  myRole = getDeviceRole(actualMAC);
  Serial.print("Erkannte Rolle: ");
  Serial.println(deviceIndexToString(myRole));

  Serial.print("Erwartete MAC: ");
  if (myRole >= 0 && myRole < ROLE_COUNT && deviceAddresses[myRole] != nullptr) {
    printMAC(deviceAddresses[myRole]);
  } else {
    Serial.println("—");
  }

  //--------- Doppelcheck Rolle MAC -----
  if (compareMACs(actualMAC, deviceAddresses[myRole])) {
    Serial.println("✅ MAC passt zur Rolle!");
  } else {
    Serial.println("❌ MAC passt NICHT zur Rolle!");
  }

  //------ ESPNow Verbindung etc.-------
  esp_now_register_send_cb(messageSent);
  memcpy(peerInfo.peer_addr, MasterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  //------Default Nachricht bauen-------
  myMessage.waagenNummer = myRole;
  myMessage.gewicht = 0;
  myMessage.timestamp = millis();
  myMessage.statusFlag = Default;

  //---------- Waage Verbinden ---------
  myscale.begin(dataPin, clockPin);

  //---- EEPROM Open + Verarbeitung ----
  EEPROMDATA.begin("savedSettings", RO_MODE);  // Open our namespace (or create it
                                               //  if it doesn't exist) in RO mode.

  bool sclInit = EEPROMDATA.isKey("scaleInit");  // Test for the existence
                                                 // of the "already initialized" key.

  //Initialisieren der Waage beim ersten Start bzw. nach Rücksetzen des Init Flags
  if (sclInit == false) {
    EEPROMDATA.end();                            // close the namespace in RO mode and...
    EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
    myMessage.statusFlag = CalibrationRequired;
    calibration.start();
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
  Serial.println("----- Startup completed -----");
}

void loop() {

  calibration.update();

  if (!calibration.isActive()) {
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
        EEPROMDATA.end();                            // close the namespace in RO mode and...
        EEPROMDATA.begin("savedSettings", RW_MODE);  //  reopen it in RW mode.
        EEPROMDATA.remove("scaleInit");
        calibration.start();
        break;
      default:
        // myMessage.statusFlag = Default;
        break;
    }
  }


  //-------Gewicht Auslesen------------
  //-----+Median über Werte bilden---
  if (myscale.is_ready() && !calibration.isActive()) {
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
  // myMessage.waagenNummer = myRole; //Ändert sich ja nicht mehr
  myMessage.gewicht = aktuellesGewicht;
  myMessage.timestamp = millis();
  if (!calibration.isActive()) {
    myMessage.statusFlag = Default;
  }


  //-------Nachricht Senden------------
  currentTime = millis();
  if (currentTime > lastTransmitTime + 100) {
    sendeDaten();
    lastTransmitTime = currentTime;
  }
}