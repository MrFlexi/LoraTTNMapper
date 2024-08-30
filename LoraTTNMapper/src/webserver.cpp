#include "globals.h"

#if (USE_WEBSERVER)

// static const char TAG[] = __FILE__;
static const char TAG[] = "";

#include "webserver.h"
AsyncWebServer server(80);


// Function to handle multi-file download
void handleMultiDownload(AsyncWebServerRequest *request) {
  if (request->args() == 0) {
    request->send(400, "text/plain", "No files selected for download");
    return;
  }

  String boundary = "----ESP32Boundary";
  AsyncWebServerResponse *response = request->beginChunkedResponse("multipart/mixed; boundary=" + boundary,
    [request, boundary](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      static int fileIndex = 0;
      static File file;
      static String fileName;
      static size_t fileOffset = 0;

      if (fileIndex >= request->args()) {
        return 0;
      }

      if (index == 0) {
        fileName = request->arg(fileIndex);
        fileName = "/" + fileName;
        Serial.println(fileName);
        file = SPIFFS.open(fileName, "r");
        if (!file) {
          fileIndex++;
          Serial.print("File not found");Serial.println(fileName);
          return 0;
        }

        String header = "--" + boundary + "\r\n";
        header += "Content-Type: application/octet-stream\r\n";
        header += "Content-Disposition: attachment; filename=\"" + fileName + "\"\r\n\r\n";
        memcpy(buffer, header.c_str(), header.length());
        return header.length();
      }

      if (file && file.available()) {
        size_t len = file.read(buffer, maxLen);
        fileOffset += len;
        if (!file.available()) {
          file.close();
          fileIndex++;
        }
        return len;
      }

      if (fileIndex < request->args()) {
        String footer = "\r\n";
        memcpy(buffer, footer.c_str(), footer.length());
        fileIndex++;
        return footer.length();
      }

      return 0;
    }
  );

  request->send(response);
}

void setup_webserver()
{
  if (WiFi.status() == WL_CONNECTED)
  {    
    Serial.println();
    Serial.println("----------------- Setup Webserver-------------------------");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Index requested");

      if (SPIFFS.exists("/index.html"))
      {        
        request->send(SPIFFS, "/index.html", "text/html");
      }
      else
      {
        request->send(404, "text/plain", "File not found");
        Serial.println("File not found");
      } });


    server.on("/download", HTTP_POST, [](AsyncWebServerRequest *request){
      handleMultiDownload(request);
    });  

    server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
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

            

      server.on("/maclist_old", HTTP_GET, [](AsyncWebServerRequest *request){      
      String filename = get_wificounter_filename();
            
      Serial.print("Webserver request:");Serial.println(filename);
      if (SPIFFS.exists(filename))
      { 
        request->send(SPIFFS, filename, "application/json");
      }
      else
      {
        request->send(404, "text/plain", "File not found ");
      } 
      });

  	  server.on("/maclist", HTTP_GET, [](AsyncWebServerRequest *request){                  
      Serial.print("Webserver request:");
      String output = wificounter_generateHTMLFileList();
      request->send(200, "text/html", output);
      
      });
        

    server.begin();
    server.serveStatic("/", SPIFFS, "/");
  }
  else
  {
    ESP_LOGI(TAG, "Webserver: No Wifi connection found");
  }
}

#endif