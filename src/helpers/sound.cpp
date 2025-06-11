#include "config.h"

void oslavnaMelodie() {
  buzzer.tone(880, 500);
  delay(500);
  buzzer.noTone();
}