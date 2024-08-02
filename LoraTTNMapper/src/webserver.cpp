#include "globals.h"

#if (USE_WEBSERVER)

// static const char TAG[] = __FILE__;
static const char TAG[] = "";

#include "webserver.h"
AsyncWebServer server(80);

void setup_webserver()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      Serial.println("Index requested");
      request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/MPU", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Index requested");
      request->send(SPIFFS, "/MPU/index.html", "text/html");
    });

     server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/MPU/style.css", "text/css");
  });

      server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/MPU/script.js", "text/css");
  });

    server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      if (SPIFFS.exists("/LOGS.txt"))
      {
        request->send(SPIFFS, "/LOGS.txt", "text/plain");
      }
      else
      {
        request->send(404, "text/plain", "LOGS not found");
      } });

    server.on("/ionic", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      if (SPIFFS.exists("/index2.html"))
      {
        request->send(SPIFFS, "/index2.html", "text/plain");
      }
      else
      {
        request->send(404, "text/plain", "index2.html not found");
      } });

    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      if (SPIFFS.exists("/settings.jsn"))
      {
        request->send(SPIFFS, "/settings.jsn", "application/json");
      }
      else
      {
        request->send(404, "text/plain", "File not found /settings.jsn");
      } });

    server.begin();
    server.serveStatic("/", SPIFFS, "/");
  }
  else
  {
    ESP_LOGI(TAG, "Webserver: No Wifi connection found");
  }
}

#endif