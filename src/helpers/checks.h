#pragma once
#include "config.h"

void kontrolaSlepaUlicka(int state, State prevState);
void kontrolaTKrizovatka(int state, State prevState);
void kontrolaLKrizovatka(int state, State prevState);
void kontrolaPKrizovatka(int state, State prevState);
void kontrolaZtrataCaryLeva();
void kontrolaZtrataCaryPrava();
void kontrolaJizdaRovne(int state);
void kontrolaRovne();