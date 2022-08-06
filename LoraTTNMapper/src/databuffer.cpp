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

String DataBuffer::to_json_web()
{
  const int capacity = JSON_OBJECT_SIZE(100) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  String JsonStr;
  char s[20];

  doc.clear();
  JsonArray sensors = doc.createNestedArray("sensors");

  JsonObject sensors_1 = sensors.createNestedObject();
  sensors_1["name"] = "Battery";
  sensors_1["subheader"] = "Voltage";
  sensors_1["value"] = data.bat_voltage;
  sensors_1["unit"] = "V";

  JsonObject sensors_0 = sensors.createNestedObject();
  sensors_0["name"] = "Battery";
  sensors_0["subheader"] = "Charge current";
  sensors_0["value"] = data.bat_charge_current;
  sensors_0["unit"] = "mA";
  sensors_0["indicator"] = "Up";
  sensors_0["valueColor"] = "Good";

  JsonObject sensors_2 = sensors.createNestedObject();
  sensors_2["name"] = "Bus";
  sensors_2["subheader"] = "Voltage";
  sensors_2["value"] = data.bus_voltage;
  sensors_2["unit"] = "V";

  JsonObject sensors_3 = sensors.createNestedObject();
  sensors_3["name"] = "Bus";
  sensors_3["subheader"] = "Current";
  sensors_3["value"] = data.bus_current;
  sensors_3["unit"] = "mA";
  sensors_3["indicator"] = "Up";
  sensors_3["valueColor"] = "Good";


  sprintf(s, "%04.2g", data.sun_elevation);
  JsonObject sensors_4 = sensors.createNestedObject();
  sensors_4["name"] = "Sun position";
  sensors_4["subheader"] = "elevation";
  sensors_4["value"] = s;
  sensors_4["unit"] = "degree";

  sprintf(s, "%04.2g", data.sun_azimuth);
  JsonObject sensors_5 = sensors.createNestedObject();
  sensors_5["name"] = "Sun position";
  sensors_5["subheader"] = "azimuth";
  sensors_5["value"] = s;
  sensors_5["unit"] = "degree";

#if (USE_PWM_SERVO)
  JsonObject sensors_6 = sensors.createNestedObject();
  sensors_6["name"] = "Servo 1";
  sensors_6["subheader"] = "position";
  sensors_6["value"] = data.servo1;
  sensors_6["unit"] = "degree"; 

 JsonObject sensors_7 = sensors.createNestedObject();
  sensors_7["name"] = "Servo 2";
  sensors_7["subheader"] = "position";
  sensors_7["value"] = data.servo2;
  sensors_7["unit"] = "degree";
#endif

JsonObject sensors_8 = sensors.createNestedObject();
  strftime(s, sizeof(s), "%H:%M:%S", &dataBuffer.data.timeinfo);
  sensors_8["name"] = "Time";
  sensors_8["subheader"] = s;
  sensors_8["value"] = s;
  sensors_8["unit"] = "";
  Serial.println("make it json for websocket");
  Serial.println(&dataBuffer.data.timeinfo, "%A, %B %d %Y %H:%M:%S");

  serializeJson(doc, JsonStr);
  return JsonStr;
}

String DataBuffer::to_json()
{
  // Json format suitable for direct input to INFLUX DB

  const int capacity = JSON_OBJECT_SIZE(100) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  String JsonStr;

  doc.clear();

  JsonObject tags = doc.createNestedObject("tags");
  tags["device"] = DEVICE_NAME;
  tags["ip"] = String(data.ip_address);

  JsonObject measurement = doc.createNestedObject("measurement");

  // Battery Management
  measurement["bat_voltage"] = data.bat_voltage;
  measurement["bat_charge_current"] = data.bat_charge_current;    
  measurement["bat_fuel_gauge"] = data.bat_DeltamAh;
  measurement["bus_voltage"] = data.bus_voltage;
  measurement["bus_current"] = data.bus_current;

#if (HAS_INA219)
  measurement["panel_voltage"] = data.panel_voltage;
  measurement["panel_current"] = data.panel_current;
#endif

  // Device
  measurement["sleep_time"] = settings.sleep_time;
  measurement["bat_max_charge_curr"] = settings.bat_max_charge_current;
  measurement["BootCounter"] = data.bootCounter;

  // BME280
  measurement["temperature"] = data.temperature;
  measurement["humidity"] = data.humidity;
  measurement["soil_moisture"] = data.soil_moisture;

#if (USE_DISTANCE_SENSOR_HCSR04)
  // HC-SR04 Sonic distance sensor
  measurement["HCSR04_Distance"] = data.distance;
#endif

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