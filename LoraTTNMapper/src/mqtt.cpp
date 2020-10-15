#include "globals.h"
#include "mqtt.h"

PubSubClient MqttClient(wifiClient);

//const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_server = "85.209.49.65"; // Netcup
const char *mqtt_topic = "mrflexi/device";

long lastMsgAlive = 0;
long lastMsgDist = 0;

void mqtt_loop()
{
  // MQTT Connection
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!MqttClient.connected())
    {
      Serial.print("MQTT Loop: MQTT Client not connected ");
      reconnect();
    }
    MqttClient.loop();
  }
  else
  {
    Serial.print("MQTT Loop: Wifi not connected ");
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

  Serial.print("MQTT message in [");
  u8g2log.print(topic);
  u8g2log.print("\n");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }

  // Convert Payload to a JSON object
  StaticJsonDocument<200> doc;
  deserializeJson(doc, message);
  serializeJsonPretty(doc, Serial);

  // Check if there is a incomming command
  const char *action = doc["command"]["action"];
  const char *value = doc["command"]["value"];
  if (action)
  {
      Serial.print( action ); Serial.println( value ); 

    if ( action== "reset_gauge")
    {
      Serial.println("MQTT: Reset Coulomb Counter");
#if (HAS_PMU)
      pmu.ClearCoulombcounter();
#endif
    }


if ( action== "sleep_time")
    {     
      dataBuffer.settings.sleep_time = atoi( value );
      save_settings();
    }
  }  
}

void reconnect()
{
  // Loop until we're reconnected
  while (!MqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (MqttClient.connect("Mqtt Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      MqttClient.publish(mqtt_topic, "connected");
      // ... and resubscribe
      MqttClient.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(MqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_mqtt()
{

  MqttClient.setServer(mqtt_server, 1883);
  MqttClient.setCallback(callback);
  MqttClient.setBufferSize(500);

  if (!MqttClient.connected())
  {
    reconnect();
  }

  log_display("MQTT connected");
  MqttClient.publish(mqtt_topic, "ESP32 is alive...");
}

void mqtt_send()
{

  const int capacity = JSON_OBJECT_SIZE(16) + JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;

  doc.clear();

  doc["device"] = DEVICE_NAME;
  doc["BootCounter"] = String(dataBuffer.data.bootCounter);

  doc["bat_voltage"] = String(dataBuffer.data.bat_voltage);
  doc["bat_charge_current"] = String(dataBuffer.data.bat_charge_current);
  doc["bat_discharge_current"] = String(dataBuffer.data.bat_discharge_current);
  doc["bat_charge_current"] = String(dataBuffer.data.bat_charge_current);
  doc["bat_fuel_gauge"] = String(dataBuffer.data.bat_DeltamAh);

  doc["panel_voltage"] = dataBuffer.data.panel_voltage;
  doc["panel_current"] = dataBuffer.data.panel_current;

  doc["TXCounter"] = String(dataBuffer.data.txCounter);
  doc["temperature"] = String(dataBuffer.data.temperature);
  doc["humidity"] = String(dataBuffer.data.humidity);
  doc["cpu_temperature"] = String(dataBuffer.data.cpu_temperature);

  // Add the "location"
  JsonObject location = doc.createNestedObject("location");
  location["lat"] = dataBuffer.data.gps.lat();
  location["lon"] = dataBuffer.data.gps.lng();

  char buffer[600];
  serializeJson(doc, buffer);
  //Serial.println();
  //Serial.println(buffer);
  //Serial.println();
  MqttClient.publish(mqtt_topic, buffer);
  serializeJsonPretty(doc, Serial);
}
