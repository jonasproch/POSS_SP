#include "helpers/actions.h"
#include "helpers/config.h"
#include "helpers/encoder.h"
#include "helpers/led.h"
#include "helpers/motor.h"
#include "helpers/sensor.h"
#include "helpers/sound.h"
#include "stavovy_automat.h"

// Setup
void setup() {
  Serial.begin(115200);
  Serial.println("Start Setup");

  // Nárazník
  pinMode(pravyNaraznik, INPUT_PULLUP);
  pinMode(levyNaraznik, INPUT_PULLUP);

  // Motory
  pinMode(pwmMotorPravy, OUTPUT);
  pinMode(inMotorPravy1, OUTPUT);
  pinMode(inMotorPravy2, OUTPUT);
  pinMode(pwmMotorLevy, OUTPUT);
  pinMode(inMotorLevy1, OUTPUT);
  pinMode(inMotorLevy2, OUTPUT);

  // Frekvence PWM
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);

  // Enkodéry
  pinMode(pravyEnkoderA, INPUT_PULLUP);
  pinMode(pravyEnkoderB, INPUT_PULLUP);
  pinMode(levyEnkoderA, INPUT_PULLUP);
  pinMode(levyEnkoderB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pravyEnkoderA), &pravyEncoderAInt,
                  CHANGE);
  attachInterrupt(digitalPinToInterrupt(levyEnkoderA), &levyEncoderAInt,
                  CHANGE);

  // Servo
  servo.attach(servoPin);
  servo.write(90);

  // RGB LED ring
  ledRing.setpin(rgbLEDringPin);
  nastavLED(LED_OFF);

  // Bzučák
  buzzer.setpin(buzzerPin);
  buzzer.noTone();

  // Gyro
  gyro.begin();

  // Sledování čáry
  RGBLineFollower.begin();

  Serial.println("Cekam na stisk leveho narazniku...");
  // Signalizace čekání na start
  nastavLEDv2(0, LED_ZLUTA);

  while (digitalRead(levyNaraznik)) {
  }

  // Chování při startu
  buzzer.tone(440, 100);
  Serial.println("Startuji!");
  nastavLEDv2(1, LED_ZELENA);
  aktualniStav = MAPOVANI_JEDU;
}

void loop() {
  // Nouzová brzda
  if (digitalRead(pravyNaraznik) == LOW) {
    nouzovaBrzda();
  }

  if (aktualniStav == STOP) {
    zastavVse();
    return;
  }

  stavovyAutomat();
}
