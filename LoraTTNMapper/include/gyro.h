//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#ifndef _ADXL_H
#define _ADXL_H

#include <Wire.h>
#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"
#include "globals.h"

int brightness = 255;
volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high


void gyro_handle_interrupt(void);
//extern ADXL345 adxl; //variable adxl is an instance of the ADXL345 library



#endif