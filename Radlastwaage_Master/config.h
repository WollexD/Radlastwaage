// #pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

extern uint8_t MasterAddress[];
extern uint8_t LVAddress[];
extern uint8_t LHAddress[];
extern uint8_t RVAddress[];
extern uint8_t RHAddress[];

// Array aus Pointern auf die bestehenden Arrays
extern uint8_t* deviceAddresses[];

// Optionale Namen für besseren Zugriff
enum DeviceIndex {
  LV = 0,
  LH = 1,
  RV = 2,
  RH = 3,
  MASTER = 4,
  Vorne = 10,
  Hinten = 20
};

// hier inline definieren → funktioniert überall, wo der Header eingebunden wird
inline DeviceIndex& operator++(DeviceIndex& d) {
  d = static_cast<DeviceIndex>(static_cast<int>(d) + 1);
  return d;
}

inline bool operator<(DeviceIndex a, DeviceIndex b) {
  return static_cast<int>(a) < static_cast<int>(b);
}

enum StatusFlags {
  // --- Allgemein ---
  Default = 0,

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

#endif