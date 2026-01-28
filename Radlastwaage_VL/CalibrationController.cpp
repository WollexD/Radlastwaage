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
  active = true;
  stepTimestamp = millis();
  myMessage.statusFlag = CalibrationWaitRemoveAllWeight;

  Serial.println("\n======= CALIBRATION START =======");
}

bool CalibrationController::isActive() const {
  return active;
}

void CalibrationController::update() {
  if (!active) return;

  unsigned long now = millis();

  switch (myMessage.statusFlag) {

    case CalibrationWaitRemoveAllWeight:
      Serial.println("CalibrationWaitRemoveAllWeight");
      if (digitalRead(tasterPin)) {
        myMessage.statusFlag = CalibrationWorkingZeroing;
        stepTimestamp = now;
      }

      break;

    case CalibrationWorkingZeroing:
      Serial.println("CalibrationWorkingZeroing");
      if (now - stepTimestamp >= 1000) {
        myscale.tare(20);
        offset = myscale.get_offset();

        Serial.print("OFFSET: ");
        Serial.println(offset);

        myMessage.statusFlag = CalibrationWaitAfterZeroing;
      }
      break;

    case CalibrationWaitAfterZeroing:
    Serial.println("CalibrationWaitAfterZeroing");
      if (digitalRead(tasterPin) && now - stepTimestamp >= 1000) {
        myMessage.statusFlag = CalibrationWaitPlaceWeight;
        stepTimestamp = now;
      }
      break;

    case CalibrationWaitPlaceWeight:
    Serial.println("CalibrationWaitPlaceWeight");
      if (digitalRead(tasterPin) && now - stepTimestamp >= 1000) {
        myMessage.statusFlag = CalibrationWorkingInProgress;
        stepTimestamp = now;
      }
      break;

    case CalibrationWorkingInProgress:
    Serial.println("CalibrationWorkingInProgress");
      if (now - stepTimestamp >= 1000) {

        float weight = 640;
        myscale.calibrate_scale(weight, 20);
        scale = myscale.get_scale();

        EEPROMDATA.putFloat("offset", offset);
        EEPROMDATA.putFloat("scale", scale);
        EEPROMDATA.putBool("scaleInit", true);

        myMessage.statusFlag = CalibrationCompleted;
      }
      break;

    case CalibrationCompleted:
    Serial.println("CalibrationCompleted");
      if (digitalRead(tasterPin)) {
        myMessage.statusFlag = Default;
        active = false;
        Serial.print("Offset: ");
        Serial.print(offset);
        Serial.print("Scale: ");
        Serial.print(scale, 6);

        Serial.println("======= CALIBRATION ENDE =======");
      }
      break;

    default:
      break;
  }
}
