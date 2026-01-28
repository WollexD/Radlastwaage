#ifndef CALIBRATION_CONTROLLER_H
#define CALIBRATION_CONTROLLER_H

#include <Arduino.h>
#include <HX711.h>
#include <Preferences.h>
#include "config.h"   // für StatusFlags & data

class CalibrationController {
public:
  CalibrationController(HX711& scale,
                         Preferences& prefs,
                         uint8_t tasterPin,
                         data& message);

  void start();          // Kalibrierung starten
  void update();         // non-blocking Ablauf
  bool isActive() const; // läuft gerade eine Kalibrierung?

private:
  HX711& myscale;
  Preferences& EEPROMDATA;
  uint8_t tasterPin;
  data& myMessage;

  bool active = false;
  unsigned long stepTimestamp = 0;

  float offset = 0.0f;
  float scale = 0.0f;
};

#endif
