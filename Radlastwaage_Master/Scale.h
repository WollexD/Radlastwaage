#pragma once


class Scale {

public:
  bool getChanged();
  float getNewWeight();

private:
  StatusFlags _status = CalibrationRequired;
  bool _changed = false;
  float _weight = 0;
  unsigned long _lastTimestamp;
  unsigned long _currentTimestamp;
  DeviceIndex _scaleNumber;
  uint8_t _scaleAdress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};