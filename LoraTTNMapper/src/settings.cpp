#include "globals.h"
#include "settings.h"

eSPIFFS fileSystem;
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
    dataBuffer.settings.sleep_time = doc["sleep_time"];

    if (dataBuffer.settings.sleep_time > 0)
    {
    }
    else
    {
      dataBuffer.settings.sleep_time = TIME_TO_SLEEP;
    }

    ESP_LOGI(TAG, "Sleeptime: %2d", dataBuffer.settings.sleep_time);
  }
  file.close();
}


void saveConfiguration() {
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(filename);

  // Open file for writing
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}




void load_settings()
{

  DynamicJsonDocument doc(2048);

  ESP_LOGI(TAG, "Loading settings...");

  if (fileSystem.openFromFile("/settings.jsn", doc))
  {
    serializeJsonPretty(doc, Serial);
  }
  dataBuffer.settings.sleep_time = doc["sleep_time"];

  if (dataBuffer.settings.sleep_time > 0)
  {
  }
  else
  {
    dataBuffer.settings.sleep_time = TIME_TO_SLEEP;
  }

  ESP_LOGI(TAG, "Sleeptime: %2d", dataBuffer.settings.sleep_time);
}

void save_settings()
{

  ESP_LOGI(TAG, "Saving settings...");

  DynamicJsonDocument doc(2048);
  JsonObject obj = doc.createNestedObject("settings");

  obj["sleep_time"] = String(dataBuffer.settings.sleep_time);

  serializeJsonPretty(doc, Serial);
  if (fileSystem.saveToFile("/settings.jsn", doc))
  {
    ESP_LOGI(TAG, "Successfully wrote data to file");
  }
}
