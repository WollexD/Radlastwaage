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

  bool _active = false;
  unsigned long _stepTimestamp = 0;
  int32_t _offset = 0;
  float _scale = 0;
};

#endif
