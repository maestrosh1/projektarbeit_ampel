#include "io.h"
#include <Arduino.h>

const int ledPin = 3;
bool ledState = true;

void setupIO() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
}

void setLED(bool state) {
  ledState = state;
  digitalWrite(ledPin, ledState ? HIGH : LOW);
}

bool isLEDOn() {
  return ledState;
}