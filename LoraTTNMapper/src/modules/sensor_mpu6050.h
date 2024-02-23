#pragma once

#if (USE_MPU6050)

#include <Arduino.h>
#include "globals.h"

// ---------------------------- MPU 6050 -------------------------------
#define OUTPUT_READABLE_YAWPITCHROLL
#define INTERRUPT_PIN 13
#define LED_PIN 13       // (Arduino is 13, Teensy is 11, Teensy++ is 6)
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"

void setup_mpu6050();
void get_mpu6050_data();

#endif

