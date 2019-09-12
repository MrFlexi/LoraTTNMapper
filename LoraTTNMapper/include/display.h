//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------


#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "globals.h"

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 6

#include <U8g2lib.h>

extern HAS_DISPLAY u8g2;             // 
extern U8G2LOG u8g2log;             // Create a U8g2log object

// allocate memory
extern uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

void log_display(String s);
void setup_display(void);
void showPage(int page);


class DataBuffer
{
  public:
    DataBuffer();
    void set( bmeStatus_t input );
    void get();  
    bmeStatus_t data ;
};

extern DataBuffer dataBuffer;
extern bmeStatus_t sensorValues;

#endif