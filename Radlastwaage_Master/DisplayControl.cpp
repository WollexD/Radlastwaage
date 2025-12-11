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


float DisplayControl::calcProzent(int version) {
  //1 = Vorne 2 = hinten, 10 = LV, 11 = LH, 12 = RV, 13 = RH

  float gewichtVorne = _lastWeights[0] + _lastWeights[2];
  float gewichtHinten = _lastWeights[1] + _lastWeights[3];
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
      return (_lastWeights[0] / gewichtGesamt) * 100;
      break;

    case 11:
      return (_lastWeights[1] / gewichtGesamt) * 100;
      break;

    case 12:
      return (_lastWeights[2] / gewichtGesamt) * 100;
      break;

    case 13:
      return (_lastWeights[3] / gewichtGesamt) * 100;
      break;

    default:
      break;
  }
  return 0;
}

//----ALT-----
void DisplayControl::standardLine0() {
  int line = 0;
  clearLine(line);
  char weightLV[6];
  formatWeight5(_lastWeights[line], weightLV);

  replaceAt(lines[line], 0, "LV");
  replaceAt(lines[line], 2, weightLV);
  replaceAt(lines[line], 7, "kg|Gesamt:");
}
//----ALT-----
void DisplayControl::standardLine1() {
  int line = 1;
  clearLine(line);
  char weightLH[6];
  formatWeight5(_lastWeights[line], weightLH);

  replaceAt(lines[line], 0, "LH");
  replaceAt(lines[line], 2, weightLH);
  replaceAt(lines[line], 7, "kg|");

  char weightSumm[7];
  formatWeight6(_lastWeights[0] + _lastWeights[1] + _lastWeights[2] + _lastWeights[3], weightSumm);
  replaceAt(lines[line], 12, weightSumm);
  replaceAt(lines[line], 18, "kg");
}
//----ALT-----
void DisplayControl::standardLine2() {
  int line = 2;
  clearLine(line);
  char weightRV[6];
  formatWeight5(_lastWeights[line], weightRV);

  replaceAt(lines[line], 0, "RV");
  replaceAt(lines[line], 2, weightRV);
  replaceAt(lines[line], 7, "kg|VtlVA");

  char vertlVA[5];
  formatVtl4(calcProzent(1), vertlVA);
  replaceAt(lines[line], 15, vertlVA);
  replaceAt(lines[line], 19, "%");
}
//----ALT-----
void DisplayControl::standardLine3() {
  int line = 3;
  clearLine(line);
  char weightRH[6];
  formatWeight5(_lastWeights[line], weightRH);

  replaceAt(lines[line], 0, "RH");
  replaceAt(lines[line], 2, weightRH);
  replaceAt(lines[line], 7, "kg|VtlHA");

  char vertlHA[5];
  formatVtl4(calcProzent(2), vertlHA);
  replaceAt(lines[line], 15, vertlHA);
  replaceAt(lines[line], 19, "%");
}
//----ALT-----
void DisplayControl::updateWeight(float weight, int scale) {
  if (long(_lastWeights[scale]) != long(weight)) {
    _lastWeights[scale] = weight;
    _weightsChanged[scale] = true;
    _needUpdate = true;
  }
}

void DisplayControl::changeAnsicht(int newAnsicht) {
  this->_ansicht = newAnsicht;
  _needUpdate = true;
}

void DisplayControl::setStandardansicht() {
  changeAnsicht(0);
}

void DisplayControl::nextAnsicht() {
  changeAnsicht((this->_ansicht + 1) % _ansichtCount);
}

void DisplayControl::AutoHintergrund() {
  _needUpdate = true;
  replaceAt(lines[0], 8, "\x01");
}

//----ALT-----
void DisplayControl::changeLine(int line, char text[21]) {
  strcpy(lines[line], "                    ");
  replaceAt(lines[line], 0, text);
  _linechanged[line] = true;
  _needUpdate = true;
}
//----ALT-----
void DisplayControl::clearLine(int line) {
  strcpy(lines[line], "                    ");
  _linechanged[line] = true;
  _needUpdate = true;
}

void DisplayControl::DrawBGStandard(){
  lcd.setCursor(0, 0);
  lcd.print("LV     kg|Gesamt:   ");
  lcd.setCursor(0, 1);
  lcd.print("LH     kg|        kg");
  lcd.setCursor(0, 2);
  lcd.print("RV     kg|VtlVA    %");
  lcd.setCursor(0, 3);
  lcd.print("RH     kg|VtlHA    %");
}

void DisplayControl::newUpdateScreen() {
  switch (this->_ansicht) {
    case 1:

      break;

    default:
      // standardLine0();
      
      break;
  }

  _needUpdate = false;
  for (int i = 0; i < 4; i++) {
    if (_linechanged[i]) {
      _linechanged[i] = false;
      lcd.setCursor(0, i);
      lcd.print(lines[i]);
    }
  }
}

