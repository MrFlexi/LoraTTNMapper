#include "globals.h"
#include "settings.h"

eSPIFFS fileSystem;
String message;

void load_settings()
{

DynamicJsonDocument doc(2048);

Serial.println();
Serial.println("Loading settings...");   

if (fileSystem.openFromFile("/settings.jsn", doc)) {
        Serial.println("Successfully read file and parsed data: ");
        serializeJsonPretty(doc, Serial);
    }

  dataBuffer.settings.sleep_time = doc["sleep_time"];
 Serial.println(dataBuffer.settings.sleep_time);  

}

void save_settings()
{

Serial.println();
 Serial.println("Saving settings...");   

dataBuffer.settings.sleep_time = 5;

DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);


   serializeJsonPretty(doc, Serial);
  if (fileSystem.saveToFile("/settings.jsn", doc)) {
        Serial.println("Successfully wrote data to file");
    }

}
