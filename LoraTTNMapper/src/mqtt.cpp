#include "globals.h"
#include "mqtt.h"
#include "time.h"

#if (USE_MQTT)
PubSubClient MqttClient(wifiClient);

//const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_server = "85.209.49.65"; // Netcup
const char *mqtt_topic = "mrflexi/device/";
const char *mqtt_topic_mosi = "/mosi";
const char *mqtt_topic_miso = "/miso";
const char *mqtt_topic_irq = "/miso/irq";
const char *mqtt_topic_traincontroll = "/TrainControll/";

long lastMsgAlive = 0;
long lastMsgDist = 0;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  dataBuffer.data.timeinfo = timeinfo;
  dataBuffer.data.timeinfo.tm_year = dataBuffer.data.timeinfo.tm_year + 1900;
  dataBuffer.data.timeinfo.tm_mon = dataBuffer.data.timeinfo.tm_mon + 1;
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.print("Day of week: ");
  //Serial.println(&timeinfo, "%A");
  //Serial.print("Month: ");
  //Serial.println(&timeinfo, "%B");
  //Serial.print("Day of Month: ");
  //Serial.println(&timeinfo, "%d");
  //Serial.print("Year: ");
  //Serial.println(&timeinfo, "%Y");
  //Serial.print("Hour: ");
  //Serial.println(&timeinfo, "%H");
  //Serial.print("Hour (12 hour format): ");
  //Serial.println(&timeinfo, "%I");
  //Serial.print("Minute: ");
  //Serial.println(&timeinfo, "%M");
  //Serial.print("Second: ");
  //Serial.println(&timeinfo, "%S");

  //Serial.println("Time variables");
  //char timeHour[3];
  //strftime(timeHour, 3, "%H", &timeinfo);
  //Serial.println(timeHour);
  //char timeWeekDay[10];
  //strftime(timeWeekDay, 10, "%A", &timeinfo);
  //Serial.println(timeWeekDay);
  //Serial.println();


//--------------------------------------------------------------------------
// Sun Elevation Calculation
//--------------------------------------------------------------------------
Helios helios;

double dAzimuth;
double dElevation;

//----------------------------------------
  // Calc Sun Position München
  //----------------------------------------
  Serial.println();
  Serial.println();
  Serial.println("Sun Azimuth and Elevation Munich");
  
  helios.calcSunPos(2022, dataBuffer.data.timeinfo.tm_mon, dataBuffer.data.timeinfo.tm_mday, dataBuffer.data.timeinfo.tm_hour - 2, dataBuffer.data.timeinfo.tm_min, 00.00, 11.57754, 48.13641);
  helios.calcSunPos(2022, dataBuffer.data.timeinfo.tm_mon, dataBuffer.data.timeinfo.tm_mday, 12, dataBuffer.data.timeinfo.tm_min, 00.00, 11.57754, 48.13641);
  Serial.printf("Azimuth: %f3\n", helios.dAzimuth);
  Serial.printf("Elevation: %f3\n", helios.dElevation);

  dataBuffer.data.sun_azimuth = helios.dAzimuth;
  dataBuffer.data.sun_elevation = helios.dElevation;
}


void doConcat(const char *a, const char *b, const char *c, char *out)
{
  strcpy(out, a);
  strcat(out, b);
  strcat(out, c);
}

