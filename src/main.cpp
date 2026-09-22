#include <Arduino.h>

const int KNOP1_PIN = 5;
const int KNOP2_PIN = 6; // reserved, not used yet

int counter = 0;

const int LED1 = 7;
const int LED2 = 2;
const int LED3 = 3;
const int LED4 = 4;

const int LED_PINS[] = {LED1, LED2, LED3, LED4};

// Debounce state
int lastKnop1Reading = HIGH;   // last raw reading
int knop1State = HIGH;         // debounced state
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 25; // ms

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
    int reading = digitalRead(KNOP1_PIN);

    if (reading != lastKnop1Reading) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != knop1State) {
            knop1State = reading;

            if (knop1State == LOW) {
                counter++;
                if (counter > 15) counter = 0;
                Calculate(counter);
            }
        }
    }

    lastKnop1Reading = reading;
}
