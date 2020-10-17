#include "globals.h"
#include "settings.h"

eSPIFFS fileSystem;
String message;

void load_settings()
{

DynamicJsonDocument doc(2048);

ESP_LOGI(TAG,"Loading settings...");   

if (fileSystem.openFromFile("/settings.jsn", doc)) {
      serializeJsonPretty(doc, Serial);
    }
dataBuffer.settings.sleep_time = doc["sleep_time"];
}

void save_settings()
{

ESP_LOGI(TAG,"Saving settings...");   

dataBuffer.settings.sleep_time = 5;

DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);


   serializeJsonPretty(doc, Serial);
  if (fileSystem.saveToFile("/settings.jsn", doc)) {
      ESP_LOGI(TAG,"Successfully wrote data to file");
    }

}
