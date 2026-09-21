#include <Arduino.h>

const int LAMP_PIN = 2;
const int KNOP_PIN = 3;
void setup() {
    pinMode(LAMP_PIN, OUTPUT);
    pinMode(KNOP_PIN, INPUT_PULLUP);
}

void loop() {
    int knopStatus = digitalRead(KNOP_PIN);

    if (knopStatus == LOW) {
        digitalWrite(LAMP_PIN, HIGH);
    } else {
        digitalWrite(LAMP_PIN, LOW);
    }
}
