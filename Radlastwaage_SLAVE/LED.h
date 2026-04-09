#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED {
  public:
    LED(int ledPin);

    void begin();
    void setMode(int newMode);
    bool lockMode(bool lockIt);
    void update();

  private:
    int pin;
    unsigned long lastChange;
    int state;
    int mode;
    bool modeLocked;
    int step;

    void toggle();
    void handleDoubleBlink(unsigned long now);
    void handleTripleBlink(unsigned long now);
};

#endif