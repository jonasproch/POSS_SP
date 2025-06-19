#include "helpers/config.h"
#include "helpers/led.h"
#include "helpers/motor.h"
#include "helpers/sensor.h"
#include "helpers/sound.h"

void stavovyAutomat() {
  int sensorStateFull;

  static unsigned long posledniCas = 0;
  unsigned long aktualniCas = millis();

  // Kontrola čáry
  if (aktualniCas - posledniCas >= 10) {
    RGBLineFollower.loop();
    sensorStateFull = filtrujSenzor();
    posledniCas = aktualniCas;
  }

  byte sensorNonFiltered = RGBLineFollower.getPositionState();

  State predchoziStavAutomatu = aktualniStav;

  switch (aktualniStav) {
    case MAPOVANI_JEDU: {
      // Ztráta čáry levá
      if (sensorNonFiltered == 11) {
        if (debugMode)
          Serial.println("Detekce: Ztrata cary leva!");

        pravyMotorStop();
        levyMotorVpred(rychlostJizdy);
        delay(25);
      }

      // Ztráta čáry prává
      if (sensorNonFiltered == 13 || sensorNonFiltered == 12) {
        if (debugMode)
          Serial.println("Detekce: Ztrata cary prava!");

        levyMotorStop();
        pravyMotorVpred(rychlostJizdy);
        delay(25);
      }

      // Slepá ulička
      if (sensorStateFull == 15 &&
          predchoziStavAutomatu != MAPOVANI_KRIZOVATKA) {
        if (debugMode)
          Serial.println("Detekce: Slepa ulicka");

        nastavLEDv2(2, LED_CERVENA);

        aktualniStav = MAPOVANI_VZAD;
        zastavVse();
      }

      // T křižovatka
      if (sensorStateFull == 0 && predchoziStavAutomatu != MAPOVANI_VZAD) {
        zastavVse();

        if (debugMode)
          Serial.println("Detekce: T-krizovatka");

        detekovanyTypKrizovatky = KRIZOVATKA_T;
        aktualniStav = MAPOVANI_KRIZOVATKA;
      }

      // Křižovatka doleva
      if (sensorStateFull == 8 && predchoziStavAutomatu != MAPOVANI_VZAD) {
        zastavVse();

        if (debugMode)
          Serial.println("Detekce: Krizovatka doleva");

        detekovanyTypKrizovatky = KRIZOVATKA_L;
        aktualniStav = MAPOVANI_KRIZOVATKA;
      }

      // Křižovatka doprava
      if (sensorStateFull == 1 && predchoziStavAutomatu != MAPOVANI_VZAD) {
        zastavVse();

        if (debugMode)
          Serial.println("Detekce: Krizovatka doprava");

        detekovanyTypKrizovatky = KRIZOVATKA_P;
        aktualniStav = MAPOVANI_KRIZOVATKA;
      }

      // Jízda rovně
      if (sensorStateFull == 9) {
        nastavLEDv2(1, LED_ZELENA);

        if (debugMode)
          Serial.println("Detekce: Jedu rovne po cary");

        levyMotorVpred(rychlostJizdy);
        pravyMotorVpred(rychlostJizdy);
        aktualniStav = MAPOVANI_JEDU;
      }

      break;
    }

    case MAPOVANI_KRIZOVATKA: {
      RGBLineFollower.loop();

      Serial.println("křižovatka");
      Serial.println(detekovanyTypKrizovatky);
      Serial.println(aktualniStav);

      if (detekovanyTypKrizovatky == 1) {
        Serial.println("T-križovatka");
        nastavLEDv2(5, LED_CYAN);
        aktualniStav = MAPOVANI_OTACIM_VLEVO;
      }

      if (detekovanyTypKrizovatky == 2) {
        Serial.println("L-križovatka");
        nastavLEDv2(3, LED_MAGENTA);
        aktualniStav = MAPOVANI_OTACIM_VLEVO;
      }

      if (detekovanyTypKrizovatky == 3) {
        Serial.println("P-križovatka");
        nastavLEDv2(4, LED_INDIGO);

        if (debugMode)
          Serial.println(
              "Resim krizovatku (T nebo P) -> Pravidlo #2: "
              "Pokracuji ROVNE.");

        levyMotorVpred(rychlostJizdy);
        pravyMotorVpred(rychlostJizdy);
        aktualniStav = MAPOVANI_JEDU;
      }

      break;
    }

      // case MAPOVANI_OVERUJI_ROVNE: {
      //   Serial.println("hit MAPOVANI_OVERUJI_ROVNE");

      //   levyMotorVpred(rychlostJizdy);
      //   pravyMotorVpred(rychlostJizdy);
      //   delay(100);

      //   zastavVse();
      //   delay(100);

      //   RGBLineFollower.loop();
      //   int stavPoPopojeti = filtrujSenzor();
      //   delay(100);

      //   RGBLineFollower.loop();
      //   stavPoPopojeti = filtrujSenzor();

      //   if (stavPoPopojeti == 9) {
      //     aktualniStav = MAPOVANI_JEDU;
      //   } else {
      //     levyMotorVzad(rychlostJizdy);
      //     pravyMotorVzad(rychlostJizdy);
      //     delay(100);

      //     zastavVse();
      //     aktualniStav = MAPOVANI_OTACIM_VPRAVO;
      //   }

      //   break;
      // }

    case MAPOVANI_OTACIM_VLEVO: {
      zastavVse();

      if (debugMode) {
        Serial.println("Stav: Otacim doleva (s popojetim)...");
        Serial.println("Popojizdim vpred...");
      }

      levyMotorVpred(130);
      pravyMotorVpred(150);
      delay(105);

      zastavVse();
      delay(150);

      boolean turningLeft = true;
      pravyMotorVpred(rychlostOtaceni);
      levyMotorVzad(rychlostOtaceni);
      delay(400);

      while (turningLeft == true) {
        RGBLineFollower.loop();
        delay(10);

        int stav = filtrujSenzor();

        if (stav == 0) {
          aktualniStav = CIL;
          break;
        }

        if (stav == 9) {
          turningLeft = false;
        }
      }

      zastavVse();

      if (debugMode)
        Serial.println("Casovane otoceni doleva (s popojetim) dokonceno.");

      aktualniStav = MAPOVANI_JEDU;
      break;
    }

    case MAPOVANI_OTACIM_VPRAVO: {
      zastavVse();

      if (debugMode)
        Serial.println("Stav: Otacim doprava (s popojetim)...");
      Serial.println("   Popojizdim vpred...");

      levyMotorVpred(150);
      pravyMotorVpred(130);
      delay(105);

      zastavVse();
      delay(150);

      boolean turningRight = true;
      levyMotorVpred(rychlostOtaceni);
      pravyMotorVzad(rychlostOtaceni);
      delay(400);

      while (turningRight == true) {
        RGBLineFollower.loop();
        delay(10);

        int stav = filtrujSenzor();

        if (stav == 9) {
          turningRight = false;
        }
      }

      zastavVse();

      if (debugMode)
        Serial.println("Casovane otoceni doprava (s popojetim) dokonceno.");

      aktualniStav = MAPOVANI_JEDU;
      break;
    }

    case MAPOVANI_VZAD: {
      boolean turningBack = true;
      pravyMotorVzad(rychlostOtaceni);
      levyMotorVpred(rychlostOtaceni);

      while (turningBack == true) {
        RGBLineFollower.loop();
        delay(10);

        int stav = filtrujSenzor();

        if (stav == 9) {
          turningBack = false;
        }
      }

      aktualniStav = MAPOVANI_JEDU;
      break;
    }

    case MAPOVANI_DOKONCENO:
      if (debugMode)
        Serial.println("Stav: Mapovani dokonceno (NEIMPLEMENTOVANO)");

      zastavVse();
      aktualniStav = STOP;
      break;

    case RYCHLY_PRUJEZD_JEDU:
      if (debugMode)
        Serial.println("Stav: Rychly prujezd - jedu (NEIMPLEMENTOVANO)");

      zastavVse();
      aktualniStav = STOP;
      break;

    case RYCHLY_PRUJEZD_OTOCKA:
      if (debugMode)
        Serial.println("Stav: Rychly prujezd - otocka (NEIMPLEMENTOVANO)");
      zastavVse();

      aktualniStav = STOP;
      break;

    case CIL:
      nastavLED(LED_BILA);

      if (debugMode)
        Serial.println("Stav: CIL!");

      zastavVse();
      oslavnaMelodie();
      aktualniStav = STOP;
      break;

    case INIT:
      if (debugMode)
        Serial.println("Chyba: Vracen stav INIT!");

      zastavVse();
      aktualniStav = STOP;
      break;

    case CHYBA:
      nastavLED(LED_CERVENA);

      if (debugMode)
        Serial.println("Stav: CHYBA!");

      zastavVse();
      aktualniStav = STOP;
      break;

    default:
      if (debugMode)
        Serial.println("Neznamy stav!");

      nastavLED(LED_CERVENA);
      zastavVse();
      aktualniStav = STOP;
      break;
  }

  if (predchoziStavAutomatu != aktualniStav) {
    Serial.print("AKTUALNI STAV: ");
    Serial.println(aktualniStav);
  }

  // if (aktualniStav != predchoziStavAutomatu && debugMode) {
  //   Serial.print("Zmena stavu z ");
  //   Serial.print(predchoziStavAutomatu);
  //   Serial.print(" na ");
  //   Serial.println(aktualniStav);
  // }
}