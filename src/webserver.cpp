#ifndef HTTP_GET
#define HTTP_GET 0x01
#endif

#ifndef HTTP_ANY
#define HTTP_ANY -1
#endif

#include "webserver.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "io.h"
#include "json_utils.h"

AsyncWebServer server(80);

String getJSONInfo() {
  StaticJsonDocument<8192> doc;
  if (!readLargeJsonFile("/config.json", doc)) {
    Serial.println("Webserver: Fehler beim Lesen der config.json");
    return "<p><strong>Fehler:</strong> JSON-Datei nicht gefunden.</p>";
  }

  Serial.println("Webserver: config.json erfolgreich gelesen");
  
  // Debug: Alle Schlüssel ausgeben
  for (JsonPair pair : doc.as<JsonObject>()) {
    Serial.println("Schlüssel gefunden: " + String(pair.key().c_str()));
  }

  String info = "<p>Verbleibende Zeit: " + String((int)doc["time"]) + "s</p>" +
                "<p>LED Farbe: " + String((const char*)doc["led_color"]) + "</p>" +
                "<p>Batteriespannung: " + String((double)doc["battery_voltage"]) + "V</p>";
  
  // NFC-Informationen hinzufügen, falls vorhanden
  if (doc.containsKey("last_nfc_uid")) {
    Serial.println("Webserver: NFC-Daten gefunden!");
    info += "<hr><h3>NFC-Daten</h3>";
    info += "<p><strong>Letzte UID:</strong> " + String((const char*)doc["last_nfc_uid"]) + "</p>";
    
    if (doc.containsKey("last_nfc_timestamp")) {
      unsigned long timestamp = doc["last_nfc_timestamp"];
      unsigned long currentTime = millis();
      unsigned long timeDiff = (currentTime - timestamp) / 1000; // in Sekunden
      info += "<p><strong>Gescannt vor:</strong> " + String(timeDiff) + " Sekunden</p>";
    }
    
    info += "<p><a href=\"/nfc-details\" class=\"button\">NFC-Details anzeigen</a></p>";
  } else {
    Serial.println("Webserver: Keine NFC-Daten gefunden");
    info += "<hr><p><em>Noch keine NFC-Karte gescannt</em></p>";
  }
  
  return info;
}

String getHTML() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) return "<html><body><h1>Datei nicht gefunden</h1></body></html>";
  String html = file.readString();
  file.close();

  html.replace("{{STATUS}}", isLEDOn() ? "AN" : "AUS");
  html.replace("{{BUTTON}}", isLEDOn()
    ? "<a class=\"button\" href=\"/ledoff\">Ausschalten</a>"
    : "<a class=\"button\" href=\"/ledon\">Einschalten</a>");
  html.replace("{{INFO}}", getJSONInfo());
  return html;
}

String getNFCDetails() {
  StaticJsonDocument<8192> doc;
  if (!readLargeJsonFile("/config.json", doc)) {
    return "<html><body><h1>Fehler beim Laden der NFC-Daten</h1><a href=\"/\">Zurück</a></body></html>";
  }

  String html = "<!DOCTYPE html><html><head><link rel=\"stylesheet\" href=\"style.css\"><meta charset=\"utf-8\"></head><body>";
  html += "<h1>NFC-Details</h1>";
  html += "<a href=\"/\" class=\"button\">← Zurück zur Hauptseite</a><br><br>";
  
  if (doc.containsKey("last_nfc_uid")) {
    html += "<h2>UID: " + String((const char*)doc["last_nfc_uid"]) + "</h2>";
    
    if (doc.containsKey("last_nfc_timestamp")) {
      unsigned long timestamp = doc["last_nfc_timestamp"];
      unsigned long currentTime = millis();
      unsigned long timeDiff = (currentTime - timestamp) / 1000;
      html += "<p><strong>Gescannt vor:</strong> " + String(timeDiff) + " Sekunden</p>";
    }
    
    if (doc.containsKey("nfc_blocks")) {
      JsonArray blocks = doc["nfc_blocks"];
      html += "<h3>NFC-Blöcke (" + String(blocks.size()) + " Blöcke)</h3>";
      html += "<div style=\"font-family: monospace; font-size: 12px;\">";
      
      for (size_t blockNum = 0; blockNum < blocks.size(); blockNum++) {
        JsonArray block = blocks[blockNum];
        html += "<p><strong>Block " + String(blockNum) + ":</strong> ";
        
        for (size_t i = 0; i < block.size(); i++) {
          uint8_t value = block[i];
          if (value < 0x10) html += "0";
          html += String(value, HEX) + " ";
          if ((i + 1) % 4 == 0) html += " "; // Gruppierung alle 4 Bytes
        }
        html += "</p>";
      }
      html += "</div>";
    }
  } else {
    html += "<p>Keine NFC-Daten verfügbar.</p>";
  }
  
  html += "</body></html>";
  return html;
}

void initWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getHTML());
  });

  server.on("/ledon", HTTP_GET, [](AsyncWebServerRequest *request){
    setLED(true);
    request->send(200, "text/html", getHTML());
  });

  server.on("/ledoff", HTTP_GET, [](AsyncWebServerRequest *request){
    setLED(false);
    request->send(200, "text/html", getHTML());
  });

  server.on("/nfc-details", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getNFCDetails());
  });

  server.serveStatic("/style.css", SPIFFS, "/style.css");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Nicht gefunden");
  });

  server.begin();
}
