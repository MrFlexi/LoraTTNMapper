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


if( dataBuffer.settings.sleep_time > 0 )
  {
  }
  else
  {
    dataBuffer.settings.sleep_time = TIME_TO_SLEEP;
   
  }

ESP_LOGI(TAG,"Sleeptime: %2d", dataBuffer.settings.sleep_time);


}

void save_settings()
{

ESP_LOGI(TAG,"Saving settings...");   


DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);


  serializeJsonPretty(doc, Serial);
  if (fileSystem.saveToFile("/settings.jsn", doc)) {
      ESP_LOGI(TAG,"Successfully wrote data to file");
    }

}
