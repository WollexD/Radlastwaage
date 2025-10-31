// #pragma once
#ifndef DISPLAYCONTROL_H
#define DISPLAYCONTROL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>


// enum DeviceIndex {
//     LV     = 0,
//     LH     = 1,
//     RV     = 2,
//     RH     = 3,
//     MASTER = 4
// };


class DisplayControl {
public:
  DisplayControl();
  void begin();
  
  void calibrierungsText(int status);

  void Standardansicht(int updateLine = 0);
  void AutoHintergrund();
  
  float calcProzent(int version);
  void changeLine(int line, char text[21]);
  void clearLine(int line);
  void updateScreen();
  void updateWeight(float weight, int scale);
  void changeAnsicht(int newAnsicht);

private:
  int Ansicht = 0;
  float _lastWeights[4] = { 0.01, 0.00, 0.00, 0.00};
  bool _weightsChanged[4] = { false, false, false, false};

  const uint8_t ADDR = 0x27;
  const uint8_t ROWS = 20;
  const uint8_t COLL = 4;
  LiquidCrystal_I2C lcd;


  void standardLine0();
  void standardLine1();
  void standardLine2();
  void standardLine3();

  bool _linechanged[4] = {false, false, false, false};
  bool _needUpdate = true;

  char lines[4][21] = {"", "", "", ""};

  int car[4][4] = {
    {2, 1, 1, 4},
    {124, 32, 32, 124},
    {124, 32, 32, 124},
    {3, 6, 6, 5},
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

  int findNextMarker(const char *text, int startIndex);
  void replaceAt(char *dest, int pos, const char *insert);
  void formatVtl4(float Vtl, char* buffer); 
  void formatWeight5(float weight, char* buffer); 
  void formatWeight6(float weight, char* buffer); 

};

#endif
