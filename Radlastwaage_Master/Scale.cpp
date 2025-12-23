#include "Scale.h"


Scale::Scale(DeviceIndex n)
  : _scaleNumber(n) {
};

bool Scale::getChanged() const {
  return this->_changed;
};

float Scale::getWeight() const {
  return this->_weight;
}

StatusFlags Scale::getStatus() const {
  return this->_status;
}

DeviceIndex Scale::getIndex() const {
  return this->_scaleNumber;
}

bool Scale::scaleAlive() {
  if (millis() - this->_lastTimestampOnMaster >= 1000) {
    this->_status = ErrorConnection;
    this->_changed = true;
    this->_changedStatus = true;
    this->_weight = 0.0f;
    deaktivatOnListeners();
    return false;
  }
  return true;
}

bool Scale::updateScale(float newWeight, StatusFlags newStatus, unsigned long newTimeStamp) {
  if (this->_status != newStatus) {
    this->_status = newStatus;
    this->_changed = true;
    this->_changedStatus = true;
  }
  if (this->_currentTimestamp != newTimeStamp) {
    this->_lastTimestamp = this->_currentTimestamp;
    this->_lastTimestampOnMaster = millis();
    this->_currentTimestamp = newTimeStamp;
    this->_changed = true;
    this->_changedTime = true;
  }

  if (this->_weight != newWeight) {
    if (std::fabs(newWeight - this->_weight) > 10.0f) {
      this->_weight = newWeight;
      this->_changed = true;
      this->_changedWeigth = true;
    }
  }

  if (this->_changed) {
    notifyListeners();
    return true;
  }
  return false;
}

// void Scale::registerAt(DisplayControl& display) {
//     display.addToAllScalesList(this);
// }

void Scale::deaktivatOnListeners() {
  for (int i = 0; i < listenerCount; i++) {
    listeners[i]->deactivateScale(this);
  }
}

void Scale::addMeToAllScallesList() {
  for (int i = 0; i < listenerCount; i++) {
    listeners[i]->addToAllScalesList(this);
  }
}

void Scale::notifyListeners() {
  for (int i = 0; i < listenerCount; i++) {
    listeners[i]->onWeightChanged(this);
  }
}