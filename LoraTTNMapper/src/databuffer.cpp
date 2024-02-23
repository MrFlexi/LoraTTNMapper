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

  JsonDocument doc;
  String JsonStr;
  char s[20];
  doc.clear();

  JsonObject Sensors = doc["Sensors"].to<JsonObject>();
  JsonObject Sensors_MotionSensor = Sensors["MotionSensor"].to<JsonObject>();
  Sensors_MotionSensor["yaw"] = dataBuffer.data.yaw;
  Sensors_MotionSensor["pitch"] = dataBuffer.data.pitch;
  Sensors_MotionSensor["roll"] = dataBuffer.data.roll;

  JsonObject Sensors_sensor2 = Sensors["DistanceSensor"].to<JsonObject>();
  Sensors_sensor2["distance"] = dataBuffer.data.LidarDistanceMM;

//JsonArray tiles = doc["tiles"].to<JsonArray>();
//JsonObject tiles_0 = tiles.add<JsonObject>();
//tiles_0["timestamp"] = "2024-02-23T10:00:00";
//tiles_0["value"] = 123;

  // Dynamic Tiles on OPENUI5 Dashboard
  JsonArray sensors = doc.createNestedArray("tiles");
#if (HAS_PMU)
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
  if (data.bat_charge_current > 0)
  {
    sensors_0["indicator"] = "Up";
    sensors_0["valueColor"] = "Good";
  }
  else
  {
    sensors_0["indicator"] = "Down";
    sensors_0["valueColor"] = "Bad";
  }

  JsonObject sensors_21 = sensors.createNestedObject();
  sensors_21["name"] = "Battery";
  sensors_21["subheader"] = "Coulomb";
  sensors_21["value"] = data.bat_DeltamAh;
  sensors_21["unit"] = "mAh";

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
#endif

#if (USE_SUN_POSITION)
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

#if (HAS_PMU)
  JsonObject sensors_10 = sensors.createNestedObject();
  strftime(s, sizeof(s), "from %H:%M:%S", &dataBuffer.data.mpp_last_timeinfo);
  sensors_10["name"] = "Max Power Point";
  sensors_10["subheader"] = s;
  sensors_10["value"] = dataBuffer.data.mpp_max_bat_charge_power;
  sensors_10["unit"] = "mW";

  // Tabelle
  JsonArray mpp = doc.createNestedArray("mpp");
  for (int i = 0; i < 13; i++)
  {
    JsonObject mpp_line = mpp.createNestedObject();
    mpp_line["pmu_charge_setting"] = dataBuffer.data.mpp_values[i].pmu_charge_setting;
    mpp_line["bus_voltage"] = dataBuffer.data.mpp_values[i].bus_voltage;
    mpp_line["bat_charge_current"] = dataBuffer.data.mpp_values[i].bat_charge_current;
    mpp_line["bat_charge_power"] = dataBuffer.data.mpp_values[i].bat_charge_power;
  }
#endif

#endif

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

#if (ESP_SLEEP)
  JsonObject sensors_9 = sensors.createNestedObject();
  sensors_9["name"] = "Deep sleep";
  sensors_9["subheader"] = "in";
  sensors_9["value"] = dataBuffer.data.MotionCounter;
  sensors_9["unit"] = "Min";
#endif

#if (HAS_INA219)
  JsonObject sensors_11 = sensors.createNestedObject();
  sensors_11["name"] = "Solar Panel";
  sensors_11["subheader"] = dataBuffer.data.ina219[0].voltage;
  sensors_11["value"] = dataBuffer.data.ina219[0].current;
  sensors_11["unit"] = "V/mA";

  JsonObject sensors_12 = sensors.createNestedObject();
  sensors_12["name"] = "Solar Panel";
  sensors_12["subheader"] = "Power";
  sensors_12["value"] = dataBuffer.data.ina219[0].power;
  sensors_12["unit"] = "mW";
#endif

#if (USE_MPU6050)
  JsonObject sensors_13 = sensors.createNestedObject();
  sensors_13["name"] = "Gyro";
  sensors_13["subheader"] = "Yaw";
  sensors_13["value"] = dataBuffer.data.yaw;
  sensors_13["unit"] = "d";
#endif

#if (USE_VL53L1X)
  JsonObject sensors_14 = sensors.createNestedObject();
  sensors_14["name"] = "Lidar Distance";
  sensors_14["subheader"] = "VL53L1X";
  sensors_14["value"] = dataBuffer.data.LidarDistanceMM;
  sensors_14["unit"] = "mm";
#endif

  serializeJson(doc, JsonStr);
  // serializeJson(doc, Serial);s
  return JsonStr;
}

// Json format suitable for direct input to INFLUX DB
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
#if (HAS_PMU)
  measurement["bat_voltage"] = data.bat_voltage;
  measurement["bat_charge_current"] = data.bat_charge_current;
  measurement["bat_fuel_gauge"] = data.bat_DeltamAh;
  measurement["bus_voltage"] = data.bus_voltage;
  measurement["bus_current"] = data.bus_current;
  measurement["bat_max_charge_curr"] = settings.bat_max_charge_current;
#endif

#if (HAS_INA219)
  measurement["panel_voltage"] = data.ina219[0].voltage;
  measurement["panel_current"] = data.ina219[0].current;
  measurement["panel_power"] = data.ina219[0].power;

#endif

  // Device
  measurement["dev_sleep_time"] = settings.sleep_time;
  measurement["dev_boot_counter"] = data.bootCounter;

  // BME280
  measurement["temperature"] = data.temperature;
  measurement["humidity"] = data.humidity;
  measurement["soil_moisture"] = data.soil_moisture;

#if (USE_DISTANCE_SENSOR_HCSR04)
  // HC-SR04 Sonic distance sensor
  measurement["hcsr04_distance"] = data.hcsr04_distance;
#endif

// Camera
#if (USE_CAMERA)
  // ESP_LOGI(TAG, "ImageSize base64: %d", data.image_buffer.length);
  measurement["image_url"] = data.image_url;
  measurement["image"] = String(data.image_buffer);
#endif

#if (USE_MPU6050)
  measurement["yaw"] = data.yaw;
  measurement["pitch"] = data.pitch;
  measurement["roll"] = data.roll;
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