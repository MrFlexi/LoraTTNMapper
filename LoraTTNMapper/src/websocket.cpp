#include "globals.h"
#include "websocket.h"

AsyncWebSocket ws("/ws");

String message_buffer_to_jsonstr(DataBuffer message_buffer)
{
 StaticJsonDocument<500> ws_json;
  
  String JsonStr;
  ws_json.clear();

  ws_json["MotionCounter"] = String( dataBuffer.data.MotionCounter );
  ws_json["TXCounter"] = "15";
  ws_json["Temperatur"] = "22.5";
  ws_json["text"] = "Hallo Welt";
  ws_json["text_time"] = "SA 8:22:01";

  // Add the "feeds" array
  JsonArray feeds = ws_json.createNestedArray("text_table");
  

  //for (int i = 0; i < message_buffer.error_msg_count; i++)
  //{
   JsonObject msg = feeds.createNestedObject();
    msg["title"] = "CPU Temp";
    msg["description"] = "400m Schwimmen in 4 Minuten";
    msg["value"] = "22.8";    
    feeds.add(msg);    

    msg["title"] = "TX Counter";    
    msg["description"] = "15";    
    feeds.add(msg);    

  //}  

  serializeJson(ws_json, JsonStr);
  //serializeJsonPretty(ws_json, Serial);
  return JsonStr;
}

void t_broadcast_message(void *parameter)
{
  // Task bound to core 0, Prio 0 =  very low

  String JsonStr;
  bool sendMessage = false;

  for (;;)
  {    
        JsonStr = message_buffer_to_jsonstr(dataBuffer);
        ws.textAll(JsonStr);        
    vTaskDelay(3000);
  }
}





void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len)
    {
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < info->len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < info->len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        Serial.println("I got your text message");
      else
        Serial.println("I got your binary message");
    }
    else
    {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
        if (info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if (info->final)
        {
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}