//----ALT-----
void DisplayControl::updateScreen() {
  if (_needUpdate) {

    if (this->_ansicht == 0) {
      standardLine0();
      standardLine1();
      standardLine2();
      standardLine3();
    }




    _needUpdate = false;
    for (int i = 0; i < 4; i++) {
      if (_linechanged[i]) {
        _linechanged[i] = false;
        lcd.setCursor(0, i);
        lcd.print(lines[i]);
      }
    }

    //Start Hinzufügen von AutoBild
    int startIndexLine = 0;
    int startIndexColl = -1;
    for (int i = 0; i < 4; i++) {
      startIndexColl = findNextMarker(lines[i], 0);
      if (startIndexColl != -1) {
        startIndexLine = i;
        break;
      }
    }

    if (startIndexColl != -1) {
      for (int lin = 0; lin < 4; lin++) {
        for (int coll = 0; coll < 4; coll++) {

          lcd.setCursor(startIndexColl + coll, startIndexLine + lin);
          if (car[lin][coll] < 8) {
            lcd.write(byte(car[lin][coll]));
          } else {
            lcd.write(car[lin][coll]);
          }
        }
      }
    }
    //Ende Hinzufügen von AutoBild
  }


  // strcpy(line1, "012345678");  schreibt "012345678" in line1.
  // strcat(line1, ausgabe6);     hängt danach ausgabe6 hinten dran.
}

int DisplayControl::findNextMarker(const char* text, int startIndex) {
  for (int i = startIndex; i < 21; i++) {
    if (text[i] == '\x01') {
      return i;
    }
  }
  return -1;
}

//----ALT-----
void DisplayControl::replaceAt(char* dest, int pos, const char* insert) {
  size_t lenDest = strlen(dest);
  size_t lenInsert = strlen(insert);

  // Sicherheitscheck: Wenn Position größer als Länge → ans Ende schreiben
  if (pos > lenDest) pos = lenDest;

  // Temporären Speicher für den Rest anlegen
  char temp[25];  // ausreichend groß wählen!
  strcpy(temp, &dest[pos + lenInsert]);

  // Schreibe die neue Zeichenkette an die gewünschte Position
  memcpy(&dest[pos], insert, lenInsert);

  // Hänge den vorher gespeicherten Rest wieder hinten dran
  strcpy(&dest[pos + lenInsert], temp);
}
void DisplayControl::formatVtl4(float Vtl, char* buffer) {  // Funktion: Float -> "00,0"-String
  // Überlaufbehandlung
  if (Vtl < 0) {
    strcpy(buffer, "XX,X");
    return;
  }
  if (Vtl > 99.9) {
    strcpy(buffer, "XX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int Vtl_int = (int)(Vtl * 10.0 + (Vtl >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(Vtl_int / 10);
  int hinten = abs(Vtl_int % 10);

  sprintf(buffer, "%2d,%1d", vorn, hinten);  // z. B. " 12,3"
}
void DisplayControl::formatWeight5(float weight, char* buffer) {  // Funktion: Float -> "000,0"-String
  weight = weight / 1000;                                         //Umrechnung von Gramm in kg
  // Überlaufbehandlung
  if (weight < -99.9) {
    strcpy(buffer, "-XX,X");
    return;
  }
  if (weight > 999.9) {
    strcpy(buffer, "XXX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int gewicht_int = (int)(weight * 10.0 + (weight >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(gewicht_int / 10);
  int hinten = abs(gewicht_int % 10);

  // Formatieren abhängig von Vorzeichen
  if (weight < 0) {
    // Platz für '-' berücksichtigen (max. -99,9)
    sprintf(buffer, "-%2d,%1d", vorn, hinten);  // z. B. -12,3 → "-12,3"
  } else {
    // Rechtsbündig mit Leerzeichen für positive Werte
    sprintf(buffer, "%3d,%1d", vorn, hinten);  // z. B. " 12,3"
  }
}
void DisplayControl::formatWeight6(float weight, char* buffer) {  // Funktion: Float -> "0000,0"-String
  weight = weight / 1000;                                         //Umrechnung von Gramm in kg
  // Überlaufbehandlung
  if (weight < -999.9) {
    strcpy(buffer, "-XXX,X");
    return;
  }
  if (weight > 9999.9) {
    strcpy(buffer, "XXXX,X");
    return;
  }

  // Rundung auf eine Nachkommastelle
  int gewicht_int = (int)(weight * 10.0 + (weight >= 0 ? 0.5 : -0.5));

  // Ganzzahl- und Nachkommastellen extrahieren
  int vorn = abs(gewicht_int / 10);
  int hinten = abs(gewicht_int % 10);

  // Formatieren abhängig von Vorzeichen
  if (weight < 0) {
    // Platz für '-' berücksichtigen (max. -99,9)
    sprintf(buffer, "-%3d,%1d", vorn, hinten);  // z. B. -12,3 → "-12,3"
  } else {
    // Rechtsbündig mit Leerzeichen für positive Werte
    sprintf(buffer, "%4d,%1d", vorn, hinten);  // z. B. " 12,3"
  }
}