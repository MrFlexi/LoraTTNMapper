#include "globals.h"
#include "mqtt.h"


PubSubClient MqttClient(wifiClient);

//const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_server = "85.209.49.65"; // Netcup
const char *mqtt_topic = "mrflexi/solarserver/";

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
  Serial.print("Message arrived [");

  u8g2log.print(topic);
  u8g2log.print("\n");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    u8g2log.print((char)payload[i]);
  }
  Serial.println();
  u8g2log.print("\n");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
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
      MqttClient.publish("MrFlexi/nodemcu", "connected");
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

  if (!MqttClient.connected())
  {
    reconnect();
  }

  log_display("MQTT connected");
  MqttClient.publish("mrflexi/solarserver/info", "ESP32 is alive...");
}


void mqtt_send()
{

  const int capacity=JSON_OBJECT_SIZE(16)+JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
 
  doc.clear();

  doc["device"] = DEVICE_NAME;
  doc["BootCounter"] = String( dataBuffer.data.bootCounter );

  doc["bat_voltage"] = String( dataBuffer.data.bat_voltage);
  //doc["bat_charge_current"] = String( dataBuffer.data.bat_charge_current);
  //doc["bat_discharge_current"] = String( dataBuffer.data.bat_discharge_current);
  //doc["bat_charge_current"] = String( dataBuffer.data.bat_charge_current);
  //doc["bat_fuel_gauge"] = String( dataBuffer.data.bat_DeltamAh);

  doc["panel_voltage"] = dataBuffer.data.panel_voltage;
  doc["panel_current"] = dataBuffer.data.panel_current;
 
  doc["TXCounter"] = String( dataBuffer.data.txCounter );
  doc["temperature"] = String( dataBuffer.data.temperature );
  doc["humidity"] = String( dataBuffer.data.humidity);
  doc["cpu_temperature"] = String( dataBuffer.data.cpu_temperature);

  // Add the "location" 
  JsonObject location = doc.createNestedObject("location");
  location["lat"]= dataBuffer.data.gps.lat();
  location["lon"]= dataBuffer.data.gps.lng();
 
 char buffer[600];
  serializeJson(doc, buffer);
  //Serial.println();
  //Serial.println(buffer);
  //Serial.println();
  MqttClient.publish("mrflexi/device",buffer);
  serializeJsonPretty(doc, Serial);
}
