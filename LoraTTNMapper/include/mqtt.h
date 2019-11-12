#ifndef _MQTT_H
#define _MQTT_H

#include "globals.h"

#if (USE_MQTT)
#include <PubSubClient.h>

//const char *mqtt_server = "192.168.1.144"; // Laptop
//const char *mqtt_server = "test.mosquitto.org"; // Laptop
const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_topic = "mrflexi/solarserver/";

PubSubClient client(wifiClient);
long lastMsgAlive = 0;
long lastMsgDist = 0;

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void setup_mqtt()

#endif
#endif