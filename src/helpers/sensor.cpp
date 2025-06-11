#include "config.h"

int filtrujSenzor() {
  static int posledniHodnoty[5] = {-1, -1, -1, -1, -1};
  static byte index = 0;
  // Načtení nové hodnoty ze senzoru
  byte newValue = RGBLineFollower.getPositionState();

  if (posledniHodnoty[0] == -1) {
    for (int i = 0; i < 5; ++i) {
      posledniHodnoty[i] = newValue;
    }
  }

  posledniHodnoty[index] = newValue;
  index = (index + 1) % 5;
  int counts[16] = {0};

  for (int i = 0; i < 5; ++i) {
    if (posledniHodnoty[i] >= 0 && posledniHodnoty[i] < 16) {
      counts[posledniHodnoty[i]]++;
    }
  }

  int majorState = posledniHodnoty[index];
  int maxCount = 0;

  for (int i = 0; i < 16; ++i) {
    if (counts[i] > maxCount) {
      maxCount = counts[i];
      majorState = i;
    }
  }

  Serial.print("Majority bitmask: ");
  Serial.println(majorState);
  return majorState;
}

bool jeMoznostLeva() {
  const int MASKA_LEVY_KRAJNI = 0b1000;
  int stav = RGBLineFollower.getPositionState();

  if (debugMode) {
    Serial.print("Kontrola vlevo: stav senzoru = ");
    Serial.println(stav, BIN);
  }

  // Kontrola levé pozice
  return (stav & MASKA_LEVY_KRAJNI) == 0;
}

bool jeMoznostPrava() {
  const int MASKA_PRAVY_KRAJNI = 0b0001;
  int stav = RGBLineFollower.getPositionState();

  if (debugMode) {
    Serial.print("Kontrola vpravo: stav senzoru = ");
    Serial.println(stav, BIN);
  }

  // Kontrola pravé pozice
  return (stav & MASKA_PRAVY_KRAJNI) == 0;
}