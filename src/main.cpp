#include <Arduino.h>
#include <SevSeg.h>

SevSeg sevseg;

const int BUTTON1_PIN = 5;   // sensor 1: voor- en achterwiel
const int BUTTON2_PIN = 6;   // sensor 2: stopt de tijd

int carCount = 0;

// Waar we zijn met de huidige auto:
// 0 = wachten op het voorwiel bij knop 1
// 1 = tijd loopt, wachten op knop 2
// 2 = snelheid bekend, wachten op het achterwiel bij knop 1
// 3 = auto geteld, wachten op het achterwiel bij knop 2
int step = 0;

const int WARNING_LED = 8;
const int LED1 = 7;
const int LED2 = 2;
const int LED3 = 3;
const int LED4 = 4;

const int LED_PINS[] = {LED1, LED2, LED3, LED4};


int lastButton1Reading = HIGH;
int button1State = HIGH;
unsigned long lastDebounceTime1 = 0;


int lastButton2Reading = HIGH;
int button2State = HIGH;
unsigned long lastDebounceTime2 = 0;

const unsigned long DEBOUNCE_DELAY = 25; // ms


const float DISTANCE = 0.6;    // meter tussen de twee sensoren
const float MIN_SPEED = 0.2;   // km/u
const float MAX_SPEED = 10;    // km/u

const float minSpeedMs = MIN_SPEED * 1000.0 / 3600.0;
const float maxTime = DISTANCE / minSpeedMs;          // 10.8 s

const float maxSpeedMs = MAX_SPEED * 1000.0 / 3600.0;
const float minTime = DISTANCE / maxSpeedMs;          // 0.216 s

const unsigned long maxTimeMs = (unsigned long)(maxTime * 1000.0);

unsigned long startTime = 0;

void setup() {
    Serial.begin(9600);
    delay(1000);
    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
    pinMode(WARNING_LED, OUTPUT);
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);

    byte digitPins[] = {A2, A3, A4};
    byte segmentPins[] = {9, 10, 11, 12, 13, A0, A1, A5};
    sevseg.begin(COMMON_CATHODE, 3, digitPins, segmentPins, true, false, true, false);
    sevseg.setBrightness(90);
    sevseg.blank();
}

// Zet het aantal auto's binair op de vier leds
void showCount(int count) {
    Serial.println(count);
    for (int i = 3; i >= 0; i--) {
        int bitValue = 1 << i;
        if (count & bitValue) {
            digitalWrite(LED_PINS[i], HIGH);
        } else {
            digitalWrite(LED_PINS[i], LOW);
        }
    }
    Serial.println();
}

// waarschuwing lampje aan, scherm leeg en opnieuw beginnen.
void triggerWarning() {
    digitalWrite(WARNING_LED, HIGH);
    sevseg.blank();
    step = 0;
    Serial.println("Warning: reset");
}

void calculateSpeed(unsigned long elapsedMs) {
    float time = elapsedMs / 1000.0;
    float speedKmh;

    if (time <= minTime) {
        speedKmh = MAX_SPEED;                  // te snel, we laten 10 km/u zien
    } else if (time >= maxTime) {
        digitalWrite(WARNING_LED, HIGH);       // te langzaam
        speedKmh = MIN_SPEED;
    } else {
        speedKmh = (DISTANCE / time) * 3.6;    // m/s omrekenen naar km/u
    }

    Serial.print("Elapsed time: ");
    Serial.print(elapsedMs);
    Serial.println(" ms");

    Serial.print("Speed: ");
    Serial.print(speedKmh, 2);
    Serial.println(" km/h");

    // Keer 10 en afronden, zodat er één cijfer achter de punt komt
    sevseg.setNumber((int)(speedKmh * 10 + 0.5), 1);
}

void loop() {
    unsigned long now = millis();
    int reading1 = digitalRead(BUTTON1_PIN);
    int reading2 = digitalRead(BUTTON2_PIN);

    // Knop 1
    if (reading1 != lastButton1Reading) {
        lastDebounceTime1 = now;
    }

    if ((now - lastDebounceTime1) > DEBOUNCE_DELAY) {
        if (reading1 != button1State) {
            button1State = reading1;

            if (button1State == LOW) {
                if (step == 0) {
                    // Voorwiel: tijd gaat lopen
                    digitalWrite(WARNING_LED, LOW);
                    startTime = now;
                    step = 1;
                    Serial.println("Front wheel: timer started");
                    // leeg display
                    sevseg.setNumber((int)(000), 1);
                } else if (step == 2) {
                    carCount++;
                    if (carCount > 15) carCount = 0;
                    Serial.print("Rear wheel: car count ");
                    showCount(carCount);
                    step = 3;
                }
            }
        }
    }

    // Duurt het te lang, dan laten we de laagste snelheid zien.
    // auto telt niet mee
    if (step == 1 && (now - startTime) >= maxTimeMs) {
        Serial.println("Timeout: too slow");
        calculateSpeed(maxTimeMs);
        triggerWarning();
    }

    // Knop 2
    if (reading2 != lastButton2Reading) {
        lastDebounceTime2 = now;
    }

    if ((now - lastDebounceTime2) > DEBOUNCE_DELAY) {
        if (reading2 != button2State) {
            button2State = reading2;

            if (button2State == LOW) {
                if (step == 1) {
                    // Tijd stoppen en snelheid uitrekenen
                    calculateSpeed(now - startTime);
                    step = 2;
                } else if (step == 3) {
                    // Achterwiel over knop 2: auto is voorbij, opnieuw beginnen
                    Serial.println("Rear wheel passed sensor 2: ready");
                    step = 0;
                } else {
                    // Knop 2 op het verkeerde moment
                    triggerWarning();
                }
            }
        }
    }

    lastButton1Reading = reading1;
    lastButton2Reading = reading2;
    sevseg.refreshDisplay();
}