#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void initWebServer();
String getJSONInfo();
String getNFCDetails();

#endif
