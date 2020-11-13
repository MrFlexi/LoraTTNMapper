#pragma once


//--------------------------------------------------------------------------
// i2c Sensors
//--------------------------------------------------------------------------


#if (HAS_INA3221)
extern SDL_Arduino_INA3221 ina3221;
void print_ina3221();
#endif


#if (HAS_INA219)
#include <Adafruit_INA219.h>
extern Adafruit_INA219 ina219;

void print_ina219();
#endif

#if (USE_BME280)
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
extern Adafruit_BME280 bme; // I2C   PIN 21 + 22
#endif


void setup_i2c_sensors();

