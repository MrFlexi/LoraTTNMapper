#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H

#include <ArduinoJson.h>


// local Tag for logging

 extern AsyncWebSocket ws;

void t_broadcast_message(void *parameter);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

#endif
