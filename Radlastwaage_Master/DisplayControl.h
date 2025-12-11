// #pragma once
#ifndef DISPLAYCONTROL_H
#define DISPLAYCONTROL_H

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


class DisplayControl : public ScaleListener {
public:
  DisplayControl();
  void begin();

  void clear();

  void calibrierungsText(int status);

  void changeAnsicht(int newAnsicht);
  void setStandardansicht();
  void nextAnsicht();

  void AutoHintergrund();
  float calcProzent(int version);
  void changeLine(int line, char text[21]);
  void clearLine(int line);
  void newUpdateScreen();
  void updateScreen();
  void updateWeight(float weight, int scale);

  void onWeightChanged(Scale* caller) override;

private:
  int _ansicht = 0;
  int _ansichtCount = 3; //Anzahl Ansichten
  std::vector<const Scale*> changedScales;
  float _lastWeights[4] = { 0.01, 0.00, 0.00, 0.00 };//----ALT-----
  bool _weightsChanged[4] = { false, false, false, false };//----ALT-----

  const uint8_t ADDR = 0x27;
  const uint8_t ROWS = 20;
  const uint8_t COLL = 4;
  LiquidCrystal_I2C lcd;


  void standardLine0();  //----ALT-----
  void standardLine1();//----ALT-----
  void standardLine2();//----ALT-----
  void standardLine3();//----ALT-----

  bool _linechanged[4] = { false, false, false, false };//----ALT-----
  bool _needUpdate = true;

  char lines[4][21] = { "", "", "", "" };//----ALT-----

  int car[4][4] = {
    { 2, 1, 1, 4 },
    { 124, 32, 32, 124 },
    { 124, 32, 32, 124 },
    { 3, 6, 6, 5 },
  };

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

  int findNextMarker(const char* text, int startIndex); //----ALT-----
  void replaceAt(char* dest, int pos, const char* insert);//----ALT-----
  void formatVtl4(float Vtl, char* buffer);
  void formatWeight5(float weight, char* buffer);
  void formatWeight6(float weight, char* buffer);
};

#endif
