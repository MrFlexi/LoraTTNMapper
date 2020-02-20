//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#ifndef _ADXL_H
#define _ADXL_H

#include <Wire.h>
#include <Arduino.h>

void setup_gyro(void);
void gyro_handle_interrupt(void);
double gyro_get_yaw();
void gyro_get_values();

#endif