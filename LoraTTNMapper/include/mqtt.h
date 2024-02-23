#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#if (USE_MQTT)
#include <PubSubClient.h>

void mqtt_loop();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void mqtt_send();
void setup_mqtt();
void mqtt_send_irq();
void mqtt_send_lok(int id, uint16_t speed, int dir);

#if (USE_MPU6050)
void mqtt_send_mpu();
#endif

#endif
