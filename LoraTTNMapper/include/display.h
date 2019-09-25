//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------


#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "globals.h"

#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define SLEEP 10
#define ICON_NOTES 11


#define PAGE_VALUES 1
#define PAGE_SLEEP 2

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
void drawSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol);


class DataBuffer
{
  public:
    DataBuffer();
    void set( deviceStatus_t input );
    void get();  
    deviceStatus_t data ;
};

extern DataBuffer dataBuffer;
//extern deviceStatus_t sensorValues;

#endif