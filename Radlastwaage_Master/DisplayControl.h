// #pragma once
#ifndef DISPLAYCONTROL_H
#define DISPLAYCONTROL_H

#include <map>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Scale.h"

// enum DeviceIndex {
//     LV     = 0,
//     LH     = 1,
//     RV     = 2,
//     RH     = 3,
//     MASTER = 4
// };

enum FormatMode {
  FORMAT_WEIGHT,
  FORMAT_PERCENT,
  FORMAT_STRING,
};

class DisplayControl : public ScaleListener {
public:
  DisplayControl();
  void begin();
  
  void clear();
  void changeAnsicht(int newAnsicht);
  void setStandardansicht();
  void nextAnsicht();

  void onWeightChanged(Scale* caller) override;  //+activateScale
  void deactivateScale(Scale* caller) override;
  void addToAllScalesList(Scale* caller) override;

  void updateScreen();

private:
  int _ansicht = 0;
  int _ansichtCount = 3;  //Anzahl Ansichten
  bool _needUpdate = true;
  bool _bgForcedRefreshNeeded = true;
  unsigned long _bgRefreshTime = 10000;
  unsigned long _bgLastRefreshTime = 0;

  std::vector<const Scale*> changedScales;
  std::vector<const Scale*> activeScales;
  std::map<DeviceIndex, Scale*> allScales;

  const uint8_t ADDR = 0x27;
  const uint8_t ROWS = 20;
  const uint8_t COLL = 4;
  LiquidCrystal_I2C lcd;

  int carPos[4][2]{ { 0, 0 }, { 0, 2 }, { 12, 0 }, { 12, 2 } };

  byte heart[8] = {
    B00000,
    B00000,
    B01010,
    B10101,
    B10001,
    B01010,
    B00100,
    B00000
  };
  byte leftUP[8] = {
    B00000,
    B00000,
    B00111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100
  };
  byte leftdown[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B00111,
    B00000,
    B00000
  };
  byte rightUP[8] = {
    B00000,
    B00000,
    B11100,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100
  };
  byte rightdown[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B11100,
    B00000,
    B00000
  };
  byte upper[8] = {
    B00000,
    B00000,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  };
  byte lower[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B00000,
    B00000
  };



  float getWeight(DeviceIndex idx) const ;
  float calcProzent(DeviceIndex index);
  float calcGesamtMasse();

  void calibrierungsText(int status);
  
  void formatFloatToChar(float value, char* buffer, uint8_t intDigits, uint8_t fracDigits, FormatMode mode);
  void replaceAtCoordinate(int coll, int row, int digit, int nachkommastellen, float wert, FormatMode mode, StatusFlags status);
  void place5CharStatusCode(StatusFlags status);

  void DrawBGStandard();
  void DrawBGAuto();
  void DrawScaleStatus();
  bool bgNeedRefresh(bool reset);

};

#endif
