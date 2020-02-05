#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H

#include <ArduinoJson.h>

StaticJsonDocument<500> doc;
// local Tag for logging

 extern AsyncWebSocket ws;

void t_broadcast_message(void *parameter);
