#pragma once

#if (USE_VL53L1X)

#include <Arduino.h>
#include "globals.h"

#include <Wire.h>
#include <VL53L1X.h>

void setup_VL53L1X();
void get_VL53L1X_data();

#endif

