#include "globals.h"
#include "databuffer.h"


DataBuffer::DataBuffer()
{
}

void DataBuffer::set(deviceStatus_t input)
{
  data = input;
}

void DataBuffer::get()
{
}

String DataBuffer::to_json()
{
  const int capacity = JSON_OBJECT_SIZE(100) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  String JsonStr;

    doc.clear();

    JsonObject tags = doc.createNestedObject("tags");
    tags["device"] = DEVICE_NAME;
    tags["ip"] = String(data.ip_address);

    JsonObject measurement = doc.createNestedObject("measurement");

    // Battery Management
    measurement["bat_voltage"] =        data.bat_voltage;
    measurement["bat_charge_current"] = data.bat_charge_current;
    measurement["bat_voltage"] =        data.bat_voltage;
    measurement["bat_discharge_current"] = data.bat_discharge_current;
    measurement["bat_fuel_gauge"] =     data.bat_DeltamAh;
    measurement["bus_voltage"] =        data.bus_voltage;
    measurement["bus_current"] =        data.bus_current;

    #if (HAS_INA219)
    measurement["panel_voltage"] =      data.panel_voltage;
    measurement["panel_current"] =      data.panel_current;
    #endif

    // Device
    measurement["sleep_time"] = settings.sleep_time;
    measurement["bat_max_charge_curr"] = settings.bat_max_charge_current;
    measurement["BootCounter"] = data.bootCounter;

    // BME280
    measurement["temperature"] = data.temperature;
    measurement["humidity"] = data.humidity;
    measurement["soil_moisture"] = data.soil_moisture;
   

    // HC-SR04 Sonic distance sensor
    measurement["HCSR04_Distance"] = data.distance;


    // Camera
    #if (USE_CAMERA)
    //ESP_LOGI(TAG, "ImageSize base64: %d", data.image_buffer.length);
    measurement["image_url"] = data.image_url;
    measurement["image"] = String(data.image_buffer);
    
    #endif

    // Add the "location"
    JsonObject location = doc.createNestedObject("location");
    location["lat"] = data.gps.lat();
    location["lon"] = data.gps.lng();
  
    serializeJson(doc, JsonStr);
    _error = "Jogi";
    return JsonStr;
}

DataBuffer dataBuffer;
deviceStatus_t sensorValues;