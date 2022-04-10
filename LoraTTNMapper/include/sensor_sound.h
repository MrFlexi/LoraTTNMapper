/***************************************************************************************************/
/*
  This is an Arduino sketch for HC-SR04, HC-SRF05, DYP-ME007, BLJ-ME007Y ultrasonic ranging sensor
  Operating voltage:    5v
  Operating current:    10..20mA
  Working range:        4cm..250cm
  Measuring angle:      15°
  Operating frequency:  40kHz
  Resolution:           0.3cm
  Maximum polling rate: 20Hz
  written by : enjoyneering79
  sourse code: https://github.com/enjoyneering/
*/
/***************************************************************************************************/
/*
HCSR04(trigger, echo, temperature, distance)
trigger     - trigger pin
echo        - echo pin
temperature - ambient temperature, in C
distance    - maximun measuring distance, in cm
*/


#pragma once
#if (USE_I2C_MICROPHONE)

#include <Arduino.h>
#include "globals.h"

void setup_sound_rtos();

#endif

