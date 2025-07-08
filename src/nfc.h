#ifndef NFC_H
#define NFC_H

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <ArduinoJson.h>
#include <Wire.h>

extern bool nfcAvailable; // Flag für NFC-Verfügbarkeit

void setupNFC();
void loopNFC();
void dumpTag();

#endif
