#pragma once


#if (HAS_INA3221)
extern SDL_Arduino_INA3221 ina3221;

void print_ina3221();
void setup_ina3221()
#endif



#if (HAS_INA219)
#include <Adafruit_INA219.h>
extern Adafruit_INA219 ina219;

void print_ina219();
void setup_ina219();

#endif

