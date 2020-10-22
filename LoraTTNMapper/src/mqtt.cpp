#include "globals.h"
#include "mqtt.h"

#if (USE_MQTT)
PubSubClient MqttClient(wifiClient);

//const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_server = "85.209.49.65"; // Netcup
const char *mqtt_topic = "mrflexi/device/";
const char *mqtt_topic_mosi = "/mosi";
const char *mqtt_topic_miso = "/miso";

long lastMsgAlive = 0;
long lastMsgDist = 0;

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
    ESP_LOGE(TAG, "Wifi not connected ");
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

  String action = String( action_in );
  Serial.println(action);

  ESP_LOGI(TAG, " action: %s  value: %s", action, value);

  #if (USE_FASTLED)
  if ( action2 == "LED_HeatColor")
    {
      ESP_LOGI(TAG, "MQTT: LED Heat Color");
      LED_HeatColor(atoi(value));
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
      save_settings();
    }
  }

void reconnect()
{

  char topic_in[40];
  // build MQTT topic e.g.  mrflexi/device/TBEAM-01/data
  doConcat(mqtt_topic, DEVICE_NAME, mqtt_topic_mosi, topic_in);

  // Loop until we're reconnected
  while (!MqttClient.connected())
  {
    ESP_LOGI(TAG, "Attempting MQTT connection...");
    // Attempt to connect
    if (MqttClient.connect(DEVICE_NAME))
    {
      ESP_LOGI(TAG, "connected");
      MqttClient.publish(mqtt_topic, "connected");
      MqttClient.subscribe(topic_in);
    }
    else
    {
      ESP_LOGE(TAG, "failed, rc=");
      Serial.print(MqttClient.state());
      ESP_LOGE(TAG, " try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
    log_display("MQTT connected");
    MqttClient.publish(mqtt_topic, "ESP32 is alive...");
  }
}

void mqtt_send()
{
  const int capacity = JSON_OBJECT_SIZE(19) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  char topic_out[40];

  // build MQTT topic e.g.  mrflexi/device/TBEAM-01/data
  doConcat(mqtt_topic, DEVICE_NAME, mqtt_topic_miso, topic_out);
  ESP_LOGI(TAG, "MQTT send:  %s", topic_out);

  doc.clear();
  doc["device"] = DEVICE_NAME;
  doc["BootCounter"] = String(dataBuffer.data.bootCounter);
  doc["bat_voltage"] = String(dataBuffer.data.bat_voltage);
  doc["bat_charge_current"] = String(dataBuffer.data.bat_charge_current);
  doc["bat_discharge_current"] = String(dataBuffer.data.bat_discharge_current);
  doc["bat_charge_current"] = String(dataBuffer.data.bat_charge_current);
  doc["bat_fuel_gauge"] = String(dataBuffer.data.bat_DeltamAh);

  doc["bus_voltage"] = String(dataBuffer.data.bus_voltage);
  doc["bus_current"] = String( dataBuffer.data.bus_current);   

  doc["panel_voltage"] = dataBuffer.data.panel_voltage;
  doc["panel_current"] = dataBuffer.data.panel_current;

  doc["TXCounter"] = String(dataBuffer.data.txCounter);
  doc["temperature"] = String(dataBuffer.data.temperature);
  doc["humidity"] = String(dataBuffer.data.humidity);
  doc["cpu_temperature"] = String(dataBuffer.data.cpu_temperature);
  doc["cpu_free_heap"] = String(dataBuffer.data.freeheap);
  

  // Add the "location"
  JsonObject location = doc.createNestedObject("location");
  location["lat"] = dataBuffer.data.gps.lat();
  location["lon"] = dataBuffer.data.gps.lng();

  char buffer[600];
  serializeJson(doc, buffer);
  MqttClient.publish(topic_out, buffer);
  serializeJsonPretty(doc, buffer);
  ESP_LOGI(TAG, "Payload: %s", buffer);
  Serial.println();
}
#endif