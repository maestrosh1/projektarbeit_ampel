#include "nfc.h"
#include "json_utils.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPIFFS.h>

// Pins für I2C und IRQ
#define I2C_SDA 6
#define I2C_SCL 7
#define PN532_IRQ 8
#define PN532_RESET -1

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

volatile bool interrupt = false;
bool nfcAvailable = false; // Flag für NFC-Verfügbarkeit
uint8_t KEY_A[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void IRAM_ATTR irqHandler() { interrupt = true; }

void setupNFC()
{
  Serial.println("  - Starte I2C-Verbindung...");
  Wire.begin(I2C_SDA, I2C_SCL, 400000); // I2C @ 400kHz
  Serial.println("  - I2C gestartet");
  
  Serial.println("  - Initialisiere NFC-Reader...");
  nfc.begin();
  Serial.println("  - NFC begin() abgeschlossen");
  
  Serial.println("  - Konfiguriere SAM...");
  nfc.SAMConfig();
  Serial.println("  - SAM konfiguriert");
  
  Serial.println("  - Prüfe Firmware-Version...");
  if(!nfc.getFirmwareVersion()){
    Serial.println("NFC-Reader nicht gefunden! Hotspot läuft ohne NFC.");
    nfcAvailable = false; // NFC nicht verfügbar
    return;
  } else {
    Serial.println("NFC-Reader initialisiert");
    nfcAvailable = true; // NFC verfügbar
  }
  

  pinMode(PN532_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PN532_IRQ), irqHandler, FALLING);

  Serial.println("Waiting for NFC tags...");
}

void loopNFC()
{
  // Nur ausführen, wenn NFC verfügbar ist
  if (!nfcAvailable) {
    return;
  }
  
  if (nfc.inListPassiveTarget()) {
    dumpTag();
  }
}

void dumpTag()
{
  uint8_t uid[7], uidLen;
  if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 0))
    return;

  // Bestehende config.json lesen
  StaticJsonDocument<8192> doc;
  const char* configPath = "/config.json";
  
  if (!readLargeJsonFile(configPath, doc)) {
    Serial.println("Konnte config.json nicht lesen, erstelle neue Struktur");
    // Falls die Datei nicht existiert oder fehlerhaft ist, erstelle leeres JSON-Objekt
    doc.clear();
  }

  // UID als Hex-String erstellen
  String uidStr = "";
  for (uint8_t i = 0; i < uidLen; i++) {
    if (uid[i] < 0x10) uidStr += '0';
    uidStr += String(uid[i], HEX);
  }
  
  // NFC-Daten zum bestehenden JSON hinzufügen
  doc["last_nfc_uid"] = uidStr;
  doc["last_nfc_timestamp"] = millis(); // Zeitstempel hinzufügen

  JsonArray blocks = doc.createNestedArray("nfc_blocks");

  uint8_t data[16];
  for (uint8_t block = 0; block < 64; block++) {
    if (block % 4 == 0) {
      if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLen, block, 0, KEY_A)) {
        Serial.printf("Auth-Error Sector %u\n", block / 4);
        return;
      }
    }
    if (nfc.mifareclassic_ReadDataBlock(block, data)) {
      JsonArray arr = blocks.createNestedArray();
      for (uint8_t i = 0; i < 16; i++) {
        arr.add(data[i]);
      }
    } else {
      Serial.printf("Reading-Error Block %u\n", block);
      return;
    }
  }

  // JSON-Datei speichern
  if (writeLargeJsonFile(configPath, doc)) {
    Serial.println("NFC-Daten erfolgreich in config.json gespeichert");
    Serial.println("UID: " + uidStr);
  } else {
    Serial.println("Fehler beim Speichern der NFC-Daten");
  }
}
