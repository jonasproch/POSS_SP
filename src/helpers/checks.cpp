#include "config.h"
#include "led.h"
#include "motor.h"
#include "sensor.h"

void kontrolaSlepaUlicka(int state, State prevState) {
  if (state == 15 && prevState != MAPOVANI_KRIZOVATKA) {
    nastavLEDv2(2, LED_CERVENA);

    aktualniStav = MAPOVANI_VZAD;
    zastavVse();
  }
}

void kontrolaTKrizovatka(int state, State prevState) {
  if (state == 0 && prevState != MAPOVANI_VZAD) {
    zastavVse();

    detekovanyTypKrizovatky = KRIZOVATKA_T;
    aktualniStav = MAPOVANI_KRIZOVATKA;
  }
}

void kontrolaLKrizovatka(int state, State prevState) {
  if (state == 8 && prevState != MAPOVANI_VZAD) {
    zastavVse();

    detekovanyTypKrizovatky = KRIZOVATKA_L;
    aktualniStav = MAPOVANI_KRIZOVATKA;
  }
}

void kontrolaPKrizovatka(int state, State prevState) {
  if (state == 1 && prevState != MAPOVANI_VZAD) {
    zastavVse();

    detekovanyTypKrizovatky = KRIZOVATKA_P;
    aktualniStav = MAPOVANI_KRIZOVATKA;
  }
}

void kontrolaZtrataCaryLeva() {
  pravyMotorStop();
  levyMotorVpred(rychlostJizdy);
  delay(25);
}

void kontrolaZtrataCaryPrava() {
  levyMotorStop();
  pravyMotorVpred(rychlostJizdy);
  delay(25);
}

void kontrolaJizdaRovne(int state) {
  if (state == 9) {
    nastavLEDv2(1, LED_ZELENA);

    levyMotorVpred(rychlostJizdy);
    pravyMotorVpred(rychlostJizdy);
    aktualniStav = MAPOVANI_JEDU;
  }
}

void kontrolaRovne() {
  levyMotorVpred(rychlostJizdy);
  pravyMotorVpred(rychlostJizdy);
  delay(100);

  zastavVse();
  delay(100);

  RGBLineFollower.loop();
  int stavPoPopojeti = filtrujSenzor();
  delay(100);

  RGBLineFollower.loop();
  stavPoPopojeti = filtrujSenzor();

  if (stavPoPopojeti == 9) {
    aktualniStav = MAPOVANI_JEDU;
  } else {
    levyMotorVzad(rychlostJizdy);
    pravyMotorVzad(rychlostJizdy);
    delay(100);

    zastavVse();
    aktualniStav = MAPOVANI_OTACIM_VPRAVO;
  }
}