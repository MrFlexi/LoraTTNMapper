#ifndef _DASH_H
#define _DASH_H

#if (USE_DASH)
#include "globals.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

extern AsyncWebServer server;

extern void update_web_dash(void);
extern void create_web_dash(void);
#endif

#endif