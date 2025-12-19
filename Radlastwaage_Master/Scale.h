#pragma once
#include "Event.h"
#include "config.h"

// 🔧 Forward Declaration
class Scale;

class ScaleListener {
public:
  virtual void onWeightChanged(Scale* caller) = 0;
};

class Scale : public Event<ScaleListener> {
public:
  Scale(DeviceIndex n) : _scaleNumber(n) {};  // Konstruktor muss noch überarbeitet werden!
  bool getChanged() const;
  float getWeight() const;
  DeviceIndex getIndex() const;
  bool updateScale(float newWeight, StatusFlags newStatus, unsigned long newTimeStamp);

  // bool isTimetoLong...
  DeviceIndex _scaleNumber;

private:
  StatusFlags _status = CalibrationRequired;
  bool _changed = false;
  bool _changedWeigth = false;
  bool _changedStatus = false;
  bool _changedTime   = false;
  float _weight = 0;
  unsigned long _lastTimestamp;
  unsigned long _currentTimestamp;
  uint8_t _scaleAdress[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  void notifyListeners();
};