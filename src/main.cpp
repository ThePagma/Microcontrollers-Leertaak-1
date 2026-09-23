#include <Arduino.h>

const int KNOP1_PIN = 5;   // start button
const int KNOP2_PIN = 6;   // stop button

int counter = 0;

const int WarningLED = 8;
const int LED1 = 7;
const int LED2 = 2;
const int LED3 = 3;
const int LED4 = 4;

const int LED_PINS[] = {LED1, LED2, LED3, LED4};


int lastKnop1Reading = HIGH;
int knop1State = HIGH;
unsigned long lastDebounceTime = 0;


int lastKnop2Reading = HIGH;
int knop2State = HIGH;
unsigned long lastDebounceTime2 = 0;

const unsigned long DEBOUNCE_DELAY = 25; // ms


const float Distance = 0.6;   // meter
const float minSpeed = 0.2;   // km/uur
const float maxSpeed = 10;    // km/uur

const float snelheidMsMin = minSpeed * 1000.0 / 3600.0;  // minimale snelheid in m/s
const float tijdMax = Distance / snelheidMsMin;          // maximale tijd in s (10.8 s)

const float snelheidMsMax = maxSpeed * 1000.0 / 3600.0;  // maximale snelheid in m/s
const float tijdMin = Distance / snelheidMsMax;          // minimale tijd in s (0.216 s)

const unsigned long tijdMaxMs = (unsigned long)(tijdMax * 1000.0);

// Timer state
bool timerRunning = false;
unsigned long startTime = 0;

void setup() {
    Serial.begin(9600);
    delay(1000);
    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
    pinMode(WarningLED, OUTPUT);
    pinMode(KNOP1_PIN, INPUT_PULLUP);
    pinMode(KNOP2_PIN, INPUT_PULLUP);
}

void Calculate(int count) {
    Serial.println(count);
    for (int i = 3; i >= 0; i--) {
        int bitWaarde = 1 << i;
        if (count & bitWaarde) {
            digitalWrite(LED_PINS[i], HIGH);
        } else {
            digitalWrite(LED_PINS[i], LOW);
        }
    }
    Serial.println();
}

void CalculateSpeed(unsigned long elapsedMs) {
    float tijd = elapsedMs / 1000.0;   // seconden
    float speedKmh;

    if (tijd <= tijdMin) {
        speedKmh = maxSpeed;           // te snel: begrens op 10 km/uur
    } else if (tijd >= tijdMax) {
        digitalWrite(WarningLED, HIGH);
        speedKmh = minSpeed;           // te langzaam: begrens op 0.2 km/uur
    } else {
        speedKmh = (Distance / tijd) * 3.6;   // m/s naar km/uur
    }

    Serial.print("Elapsed time: ");
    Serial.print(elapsedMs);
    Serial.println(" ms");

    Serial.print("Speed: ");
    Serial.print(speedKmh, 2);
    Serial.println(" km/uur");
}

void loop() {
    unsigned long now = millis();
    int reading = digitalRead(KNOP1_PIN);
    int reading2 = digitalRead(KNOP2_PIN);


    if (reading != lastKnop1Reading) {
        lastDebounceTime = now;
    }

    if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != knop1State) {
            knop1State = reading;

            if (knop1State == LOW) {
                digitalWrite(WarningLED, LOW);
                counter++;
                if (counter > 15) counter = 0;
                Calculate(counter);

                if (!timerRunning) {
                    startTime = now;
                    timerRunning = true;
                    Serial.println("Timer started");
                }
            }
        }
    }


    if (timerRunning && (now - startTime) >= tijdMaxMs) {
        timerRunning = false;
        Serial.println("Timeout: too slow");
        CalculateSpeed(tijdMaxMs);
    }

    if (reading2 != lastKnop2Reading) {
        lastDebounceTime2 = now;
    }

    if ((now - lastDebounceTime2) > DEBOUNCE_DELAY) {
        if (reading2 != knop2State) {
            knop2State = reading2;

            if (knop2State == LOW) {
                if (timerRunning) {
                    unsigned long elapsed = now - startTime;
                    timerRunning = false;
                    CalculateSpeed(elapsed);
                } else {
                    Serial.println("Button 2 pressed without start, ignored");
                    digitalWrite(WarningLED, HIGH);
                }
            }
        }
    }

    lastKnop1Reading = reading;
    lastKnop2Reading = reading2;
}