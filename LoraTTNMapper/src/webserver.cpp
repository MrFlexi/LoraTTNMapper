#include "globals.h"

#if (USE_WEBSERVER)

#include "webserver.h"
AsyncWebServer server(80);



void setup_webserver()
{
    if (WiFi.status() == WL_CONNECTED)
  {
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Index requested");
      request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/LOGS.txt")) {
      request->send(SPIFFS, "/LOGS.txt", "text/plain");
    } else {
      request->send(404, "text/plain", "LOGS not found");
    }
    });


    server.on("/ionic", HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/index2.html")) {
      request->send(SPIFFS, "/index2.html", "text/plain");
    } else {
      request->send(404, "text/plain", "index2.html not found");
    }
    });


    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/settings.jsn")) {
      request->send(SPIFFS, "/settings.jsn", "application/json");
    } else {
      request->send(404, "text/plain", "File not found /settings.jsn");
    }
    });

    server.begin();
    server.serveStatic("/", SPIFFS, "/");
  }
}

#endif