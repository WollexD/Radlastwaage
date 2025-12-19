#include "esp32-hal.h"
#include "DisplayControl.h"
#include <vector>
#include <algorithm>

DisplayControl::DisplayControl()
  : lcd(ADDR, ROWS, COLL) {}

void DisplayControl::begin() {
  lcd.init();  // initialize the lcd
  lcd.createChar(0, heart);
  lcd.createChar(1, upper);
  lcd.createChar(2, leftUP);
  lcd.createChar(3, leftdown);
  lcd.createChar(4, rightUP);
  lcd.createChar(5, rightdown);
  lcd.createChar(6, lower);

  lcd.backlight();
  lcd.clear();
}


void DisplayControl::clear() {
  lcd.clear();
}


void DisplayControl::onWeightChanged(Scale* caller) {
  // Prüfen, ob Waage schon in der Liste ist (keine doppelten Einträge)
  if (std::find(changedScales.begin(), changedScales.end(), caller) == changedScales.end()) {
    changedScales.push_back(caller);
  }
}

void DisplayControl::calibrierungsText(int status) {
  lcd.clear();

  //"--------------------"
  //"
  switch (status) {
    case 100:
      lcd.setCursor(1, 1);  //Spalte , Zeile
      lcd.print("Calibration REQ.");
      break;
    case 110:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Remove all weigt");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("from Scale + Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 111:
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Waage wird genullt");
      break;
    case 112:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage ist genullt");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 113:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Kalibrierungsgewicht");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("plazieren + Press");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Button to Continue");
      break;
    case 114:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage  ");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("wird");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("Kalibriert");
      break;
    case 115:
      lcd.setCursor(0, 0);  //Spalte , Zeile
      lcd.print("Waage ist Kalibriert");
      lcd.setCursor(0, 1);  //Spalte , Zeile
      lcd.print("Press Button to");
      lcd.setCursor(0, 2);  //Spalte , Zeile
      lcd.print("End Calibration!");
      break;
    default:
      break;
  }
}

//noch fixen
float DisplayControl::calcProzent(Scale* (&waagen)[4], int version) {
  //1 = Vorne 2 = hinten, 10 = LV, 11 = LH, 12 = RV, 13 = RH

  float gewichtVorne = waagen[LV]->getWeight() + waagen[RV]->getWeight();
  float gewichtHinten = waagen[LH]->getWeight() + waagen[RH]->getWeight();
  float gewichtGesamt = gewichtVorne + gewichtHinten;

  // Prevent division by zero
  if (gewichtGesamt == 0.0f) {
    return 0.0f;
  }

  float VtlVA = (gewichtVorne / gewichtGesamt) * 100;
  float VtlHA = (gewichtHinten / gewichtGesamt) * 100;
  switch (version) {
    case 1:
      return VtlVA;
      break;

    case 2:
      return VtlHA;
      break;

    case 10:
      return (waagen[LV]->getWeight() / gewichtGesamt) * 100;
      break;

    case 11:
      return (waagen[LH]->getWeight() / gewichtGesamt) * 100;
      break;

    case 12:
      return (waagen[RV]->getWeight() / gewichtGesamt) * 100;
      break;

    case 13:
      return (waagen[RH]->getWeight() / gewichtGesamt) * 100;
      break;

    default:
      break;
  }
  return 0;
}

float DisplayControl::calcGesamtMasse(Scale* (&waagen)[4]) {
  return waagen[0]->getWeight() + waagen[1]->getWeight() + waagen[2]->getWeight() + waagen[3]->getWeight();
}



//----NEU-----
void DisplayControl::replaceAtCoordinate(int coll, int row, int digit, int nachkommastellen, float wert, FormatMode mode = FORMAT_WEIGHT) {
  char formattedString[digit + nachkommastellen + 1];
  formatFloatToChar(wert, formattedString, digit, nachkommastellen, mode);
  lcd.setCursor(coll, row);
  lcd.print(formattedString);
}


//----Ansichtensteuerung----Start----
void DisplayControl::changeAnsicht(int newAnsicht) {
  this->_ansicht = newAnsicht;
  this->_bgLastRefreshTime = 0;
  _needUpdate = true;
}

void DisplayControl::nextAnsicht() {
  changeAnsicht((this->_ansicht + 1) % _ansichtCount);
}

void DisplayControl::setStandardansicht() {
  changeAnsicht(0);
}
//----Ansichtensteuerung----Ende----


//----Hintergründe+Steuerung----Start----
void DisplayControl::DrawBGStandard() {
  lcd.setCursor(0, 0);
  lcd.print("LV     kg|Gesamt:   ");
  lcd.setCursor(0, 1);
  lcd.print("LH     kg|        kg");
  lcd.setCursor(0, 2);
  lcd.print("RV     kg|VtlVA    %");
  lcd.setCursor(0, 3);
  lcd.print("RH     kg|VtlHA    %");
}

void DisplayControl::DrawBGAuto() {
  lcd.setCursor(0, 0);
  lcd.print("        ");
  lcd.write(byte(2));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(4));
  lcd.print("        ");

  lcd.setCursor(0, 1);
  lcd.print("        |  |        ");

  lcd.setCursor(0, 2);
  lcd.print("        |  |        ");

  lcd.setCursor(0, 3);
  lcd.print("        ");
  lcd.write(byte(3));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(5));
  lcd.print("        ");
}

