#include <Arduino.h>
#include <SevSeg.h>

SevSeg sevseg;
const int KNOP1_PIN = 5;   // start button
const int KNOP2_PIN = 6;   // stop button

int Counter = 0;
int AssCount = 0;

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

const float snelheidMsMin = minSpeed * 1000.0 / 3600.0;
const float tijdMax = Distance / snelheidMsMin;          // 10.8 s

const float snelheidMsMax = maxSpeed * 1000.0 / 3600.0;
const float tijdMin = Distance / snelheidMsMax;          // 0.216 s

const unsigned long tijdMaxMs = (unsigned long)(tijdMax * 1000.0);

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

    byte digitPins[] = {A2, A3, A4};
    byte segmentPins[] = {9, 10, 11, 12, 13, A0, A1, A5};
    sevseg.begin(COMMON_CATHODE, 3, digitPins, segmentPins, true, false, true, false);
    sevseg.setBrightness(90);
}

void Count(int count) {
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
    float tijd = elapsedMs / 1000.0;
    float speedKmh;

    if (tijd <= tijdMin) {
        speedKmh = maxSpeed;
    } else if (tijd >= tijdMax) {
        digitalWrite(WarningLED, HIGH);
        speedKmh = minSpeed;
    } else {
        speedKmh = (Distance / tijd) * 3.6;   // m/s naar km/uur
    }

    Serial.print("Elapsed time: ");
    Serial.print(elapsedMs);
    Serial.println(" ms");

    Serial.print("Speed: ");
    Serial.print(speedKmh, 2);
    Serial.println(" km/uur");

    sevseg.setNumber((int)(speedKmh * 10 + 0.5), 1);
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
                AssCount++;
                if (AssCount == 2) {
                    AssCount = 0;
                    digitalWrite(WarningLED, LOW);
                    Counter++;
                    if (Counter > 15) Counter = 0;
                    Count(Counter);

                    if (!timerRunning) {
                        startTime = now;
                        timerRunning = true;
                        Serial.println("Timer started");
                    }
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
                if (timerRunning & AssCount == 2) {
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
    sevseg.refreshDisplay();
}