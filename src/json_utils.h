#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <ArduinoJson.h>

bool readJsonFile(const char* path, StaticJsonDocument<256>& doc);
bool writeJsonFile(const char* path, const StaticJsonDocument<256>& doc);

// Erweiterte Funktionen für größere JSON-Dokumente (NFC-Daten)
bool readLargeJsonFile(const char* path, StaticJsonDocument<8192>& doc);
bool writeLargeJsonFile(const char* path, const StaticJsonDocument<8192>& doc);

#endif
