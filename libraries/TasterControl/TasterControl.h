#ifndef TASTERCONTROL_H
#define TASTERCONTROL_H

#include <Arduino.h>

enum TasterEvent {
  KEIN_EVENT,
  KURZER_DRUCK,
  DOPPELKLICK,
  LANGER_DRUCK,
  SEHR_LANGER_DRUCK,
  EXTRA_LANGER_DRUCK
};

class TasterControl {
public:
  TasterControl();
  void begin(uint8_t pin, bool usePullup = false);
  bool isBlocking();
  TasterEvent update();

private:
  uint8_t _pin;
  bool _usePullup;

  unsigned long _debounceTime = 50;
  unsigned long _doubleClickGap = 400;
  unsigned long _buttonPressStart = 0;
  unsigned long _lastDebounceTime = 0;
  unsigned long _lastClickTime = 0;

  bool _buttonState = LOW;
  bool _lastReading = LOW;

  int _clickCount = 0;
  bool _clickPending = false;
};

#endif
