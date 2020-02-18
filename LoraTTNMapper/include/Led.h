//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#ifndef _FLED_H
#define _FLED_H

#include <Wire.h>
#include <Arduino.h>

#include "FastLED.h"


#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    12
#define BRIGHTNESS  20



void setup_FastLed(void); 
void loop_FastLed(void);
void LED_showSleepCounter(void);

#endif