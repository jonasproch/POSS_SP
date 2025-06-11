#include "config.h"
#include "led.h"
#include "motor.h"

void nouzovaBrzda() {
  nastavLEDv2(0, LED_MODRA);
  Serial.println("Nouzové zastavení!");
  zastavVse();
  aktualniStav = STOP;
  return;
}