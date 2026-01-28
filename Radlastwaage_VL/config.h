#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Optionale Namen für besseren Zugriff
enum DeviceIndex {
  LV = 0,
  LH = 1,
  RV = 2,
  RH = 3,
  MASTER = 4,
  Vorne = 10,
  Hinten = 20,
  ROLE_UNKNOWN = -1
};

constexpr int ROLE_COUNT = 5; // Anzahl der echten Waagen-Geräte


// ---------- MAC-Adressen ----------
extern const uint8_t MasterAddress[6];
extern const uint8_t LVAddress[6];
extern const uint8_t LHAddress[6];
extern const uint8_t RVAddress[6];
extern const uint8_t RHAddress[6];

// Array aus Pointern auf die bestehenden Arrays
extern const uint8_t* const deviceAddresses[ROLE_COUNT];



enum StatusFlags {
  // --- Allgemein ---
  Default = 0,  //alles OK (kalibriert und tara)

  // --- Waagenstatus ---
  ScaleCalibrated = 20,
  ScaleNewStarted = 30,  // statt "ScaleNewStartet"

  // --- Fehlerzustände ---
  Error = 50,             // allgemeiner Fehler
  ErrorConnection = 51,   // Verbindungsfehler
  ErrorCalibration = 52,  // Kalibrierungsfehler
  ErrorSensor = 53,       // optional: Sensorproblem

  // --- Kalibrierung / Arbeitszustände ---
  CalibrationRequired = 100,             //Kalibrierung nötig
  CalibrationWaitRemoveAllWeight = 110,  //Remove all weigt from Scale + Press Button to Continue
  CalibrationWaitAfterZeroing = 111,     //Waage ist genullt + Press Button to Continue
  CalibrationWaitPlaceWeight = 112,      //Kalibrierungsgewicht plazieren + Press Button to Continue
  CalibrationWorkingZeroing = 120,       //Waage wird genullt
  CalibrationWorkingInProgress = 121,    //Waage wird Kalibriert
  CalibrationCompleted = 130,            //Waage ist Kalibriert + Press Button um Vorgang zu beenden!
  CalibrationFailed = 140

};

// ================= STRUCT =================

typedef struct data {
  DeviceIndex waagenNummer;
  long gewicht;
  StatusFlags statusFlag;
  long timestamp;
} data;


#endif