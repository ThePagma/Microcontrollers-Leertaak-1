#include <Arduino.h>

const int KNOP1_PIN = 5;
const int KNOP2_PIN = 6;

int counter = 0;

const int LED1 = 7;
const int LED2 = 2;
const int LED3 = 3;
const int LED4 = 4;

const int LED_PINS[] = {LED1, LED2, LED3, LED4};

void setup() {
    Serial.begin(9600);
    delay(1000);
    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
    pinMode(KNOP1_PIN, INPUT_PULLUP);
    pinMode(KNOP2_PIN, INPUT_PULLUP);
}

void Calculate(int count) {
    Serial.println(count, BIN);

    for (int i = 3; i >= 0; i--) {
        int bitWaarde = 1 << i;
        if (count & bitWaarde) {
            digitalWrite(LED_PINS[i], HIGH);
            Serial.print("1");
        } else {
            digitalWrite(LED_PINS[i], LOW);
            Serial.print("0");
        }
    }
    Serial.println();
}

void loop() {
    int knopStatus = digitalRead(KNOP1_PIN);

    if (knopStatus == LOW) {
        counter++;
        Calculate(counter);
        if (counter == 15) counter = 0;
    }
}