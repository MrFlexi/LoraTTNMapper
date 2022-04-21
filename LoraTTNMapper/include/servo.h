#pragma once

#include <Arduino.h>
#include "globals.h"
#include <Wire.h>

#define USE_PCA9685_SERVO_EXPANDER 

void setup_servo_pwm();
void servo_pwm_test();
void servo_move_to();

