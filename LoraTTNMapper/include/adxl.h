//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#ifndef _ADXL_H
#define _ADXL_H

#include <Wire.h>
#include <ADXL345.h>
#include "globals.h"

//extern ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

void setup_adxl345( void );
void adxl_dumpValues(void);

#endif