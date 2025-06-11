#pragma once
#include <Servo.h>
#include "MeBuzzer.h"
#include "MeGyro.h"
#include "MeRGBLed.h"
#include "MeRGBLineFollower.h"
#include "MeUltrasonicSensor.h"

// Debug mód
extern const bool debugMode = false;

// Počáteční stav
extern State aktualniStav;
extern TypKrizovatky detekovanyTypKrizovatky;

// Levý motor
extern const int pwmMotorPravy;
extern const int inMotorPravy1;
extern const int inMotorPravy2;

// Pravý motor
extern const int pwmMotorLevy;
extern const int inMotorLevy1;
extern const int inMotorLevy2;

// Rychlosti
extern int rychlostJizdy;
extern int minRychlost;
extern int maxRychlost;
extern int rychlostOtaceni;

// Stavy senzoru
#define STAV_ROVNE 0b1001
#define STAV_KRIZ_T 0b0000
#define STAV_KRIZ_L 0b1000
#define STAV_KRIZ_P 0b0001
#define STAV_SLEPA 0b1111
#define STAV_MIMO_L 0b1100
#define STAV_MIMO_P 0b0011

// Servo
extern const byte servoPin;
extern Servo servo;

// Nárazníky
extern const byte pravyNaraznik;
extern const byte levyNaraznik;

// Enkodery
extern const byte pravyEnkoderA;
extern const byte pravyEnkoderB;
extern const byte levyEnkoderA;
extern const byte levyEnkoderB;
extern volatile long pulseCountVlevo;
extern volatile long pulseCountVpravo;

// RGB LED ring
extern const byte numberOfLEDs;
extern const byte rgbLEDringPin;
#define RINGALLLEDS 0
extern MeRGBLed ledRing;

// Definice barev
#define LED_ZLUTA 255, 255, 0
#define LED_ZELENA 0, 255, 0
#define LED_CERVENA 255, 0, 0
#define LED_MODRA 0, 0, 255
#define LED_MAGENTA 255, 0, 255
#define LED_INDIGO 75, 0, 130
#define LED_ORANZOVA 255, 165, 0
#define LED_CYAN 0, 255, 255
#define LED_BILA 255, 255, 255
#define LED_OFF 0, 0, 0

// Bzučák
extern const byte buzzerPin;
extern MeBuzzer buzzer;

// Gyro
extern MeGyro gyro;

// Ultrazvukový snímač
extern MeUltrasonicSensor sonar;

// Snímač čáry
extern MeRGBLineFollower RGBLineFollower;

// Stavový automat
enum State {
  INIT,
  MAPOVANI_JEDU,
  MAPOVANI_KRIZOVATKA,
  MAPOVANI_OVERUJI_ROVNE,
  MAPOVANI_OTACIM_VLEVO,
  MAPOVANI_OTACIM_VPRAVO,
  MAPOVANI_VZAD,
  MAPOVANI_DOKONCENO,
  RYCHLY_PRUJEZD_JEDU,
  RYCHLY_PRUJEZD_OTOCKA,
  CIL,
  STOP,
  CHYBA
};

extern const int DOBA_POPOJETI_MS;

// Typy detekovaných křižovatek/událostí
enum TypKrizovatky {
  ZADNY,
  KRIZOVATKA_T,
  KRIZOVATKA_L,
  KRIZOVATKA_P,
  SLEPA_ULICKA
};