bool DisplayControl::bgNeedRefresh() {
  if (millis() - _bgLastRefreshTime >= _bgRefreshTime) {
    _bgLastRefreshTime = millis();
    return true;
  }
  return false;
}
//----Hintergründe+Steuerung----Ende----



void DisplayControl::newUpdateScreen(Scale* (&waagen)[4]) {
  if (!_needUpdate && !bgNeedRefresh() && !changedScales.empty()) {
    return;
  }

  switch (this->_ansicht) {
    case 1:
      if (bgNeedRefresh()) {
        DrawBGAuto();
      }
      break;

    default:  //Anischt 0 bzw. Standardansicht
      if (bgNeedRefresh()) {
        DrawBGStandard();
      }

      // Nur Waagen updaten, die sich geändert haben
      for (const Scale* w : changedScales) {
        replaceAtCoordinate(2, w->_scaleNumber, 3, 1, w->getWeight());
      }
      // Nach dem Durchlauf Liste leeren
      changedScales.clear();

      replaceAtCoordinate(10, 1, 5, 2, calcGesamtMasse(waagen));
      replaceAtCoordinate(15, 2, 2, 1, calcProzent(waagen, 1), FORMAT_PERCENT);
      replaceAtCoordinate(15, 3, 2, 1, calcProzent(waagen, 2), FORMAT_PERCENT);
      break;
  }

  _needUpdate = false;
}

//----ALT-----
// void DisplayControl::updateScreen() {
//   if (_needUpdate) {

//     if (this->_ansicht == 0) {
//       standardLine0();
//       standardLine1();
//       standardLine2();
//       standardLine3();
//     }




//     _needUpdate = false;
//     for (int i = 0; i < 4; i++) {
//       if (_linechanged[i]) {
//         _linechanged[i] = false;
//         lcd.setCursor(0, i);
//         lcd.print(lines[i]);
//       }
//     }

//     //Start Hinzufügen von AutoBild
//     int startIndexLine = 0;
//     int startIndexColl = -1;
//     for (int i = 0; i < 4; i++) {
//       startIndexColl = findNextMarker(lines[i], 0);
//       if (startIndexColl != -1) {
//         startIndexLine = i;
//         break;
//       }
//     }

//     if (startIndexColl != -1) {
//       for (int lin = 0; lin < 4; lin++) {
//         for (int coll = 0; coll < 4; coll++) {

//           lcd.setCursor(startIndexColl + coll, startIndexLine + lin);
//           if (car[lin][coll] < 8) {
//             lcd.write(byte(car[lin][coll]));
//           } else {
//             lcd.write(car[lin][coll]);
//           }
//         }
//       }
//     }
//     //Ende Hinzufügen von AutoBild
//   }


//   // strcpy(line1, "012345678");  schreibt "012345678" in line1.
//   // strcat(line1, ausgabe6);     hängt danach ausgabe6 hinten dran.
// }



void DisplayControl::formatFloatToChar(float floatValue, char* buffer, uint8_t intDigits, uint8_t fracDigits, FormatMode mode) {

  const uint8_t width = intDigits + fracDigits + 1;

  for (uint8_t i = 0; i < width; i++) buffer[i] = ' ';
  buffer[width] = '\0';

  // Modusabhängige Grenzen
  if (mode == FORMAT_PERCENT) {
    if (floatValue < 0.0f || floatValue > 99.99f) {
      for (uint8_t i = 0; i < width; i++) buffer[i] = 'X';
      return;
    }
  }

  if (mode == FORMAT_WEIGHT) {
    floatValue /= 1000.0f;
  }
  // Skalierungsfaktor
  int32_t scale = 1;
  for (uint8_t i = 0; i < fracDigits; i++) scale *= 10;

  // Runden
  int32_t value = (int32_t)(floatValue * scale + (floatValue >= 0 ? 0.5f : -0.5f));

  bool negative = value < 0;
  value = abs(value);

  int32_t intPart = value / scale;
  int32_t fracPart = value % scale;

  // verfügbare Integer-Stellen
  uint8_t availInt = negative ? (intDigits - 1) : intDigits;

  // max darstellbarer Integer-Wert
  int32_t maxInt = 1;
  for (uint8_t i = 0; i < availInt; i++) maxInt *= 10;

  // Überlauf
  if (intPart >= maxInt) {
    if (negative) buffer[0] = '-';

    // XX,XX schreiben (rechtsbündig)
    int8_t pos = width - 1;
    for (uint8_t i = 0; i < fracDigits; i++) buffer[pos--] = 'X';
    buffer[pos--] = ',';
    for (uint8_t i = 0; i < availInt; i++) buffer[pos--] = 'X';
    return;
  }

  int8_t pos = width - 1;

  // Nachkommastellen
  for (uint8_t i = 0; i < fracDigits; i++) {
    buffer[pos--] = '0' + (fracPart % 10);
    fracPart /= 10;
  }

  // Komma
  buffer[pos--] = ',';

  // Integerteil
  if (intPart == 0) {
    buffer[pos--] = '0';
  } else {
    while (intPart > 0 && pos >= 0) {
      buffer[pos--] = '0' + (intPart % 10);
      intPart /= 10;
    }
  }

  // Minuszeichen fix links
  if (negative) buffer[0] = '-';
}
