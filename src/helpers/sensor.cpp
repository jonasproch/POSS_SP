#include "config.h"

int filtrujSenzor() {
  static int posledniHodnoty[5];
  int posledniHodnotySize = sizeof posledniHodnoty / sizeof posledniHodnoty[0];
  static bool initialized = false;

  if (!initialized) {
    for (int i = 0; i < posledniHodnotySize; ++i) {
      posledniHodnoty[i] = -1;
    }
    initialized = true;
  }

  byte newValue = RGBLineFollower.getPositionState();

  // Posunutí prvků
  for (int i = posledniHodnotySize - 1; i > 0; --i) {
    posledniHodnoty[i] = posledniHodnoty[i - 1];
  }
  posledniHodnoty[0] = newValue;

  int counts[16] = {0};

  for (int i = 0; i < posledniHodnotySize; ++i) {
    if (posledniHodnoty[i] >= 0 && posledniHodnoty[i] < 16) {
      counts[posledniHodnoty[i]]++;
    }
  }

  int states[16];
  for (int i = 0; i < 16; ++i) {
    states[i] = i;
  }

  for (int i = 0; i < 15; ++i) {
    for (int j = i + 1; j < 16; ++j) {
      if (counts[states[j]] > counts[states[i]]) {
        int temp = states[i];
        states[i] = states[j];
        states[j] = temp;
      }
    }
  }

  for (int state : states) {
    if (counts[state] == 0) break;

    if (state != 0 || state != 8 || state != 1 || state != 15) {
      return state;
    }

    if (counts[state] >= 3) {
      return state;
    }
  }

  return 9;
  // static int posledniHodnoty[5] = {-1, -1, -1, -1, -1};
  // static byte index = 0;
  // // Načtení nové hodnoty ze senzoru
  // byte newValue = RGBLineFollower.getPositionState();

  // if (posledniHodnoty[0] == -1) {
  //   for (int i = 0; i < 5; ++i) {
  //     posledniHodnoty[i] = newValue;
  //   }
  // }

  // posledniHodnoty[index] = newValue;
  // index = (index + 1) % 5;
  // int counts[16] = {0};

  // for (int i = 0; i < 5; ++i) {
  //   if (posledniHodnoty[i] >= 0 && posledniHodnoty[i] < 16) {
  //     counts[posledniHodnoty[i]]++;
  //   }
  // }

  // int majorState = posledniHodnoty[index];
  // int maxCount = 0;

  // for (int i = 0; i < 16; ++i) {
  //   if (counts[i] > maxCount) {
  //     maxCount = counts[i];
  //     majorState = i;
  //   }
  // }

  // Serial.print("Majority bitmask: ");
  // Serial.println(majorState);
  // return majorState;
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