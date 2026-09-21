#include <Arduino.h>

const int KNOP1_PIN = 5;
const int KNOP2_PIN = 6;

const int LED1 = 2;
const int LED2 = 3;
const int LED3 = 4;
const int LED4 = 7;

const int LED_PINS[] = {LED1, LED2, LED3, LED4};

void setup() {
    Serial.begin(9600);
    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
    pinMode(KNOP1_PIN, INPUT_PULLUP);
    pinMode(KNOP2_PIN, INPUT_PULLUP);
}

void loop() {
    int knopStatus = digitalRead(KNOP1_PIN);

    if (knopStatus == LOW) {
        digitalWrite(LED_PINS[0], HIGH);
    } else {
        digitalWrite(LED_PINS[0], LOW);
    }
}