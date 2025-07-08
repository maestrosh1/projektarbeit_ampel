#include "json_utils.h"
#include <SPIFFS.h>
#include <Arduino.h>

bool readJsonFile(const char* path, StaticJsonDocument<256>& doc) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Datei nicht gefunden: " + String(path));
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("deserializeJson fehlgeschlagen: ");
    Serial.println(error.f_str());
    return false;
  }
  return true;
}

bool writeJsonFile(const char* path, const StaticJsonDocument<256>& doc) {
  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Kann Datei nicht öffnen zum Schreiben: " + String(path));
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Fehler beim Schreiben der Datei");
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool readLargeJsonFile(const char* path, StaticJsonDocument<8192>& doc) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Datei nicht gefunden: " + String(path));
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("deserializeJson fehlgeschlagen: ");
    Serial.println(error.f_str());
    return false;
  }
  return true;
}

bool writeLargeJsonFile(const char* path, const StaticJsonDocument<8192>& doc) {
  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Kann Datei nicht öffnen zum Schreiben: " + String(path));
    return false;
  }

  if (serializeJsonPretty(doc, file) == 0) {
    Serial.println("Fehler beim Schreiben der Datei");
    file.close();
    return false;
  }
  file.close();
  return true;
}
