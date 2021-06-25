#include "globals.h"
#include "settings.h"

// eSPIFFS fileSystem;
String message;

const char *filename = "/settings.jsn";

void loadConfiguration()
{

  DynamicJsonDocument doc(2048);

  // Open file for reading
  File file = SPIFFS.open(filename);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }
  else
  {
    ESP_LOGI(TAG, "Loading settings ");
    serializeJsonPretty(doc, Serial);

    const char *value = doc["settings"]["sleep_time"];
    dataBuffer.settings.sleep_time = atoi(doc["settings"]["sleep_time"]);

    if (dataBuffer.settings.sleep_time > 0)
    {
    }
    else
    {
      dataBuffer.settings.sleep_time = TIME_TO_SLEEP;
    }

    
    
    dataBuffer.settings.bat_max_charge_current = doc["settings"]["bat_max_charge_current"];

    if (dataBuffer.settings.bat_max_charge_current > 0)
    {
    }
    else
    {
      dataBuffer.settings.bat_max_charge_current = AXP1XX_CHARGE_CUR_450MA;
    }

    ESP_LOGI(TAG, "Sleeptime: %2d", dataBuffer.settings.sleep_time);
  }
  file.close();
}

void saveConfiguration()
{
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(filename);

  // Open file for writing
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file)
  {
    Serial.println(F("Failed to create file"));
    return;
  }

  DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);
  obj["bat_max_charge_current"] = String(dataBuffer.settings.bat_max_charge_current);

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  else ESP_LOGI(TAG, "Settinges saved....");

  // Close the file
  file.close();
  
}