void mqtt_loop()
{

  // MQTT Connection
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!MqttClient.connected())
    {
      ESP_LOGE(TAG, "MQTT Client not connected ");
      reconnect();
    }
    MqttClient.loop();
  }
  else
  {
    //ESP_LOGE(TAG, "Wifi not connected ");
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{

  // char json[] = "{\"command\": {\"action\":\"sleep_time\", \"value\":\"10\"}}";
  //    {"comand": {
  //                   "action":"sleep_time",
  //                   "value":"10"
  //                  }
  //      }

  String message = "";

  ESP_LOGI(TAG, "MQTT message topic %s", topic);

  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  // Convert Payload to a JSON object
  StaticJsonDocument<500> doc;
  deserializeJson(doc, message);
  serializeJsonPretty(doc, Serial);

  // Check if there is a incomming command

  const char *action_in = doc["command"]["action"];
  const char *value = doc["command"]["value"];

  String action = String(action_in);
  ESP_LOGI(TAG, " action: %s  value: %s", action.c_str(), value);

#if (USE_FASTLED)
  if (action == "LED_HeatColor")
  {
    ESP_LOGI(TAG, "MQTT: LED Heat Color");
    LED_HeatColor(atoi(value));
  }

  if (action == "LED_on")
  {
    ESP_LOGI(TAG, "MQTT: LED on");
    LED_on(atoi(value));
  }

  if (action == "LED_off")
  {
    ESP_LOGI(TAG, "MQTT: LED off");
    LED_off();
  }

#endif

  if (action == "reset_gauge")
  {
    ESP_LOGI(TAG, "MQTT: Reset Coulomb Counter");
#if (HAS_PMU)
    pmu.ClearCoulombcounter();
#endif
  }

  if (action == "sleep_time")
  {
    dataBuffer.settings.sleep_time = atoi(value);
    ESP_LOGI(TAG, "MQTT: sleep time %2d", dataBuffer.settings.sleep_time);
    saveConfiguration();
  }

#if (USE_PWM_SERVO)
    if (action == "servo1")
  {
    dataBuffer.data.servo1 = atoi(value);
    ESP_LOGI(TAG, "MQTT: move servo 1 to %3d degree", dataBuffer.data.servo1);
    servo_move_to();
  }
#endif

#if (HAS_PMU)
  if (action == "set_bat_max_charge_current")
  {
    dataBuffer.settings.bat_max_charge_current = atoi(value);
    ESP_LOGI(TAG, "MQTT: set max charge current %2d", dataBuffer.settings.bat_max_charge_current);
    pmu.setChargeControlCur(dataBuffer.settings.bat_max_charge_current);
    saveConfiguration();
  }
#endif

  if (action == "set_experiment")
  {
    dataBuffer.settings.experiment = value;
    ESP_LOGI(TAG, "MQTT: Experiment", dataBuffer.settings.experiment);
    saveConfiguration();
  }
}

void reconnect()
{

  char topic_in[40];
  // build MQTT topic e.g.  mrflexi/device/soil_moisture-01/data
  doConcat(mqtt_topic, DEVICE_NAME, mqtt_topic_mosi, topic_in);

  int i = 0;

  if (WiFi.status() == WL_CONNECTED)
  {
    // Loop until we're reconnected
    while (!MqttClient.connected())
    {
      ESP_LOGI(TAG, "Attempting MQTT connection...");
      // Attempt to connect
      if (MqttClient.connect(DEVICE_NAME))
      {
        
        MqttClient.publish(mqtt_topic, "connected");
        MqttClient.subscribe(topic_in);
        ESP_LOGI(TAG, "Subscribed to topic %s", topic_in);
      }
      else
      {
        ESP_LOGE(TAG, "failed, rc=");
        Serial.print(MqttClient.state());
        ESP_LOGE(TAG, " try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(500);
      }
      i++;
    }
  }
  else
    ESP_LOGE(TAG, "No Wifi connection");
}

void setup_mqtt()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    MqttClient.setServer(mqtt_server, 1883);
    MqttClient.setCallback(callback);
    MqttClient.setBufferSize(500);
    MqttClient.setSocketTimeout(120);

    if (!MqttClient.connected())
    {
      reconnect();
    }
    ESP_LOGI(TAG, "MQTT connected");
    log_display("MQTT connected");
    MqttClient.publish(mqtt_topic, "ESP32 is alive...");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  printLocalTime();
  }



}

void mqtt_send()
{
  char topic_out[50];
  Serial.println();
  ESP_LOGI(TAG, "MQTT send");

#if (USE_CAMERA)
sendPhoto();
  //dataBuffer.data.buf = (const char*) captureImage()->buf;
  //Serial.println(strlen(dataBuffer.data.buf));
#endif

  // build MQTT topic e.g.  mrflexi/device/TBEAM-01/data
  doConcat(mqtt_topic, DEVICE_NAME, mqtt_topic_miso, topic_out);

  if (MqttClient.connected())
  {
    ESP_LOGI(TAG, "MQTT send:  %s", topic_out);
    ESP_LOGI(TAG, "Payload: %s", dataBuffer.to_json().c_str());
    MqttClient.publish(topic_out, dataBuffer.to_json().c_str());
  }
  else
  {
    ESP_LOGE(TAG, "Mqtt not connected");
  }
}

void mqtt_send_lok(int id, uint16_t speed, int dir)
{
  char topic_out[40];
  const int capacity = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  String JsonStr;

  // build MQTT topic e.g.
  doConcat(mqtt_topic, "Lok", "", topic_out);
  doc.clear();
  doc["command"] = "Lok";
  doc["id"] = id;
  doc["speed"] = speed;
  doc["dir"] = dir;
  serializeJson(doc, JsonStr);

  if (MqttClient.connected())
  {
    MqttClient.publish(topic_out, JsonStr.c_str());
    ESP_LOGI(TAG, "MQTT send:  %s", topic_out);
    ESP_LOGI(TAG, "Payload: %s", JsonStr);
  }
  else
  {
    ESP_LOGE(TAG, "Mqtt not connected");
  }
}

void mqtt_send_irq()
{
}
#endif