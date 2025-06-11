#include <Servo.h>
#include "MeBuzzer.h"
#include "MeGyro.h"
#include "MeRGBLed.h"
#include "MeRGBLineFollower.h"
#include "MeUltrasonicSensor.h"

// Debug mód
const bool debugMode = false; // Pro tisk na Serial

// Počáteční stav
State aktualniStav = INIT;
TypKrizovatky detekovanyTypKrizovatky = ZADNY;

// Levý motor
const int pwmMotorPravy = 11;
const int inMotorPravy1 = 49;
const int inMotorPravy2 = 48;

// Pravý motor
const int pwmMotorLevy = 10;
const int inMotorLevy1 = 47;
const int inMotorLevy2 = 46;

// Rychlosti
int rychlostJizdy = 120;
int minRychlost = 50;
int maxRychlost = 150;
int rychlostOtaceni = 100;

// Stavy senzoru
#define STAV_ROVNE 0b1001
#define STAV_KRIZ_T 0b0000
#define STAV_KRIZ_L 0b1000
#define STAV_KRIZ_P 0b0001
#define STAV_SLEPA 0b1111
#define STAV_MIMO_L 0b1100
#define STAV_MIMO_P 0b0011

// Servo
const byte servoPin = 68;
Servo servo;

// Nárazníky
const byte pravyNaraznik = 67;
const byte levyNaraznik = 62;

// Enkodery
const byte pravyEnkoderA = 19;
const byte pravyEnkoderB = 42;
const byte levyEnkoderA = 18;
const byte levyEnkoderB = 43;
volatile long pulseCountVlevo = 0;
volatile long pulseCountVpravo = 0;

// RGB LED ring
const byte numberOfLEDs = 12;
const byte rgbLEDringPin = 44;
#define RINGALLLEDS 0
MeRGBLed ledRing(0, numberOfLEDs);

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
const byte buzzerPin = 45;
MeBuzzer buzzer;

// Gyro
MeGyro gyro(1, 0x69);

// Ultrazvukový snímač
MeUltrasonicSensor sonar(PORT_10);

// Snímač čáry
MeRGBLineFollower RGBLineFollower(PORT_9);

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

const int DOBA_POPOJETI_MS = 350;

// Typy detekovaných křižovatek/událostí
enum TypKrizovatky {
  ZADNY,
  KRIZOVATKA_T,  // Všechny senzory na černé (0b0000)
  KRIZOVATKA_L,  // Odbočka pouze vlevo (0b1000)
  KRIZOVATKA_P,  // Odbočka pouze vpravo (0b0001)
  SLEPA_ULICKA   // Všechny senzory na bílé (0b1111)
};