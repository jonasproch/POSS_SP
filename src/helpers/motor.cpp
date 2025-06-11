#include "config.h"

void levyMotorVpred(int rychlost) {
  digitalWrite(inMotorLevy1, HIGH);
  digitalWrite(inMotorLevy2, LOW);
  analogWrite(pwmMotorLevy, constrain(rychlost, 0, 255));
}

void levyMotorVzad(int rychlost) {
  digitalWrite(inMotorLevy1, LOW);
  digitalWrite(inMotorLevy2, HIGH);
  analogWrite(pwmMotorLevy, constrain(rychlost, 0, 255));
}

void levyMotorStop() {
  digitalWrite(inMotorLevy1, LOW);
  digitalWrite(inMotorLevy2, LOW);
  analogWrite(pwmMotorLevy, 0);
}

void pravyMotorVpred(int rychlost) {
  digitalWrite(inMotorPravy1, LOW);
  digitalWrite(inMotorPravy2, HIGH);
  analogWrite(pwmMotorPravy, constrain(rychlost, 0, 255));
}

void pravyMotorVzad(int rychlost) {
  digitalWrite(inMotorPravy1, HIGH);
  digitalWrite(inMotorPravy2, LOW);
  analogWrite(pwmMotorPravy, constrain(rychlost, 0, 255));
}

void pravyMotorStop() {
  digitalWrite(inMotorPravy1, LOW);
  digitalWrite(inMotorPravy2, LOW);
  analogWrite(pwmMotorPravy, 0);
}

void zastavVse() {
  levyMotorStop();
  pravyMotorStop();
}