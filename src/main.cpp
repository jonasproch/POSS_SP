#include "MeAuriga.h"
#include "MeRGBLineFollower.h"
#include <Arduino.h>
#include <Servo.h>

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

// Ultrazvukový snímač
MeUltrasonicSensor sonar(PORT_10);

// Snímač čáry
MeRGBLineFollower RGBLineFollower(PORT_9);

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

// Počáteční stav
State aktualniStav = INIT;

const int DOBA_POPOJETI_MS = 350;

// Typy detekovaných křižovatek/událostí
enum TypKrizovatky {
    ZADNY,
    KRIZOVATKA_T, // Všechny senzory na černé (0b0000)
    KRIZOVATKA_L, // Odbočka pouze vlevo (0b1000)
    KRIZOVATKA_P, // Odbočka pouze vpravo (0b0001)
    SLEPA_ULICKA  // Všechny senzory na bílé (0b1111)
};

TypKrizovatky detekovanyTypKrizovatky = ZADNY;

bool debugMode = false; // Pro tisk na Serial

// Ovládání motorů
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
    digitalWrite(inMotorLevy1, LOW); // Brzdění zkratem může být efektivnější
    digitalWrite(inMotorLevy2, LOW);
    analogWrite(pwmMotorLevy, 0); // Alternativně volný doběh
}

void pravyMotorVpred(int rychlost) {
    digitalWrite(inMotorPravy1, LOW); // Opačná logika pinů z tvého kódu
    digitalWrite(inMotorPravy2, HIGH);
    analogWrite(pwmMotorPravy, constrain(rychlost, 0, 255));
}

void pravyMotorVzad(int rychlost) {
    digitalWrite(inMotorPravy1, HIGH); // Opačná logika pinů z tvého kódu
    digitalWrite(inMotorPravy2, LOW);
    analogWrite(pwmMotorPravy, constrain(rychlost, 0, 255));
}

void pravyMotorStop() {
    digitalWrite(inMotorPravy1, LOW); // Brzdění zkratem
    digitalWrite(inMotorPravy2, LOW);
    analogWrite(pwmMotorPravy, 0); // Alternativně volný doběh
}

void zastavVse() {
    levyMotorStop();
    pravyMotorStop();
}

// Senzory a pomocné funkce
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

// LED a zvuk
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

void oslavnaMelodie() {
    buzzer.tone(880, 500);
    delay(500);
    buzzer.noTone();
}

// Enkodéry
void pravyEncoderAInt() { pulseCountVpravo++; }

void levyEncoderAInt() { pulseCountVlevo++; }

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
    nastavLED(LED_OFF); // Zhasnout

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
        nastavLEDv2(0, LED_MODRA);
        Serial.println("Nouzové zastavení!");
        zastavVse();
        aktualniStav = STOP;
        return;
    }

    if (aktualniStav == STOP) {
        zastavVse();
        return;
    }

    int sensorStateFull;

    static unsigned long posledniCas = 0;
    unsigned long aktualniCas = millis();

    // Kontrola čáry
    if (aktualniCas - posledniCas >= 10) {
        RGBLineFollower.loop();
        sensorStateFull = filtrujSenzor();
        posledniCas = aktualniCas;
    }

    // Stavový automat
    State predchoziStavAutomatu = aktualniStav;

    Serial.print("Aktualni stav: ");
    Serial.println(aktualniStav);

    switch (aktualniStav) {
    case MAPOVANI_JEDU: {
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

        // Ztráta čáry levá
        if (sensorStateFull == 11) {
            if (debugMode)
                Serial.println("Detekce: Ztrata cary leva!");

            pravyMotorStop();
            levyMotorVpred(rychlostJizdy);
            delay(25);
        }

        // Ztráta čáry prává
        if (sensorStateFull == 13 || sensorStateFull == 12) {
            if (debugMode)
                Serial.println("Detekce: Ztrata cary prava!");

            levyMotorStop();
            pravyMotorVpred(rychlostJizdy);
            delay(25);
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

            break;
        }

        if (detekovanyTypKrizovatky == 2) {
            Serial.println("L-križovatka");
            nastavLEDv2(3, LED_MAGENTA);
            aktualniStav = MAPOVANI_OTACIM_VLEVO;

            break;
        }

        if (detekovanyTypKrizovatky == 3) {
            Serial.println("P-križovatka");
            nastavLEDv2(4, LED_INDIGO);

            if (debugMode)
                Serial.println("Resim krizovatku (T nebo P) -> Pravidlo #2: "
                               "Pokracuji ROVNE.");

            aktualniStav = MAPOVANI_OVERUJI_ROVNE;
            break;
        }

        break;
    }

    case MAPOVANI_OVERUJI_ROVNE: {
        Serial.println("hit MAPOVANI_OVERUJI_ROVNE");

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

        break;
    }

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

        boolean turningback = true; // Proměnná pro otáčení dozadu
        pravyMotorVpred(150);       // Rozjeď pravý motor vzad
        levyMotorVzad(150);         // Krátká pauza pro stabilizaci senzorů
        delay(400); // <<<--- HODNOTA PRO OTOČENÍ O 90° (stejná jako pro pravou
                    // stranu)
        while (turningback ==
               true) // Pokud je potřeba otáčet doleva, použij tuto podmínku
        {
            RGBLineFollower.loop();
            // Levý motor dozadu pro otočení doleva
            delay(10); // Krátká pauza pro plynulé čtení senzoru
            int stav = filtrujSenzor();
            if (stav == 9) { // Pokud vidíme rovnou čáru
                turningback = false;
            }
        }
        aktualniStav = MAPOVANI_JEDU;
        break;
    }

        // --- Další stavy (zatím prázdné nebo zjednodušené) ---
    case MAPOVANI_DOKONCENO:
        // Sem přijde logika po dokončení mapování (např. výpočet cesty,
        // optimalizace)
        if (debugMode)
            Serial.println("Stav: Mapovani dokonceno (NEIMPLEMENTOVANO)");
        zastavVse();
        aktualniStav = STOP; // Prozatím
        break;

    case RYCHLY_PRUJEZD_JEDU:
        if (debugMode)
            Serial.println("Stav: Rychly prujezd - jedu (NEIMPLEMENTOVANO)");
        zastavVse();
        aktualniStav = STOP; // Prozatím
        break;

    case RYCHLY_PRUJEZD_OTOCKA:
        if (debugMode)
            Serial.println("Stav: Rychly prujezd - otocka (NEIMPLEMENTOVANO)");
        zastavVse();
        aktualniStav = STOP; // Prozatím
        break;

    case CIL:
        nastavLED(LED_BILA); // Signalizace cíle
        if (debugMode)
            Serial.println("Stav: CIL!");
        zastavVse();
        oslavnaMelodie();
        aktualniStav = STOP; // Přejdi do konečného stavu
        break;

    case INIT:
        // Tento stav by neměl nastat po startu
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
        // Neznámý stav - bezpečnostní zastavení
        if (debugMode)
            Serial.println("Neznamy stav!");
        nastavLED(LED_CERVENA);
        zastavVse();
        aktualniStav = STOP;
        break;
    }

    // Pokud se stav změnil, zaznamenej čas vstupu do nového stavu
    if (aktualniStav != predchoziStavAutomatu) {
        // Můžeš přidat Serial print pro sledování změn stavu
        if (debugMode) {
            Serial.print("Zmena stavu z ");
            Serial.print(predchoziStavAutomatu);
            Serial.print(" na ");
            Serial.println(aktualniStav);
        }
    }
}
