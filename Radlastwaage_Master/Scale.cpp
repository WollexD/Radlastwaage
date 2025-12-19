#include "Scale.h"

bool Scale::getChanged() const{
  return this->_changed;
};

float Scale::getWeight() const {
  return this->_weight;
}

DeviceIndex Scale::getIndex() const{
  return this->_scaleNumber;
}

bool Scale::updateScale(float newWeight, StatusFlags newStatus, unsigned long newTimeStamp) {
  if (this->_status != newStatus) {
    this->_status = newStatus;
    this->_changed = true;
    this->_changedStatus = true;
  }
  if (this->_currentTimestamp != newTimeStamp) {
    this->_lastTimestamp = this->_currentTimestamp;
    this->_currentTimestamp = newTimeStamp;
    this->_changed = true;
    this->_changedTime = true;
  }

  if (this->_weight != newWeight) {
    if (std::fabs(newWeight - _weight) > 10.0f) {
      _weight = newWeight;
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

void Scale::notifyListeners() {
  for (int i = 0; i < listenerCount; i++) {
    listeners[i]->onWeightChanged(this);
  }
}