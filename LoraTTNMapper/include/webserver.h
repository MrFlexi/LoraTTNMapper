#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <ArduinoJson.h>

#if (USE_WEBSERVER)
 extern AsyncWebServer server;
 
 void setup_webserver();
#endif

#endif