#ifndef HTTP_ANY
#define HTTP_ANY -1
#endif

#ifndef HTTP_GET
#define HTTP_GET 0x01
#endif

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "webserver.h"
#include "io.h"
#include "json_utils.h"
#include "nfc.h"

#define ssid "ESP32_AP"
#define password "1234"

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void someFunction() {
  StaticJsonDocument<256> doc;

  if (readJsonFile("/data.json", doc)) {
    // Werte auslesen
    int verbleibendeZeit = doc["Verbleibende Zeit"];
    const char* ledStatus = doc["LED-Status"];
    float spannung = doc["Batteriespannung"];

    // Werte ändern
    doc["Verbleibende Zeit"] = 10;
    doc["LED-Status"] = "Grün";
    doc["Batteriespannung"] = 3.3;

    // Speichern
    if (writeJsonFile("/data.json", doc)) {
      Serial.println("JSON Datei erfolgreich aktualisiert");
    }
  }
}


void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS-Fehler");
    return;
  }

  setupIO();
  setupNFC(); // NFC-Setup hinzufügen

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(1000);
  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  initWebServer();
}

void loop() {
  loopNFC(); // NFC-Loop hinzufügen
}
