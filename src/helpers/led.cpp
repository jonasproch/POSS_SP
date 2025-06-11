#include "config.h"

void nastavLEDv2(int type, int r, int g, int b) {
  for (byte i = 1; i <= numberOfLEDs; i++) {
    ledRing.setColor(i, 0, 0, 0);
  }

  switch (type) {
    case 0:
      // kříž - čekání na start
      ledRing.setColor(12, r, g, b);
      ledRing.setColor(3, r, g, b);
      ledRing.setColor(6, r, g, b);
      ledRing.setColor(9, r, g, b);
      break;
    case 1:
      // jízda rovně
      ledRing.setColor(3, r, g, b);
      break;
    case 2:
      // slepá ulička
      ledRing.setColor(9, r, g, b);
      break;
    case 3:
      // pohyb vlevo
      ledRing.setColor(12, r, g, b);
      break;
    case 4:
      // pohyb vpravo
      ledRing.setColor(6, r, g, b);
      break;
    case 5:
      // křižovatka
      ledRing.setColor(12, r, g, b);
      ledRing.setColor(6, r, g, b);
      break;
  }
  ledRing.show();
}

void nastavLED(int r, int g, int b) {
  ledRing.setColor(RINGALLLEDS, r, g, b);
  ledRing.show();
}