#include "CalibrationController.h"

CalibrationController::CalibrationController(HX711& scale,
                                             Preferences& prefs,
                                             uint8_t taster,
                                             data& message)
  : myscale(scale),
    EEPROMDATA(prefs),
    tasterPin(taster),
    myMessage(message) {}

void CalibrationController::start() {
  _active = true;
  _stepTimestamp = millis();
  myMessage.gewicht = 0;
  myMessage.statusFlag = CalibrationWaitRemoveAllWeight;

  Serial.println("\n======= CALIBRATION START =======");
}

bool CalibrationController::isActive() const {
  return _active;
}

void CalibrationController::update() {
  if (!_active) return;

  unsigned long now = millis();

  switch (myMessage.statusFlag) {

    case CalibrationWaitRemoveAllWeight:
      if (digitalRead(tasterPin)) {
        myMessage.statusFlag = CalibrationWorkingZeroing;
        _stepTimestamp = now;
      }

      break;

    case CalibrationWorkingZeroing:
      if (now - _stepTimestamp >= 1000) {
        myscale.set_offset(0);  // Alte Offsets löschen
        myscale.tare(20);
        _offset = myscale.get_offset();

        myMessage.statusFlag = CalibrationWaitAfterZeroing;
      }
      break;

    case CalibrationWaitAfterZeroing:
      if (digitalRead(tasterPin) && now - _stepTimestamp >= 1000) {
        myMessage.statusFlag = CalibrationWaitPlaceWeight;
        _stepTimestamp = now;
      }
      break;

    case CalibrationWaitPlaceWeight:
      if (digitalRead(tasterPin) && now - _stepTimestamp >= 1000) {
        myMessage.statusFlag = CalibrationWorkingInProgress;
        _stepTimestamp = now;
      }
      break;

    case CalibrationWorkingInProgress:
      if (now - _stepTimestamp >= 1000) {

        float _weight = 640;
        myscale.calibrate_scale(_weight, 20);
        _scale = myscale.get_scale();

        EEPROMDATA.putFloat("offset", _offset);
        EEPROMDATA.putFloat("scale", _scale);
        EEPROMDATA.putBool("scaleInit", true);

        myMessage.statusFlag = CalibrationCompleted;
      }
      break;

    case CalibrationCompleted:
      if (digitalRead(tasterPin)) {
        myMessage.statusFlag = Default;
        _active = false;

        Serial.print("Offset: ");
        Serial.println(_offset);
        Serial.print("Scale: ");
        Serial.println(_scale, 6);

        Serial.println("======= CALIBRATION ENDE =======");
      }
      break;

    default:
      break;
  }
}
