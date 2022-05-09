#pragma once

#include <Arduino.h>
#include "globals.h"
#include <Wire.h>

#if (USE_SUN_POSITION)
#include "Helios.h"
#endif

#define USE_PCA9685_SERVO_EXPANDER 
//#define TRACE 

void setup_servo_pwm();
void servo_pwm_test();
void servo_move_to( uint8_t servo_number, uint8_t servo_position );
void servo_move_to_sun();
void servo_move_to_last_position();

