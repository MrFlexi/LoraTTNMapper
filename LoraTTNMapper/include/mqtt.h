#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#if (USE_MQTT)
#include <PubSubClient.h>


//const char *mqtt_server = "192.168.1.144"; // Laptop
//const char *mqtt_server = "test.mosquitto.org"; // Laptop


void mqtt_loop();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void mqtt_send();
void setup_mqtt();

#endif
