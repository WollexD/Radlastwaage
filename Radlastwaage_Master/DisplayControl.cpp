#include <map>
#include <vector>
#include <algorithm>
#include "esp32-hal.h"
#include "DisplayControl.h"

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

float DisplayControl::getWeight(DeviceIndex idx) const {
  auto it = allScales.find(idx);
  return (it != allScales.end()) ? it->second->getWeight() : 0.0f;
}


void DisplayControl::onWeightChanged(Scale* caller) {
  // Prüfen, ob Waage schon in der Liste ist (keine doppelten Einträge)
  if (std::find(changedScales.begin(), changedScales.end(), caller) == changedScales.end()) {
    changedScales.push_back(caller);
  }
  if (std::find(activeScales.begin(), activeScales.end(), caller) == activeScales.end()) {
    activeScales.push_back(caller);
  }
}

void DisplayControl::deactivateScale(Scale* caller) {
  activeScales.erase(
    std::remove(activeScales.begin(), activeScales.end(), caller),
    activeScales.end());
}

void DisplayControl::addToAllScalesList(Scale* caller) {
  DeviceIndex idx = caller->getIndex();
  allScales[idx] = caller;  // überschreibt automatisch, falls schon vorhanden
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
float DisplayControl::calcProzent(DeviceIndex index) {
  //10 = Vorne 20 = hinten, 0 = LV, 1 = LH, 2 = RV, 3 = RH
  float gewichtVorne = getWeight(LV) + getWeight(RV);
  float gewichtHinten = getWeight(LH) + getWeight(RH);
  float gewichtGesamt = gewichtVorne + gewichtHinten;

  // Prevent division by zero
  if (gewichtGesamt == 0.0f) {
    return 0.0f;
  }

  float VtlVA = (gewichtVorne / gewichtGesamt) * 100;
  float VtlHA = (gewichtHinten / gewichtGesamt) * 100;
  switch (index) {
    case Vorne:
      return VtlVA;
      break;

    case Hinten:
      return VtlHA;
      break;

    case LV:
      return (getWeight(LV) / gewichtGesamt) * 100;
      break;

    case LH:
      return (getWeight(LH) / gewichtGesamt) * 100;
      break;

    case RV:
      return (getWeight(RV) / gewichtGesamt) * 100;
      break;

    case RH:
      return (getWeight(RH) / gewichtGesamt) * 100;
      break;

    default:
      break;
  }
  return 0;
}

float DisplayControl::calcGesamtMasse() {
  return getWeight(LV) + getWeight(RV) + getWeight(LH) + getWeight(RH);
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
  lcd.print("LV N.C.kg|Gesamt:   ");
  lcd.setCursor(0, 1);
  lcd.print("LH N.C.kg|        kg");
  lcd.setCursor(0, 2);
  lcd.print("RV N.C.kg|VtlVA    %");
  lcd.setCursor(0, 3);
  lcd.print("RH N.C.kg|VtlHA    %");
}

void DisplayControl::DrawBGAuto() {
  lcd.setCursor(0, 0);
  lcd.print("  N.C.kg");
  lcd.write(byte(2));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(4));
  lcd.print("  N.C.kg");

  lcd.setCursor(0, 1);
  lcd.print("  N.C.% |  |  N.C.% ");

  lcd.setCursor(0, 2);
  lcd.print("  N.C.kg|  |  N.C.kg");

  lcd.setCursor(0, 3);
  lcd.print("  N.C.% ");
  lcd.write(byte(3));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(5));
  lcd.print("  N.C.% ");
}

bool DisplayControl::bgNeedRefresh() {
  if (millis() - this->_bgLastRefreshTime >= this->_bgRefreshTime) {
    // _bgLastRefreshTime = millis(); Zurücksetzten darf erst nach aktuallisierung erfolgen
    return true;
  }
  return false;
}
//----Hintergründe+Steuerung----Ende----



void DisplayControl::updateScreen() {
  if (!(!_needUpdate || bgNeedRefresh() || !changedScales.empty())) {
    return;
  }

  switch (this->_ansicht) {
    case 1:  //Ansicht 1 (Auto mit Radgewichten)
      if (bgNeedRefresh()) {
        DrawBGAuto();
        _bgLastRefreshTime = millis();
      }

      // Nur Waagen updaten, die sich geändert haben
      for (const Scale* w : changedScales) {
        replaceAtCoordinate(carPos[w->_scaleNumber][0], carPos[w->_scaleNumber][1], 3, 2, w->getWeight());
      }
      // Nach dem Durchlauf Liste leeren
      changedScales.clear();
      // for (DeviceIndex i = DeviceIndex::LV; i < DeviceIndex::MASTER; ++i) {

      //   replaceAtCoordinate(carPos[static_cast<int>(i)][0] + 1, carPos[static_cast<int>(i)][1] + 1, 2, 2, calcProzent(i), FORMAT_PERCENT);
      // }

      for (const Scale* w : activeScales) {
        replaceAtCoordinate(carPos[w->getIndex()][0] + 1, carPos[w->getIndex()][1] + 1, 2, 2, calcProzent(w->getIndex()), FORMAT_PERCENT);
      }
      // replaceAtCoordinate(0, 0, 4, 2, waagen[LV]->getWeight());
      // replaceAtCoordinate(0, 3, 4, 2, waagen[LH]->getWeight());
      // replaceAtCoordinate(13, 0, 4, 2, waagen[RV]->getWeight());
      // replaceAtCoordinate(13, 3, 4, 2, waagen[RH]->getWeight());



      break;

    default:  //Ansicht 0 bzw. Standardansicht
      if (bgNeedRefresh()) {
        DrawBGStandard();
        _bgLastRefreshTime = millis();
        for (const Scale* w : activeScales) {
          replaceAtCoordinate(2, w->_scaleNumber, 3, 1, w->getWeight());
        }
      } else {

        // Nur Waagen updaten, die sich geändert haben
        for (const Scale* w : changedScales) {
          replaceAtCoordinate(2, w->_scaleNumber, 3, 1, w->getWeight());
        }
        // Nach dem Durchlauf Liste leeren
        changedScales.clear();
      }

      replaceAtCoordinate(10, 1, 5, 2, calcGesamtMasse());
      replaceAtCoordinate(15, 2, 2, 1, calcProzent(Vorne), FORMAT_PERCENT);
      replaceAtCoordinate(15, 3, 2, 1, calcProzent(Hinten), FORMAT_PERCENT);
      break;
  }

  _needUpdate = false;
}

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
