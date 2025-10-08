#include "TasterControl.h"

TasterControl::TasterControl() {}

void TasterControl::begin(uint8_t pin, bool usePullup) {
  _pin = pin;
  _usePullup = usePullup;
  if (_usePullup) {
    pinMode(_pin, INPUT_PULLUP);
    _buttonState = HIGH;
    _lastReading = HIGH;
  } else {
    pinMode(_pin, INPUT);
    _buttonState = LOW;
    _lastReading = LOW;
  }
}

TasterEvent TasterControl::update() {
  bool reading = digitalRead(_pin);
  TasterEvent event = KEIN_EVENT;

  if (reading != _lastReading) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceTime) {
    if (reading != _buttonState) {
      _buttonState = reading;

      if (_buttonState == (_usePullup ? LOW : HIGH)) {
        _buttonPressStart = millis();
      } else {
        unsigned long pressDuration = millis() - _buttonPressStart;

        if (pressDuration >= 10000) {
          event = EXTRA_LANGER_DRUCK;
        } else if (pressDuration >= 5000) {
          event = SEHR_LANGER_DRUCK;
        } else if (pressDuration >= 500) {
          event = LANGER_DRUCK;
        } else {
          _clickCount++;
          _lastClickTime = millis();
          _clickPending = true;
        }
      }
    }
  }

  if (_clickPending && (millis() - _lastClickTime > _doubleClickGap)) {
    if (_clickCount == 1) {
      event = KURZER_DRUCK;
    } else if (_clickCount == 2) {
      event = DOPPELKLICK;
    }
    _clickCount = 0;
    _clickPending = false;
  }

  _lastReading = reading;
  return event;
}
