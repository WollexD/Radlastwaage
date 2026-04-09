#include "LED.h"

LED::LED(int ledPin) {
  pin = ledPin;

  lastChange = 0;
  state = LOW;
  mode = 0;
  step = 0;
}
void LED::begin() {
  pinMode(pin, OUTPUT);
}

void LED::setMode(int newMode) {
  if (newMode != mode && modeLocked == false) {
    mode = newMode;
    step = 0;
    lastChange = millis();
  }
}

bool LED::lockMode(bool lockIt = false) {
  modeLocked = lockIt;
  return modeLocked;
}

void LED::update() {
  unsigned long now = millis();

  switch (mode) {
    case 0:  // AUS
      digitalWrite(pin, LOW);
      break;

    case 1:  // Dauer AN
      digitalWrite(pin, HIGH);
      break;

    case 2:  // Langsames Blinken (1s)
      if (now - lastChange >= 1000) {
        toggle();
        lastChange = now;
      }
      break;

    case 3:  // Schnelles Blinken (200ms)
      if (now - lastChange >= 200) {
        toggle();
        lastChange = now;
      }
      break;

    case 4:  // Doppelblinken
      handleDoubleBlink(now);
      break;

    case 5:  // Dreifachblinken
      handleTripleBlink(now);
      break;
  }
}

void LED::toggle() {
  state = !state;
  digitalWrite(pin, state);
}

void LED::handleDoubleBlink(unsigned long now) {
  if (now - lastChange >= 200) {
    step++;
    lastChange = now;

    if (step == 1 || step == 3) {
      digitalWrite(pin, HIGH);
    } else if (step == 2 || step == 4) {
      digitalWrite(pin, LOW);
    } else if (step >= 6) {
      step = 0;  // Pause
    }
  }
}

void LED::handleTripleBlink(unsigned long now) {
  if (now - lastChange >= 150) {
    step++;
    lastChange = now;

    if (step % 2 == 1 && step <= 5) {
      digitalWrite(pin, HIGH);
    } else if (step % 2 == 0 && step <= 6) {
      digitalWrite(pin, LOW);
    } else if (step >= 8) {
      step = 0;  // Pause
    }
  }
}