#include <TasterControl.h>

TasterControl myButton;

void setup() {
  Serial.begin(9600);
  myButton.begin(12);  // Kein Pullup, da externer Pulldown verwendet wird
}

void loop() {
  TasterEvent event = myButton.update();

  switch (event) {
    case KURZER_DRUCK:
      Serial.println("Kurzer Druck");
      break;
    case DOPPELKLICK:
      Serial.println("Doppelklick");
      break;
    case LANGER_DRUCK:
      Serial.println("Langer Druck");
      break;
    case SEHR_LANGER_DRUCK:
      Serial.println("Sehr langer Druck");
      break;
    case EXTRA_LANGER_DRUCK:
      Serial.println("Extra langer Druck");
      break;
    default:
      break;
  }
}
