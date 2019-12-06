//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#ifndef _DISPLAY_H
#define _DISPLAY_H

extern int PageNumber; 
#define PAGE_COUNT 3
#define PAGE_VALUES 1
#define PAGE_SOLAR 2
#define PAGE_SETTINGS 3
#define PAGE_SLEEP 10         // Pages > 10 are not in the picture loop



#include "globals.h"

#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define SLEEP 10
#define ICON_NOTES 11



// assume 4x6 font, define width and height
#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 6

#include <U8g2lib.h>
//#include <ss_oled.h>

extern HAS_DISPLAY u8g2;             // 
extern U8G2LOG u8g2log;             // Create a U8g2log object


// allocate memory
extern uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];


void setup_display(void);
void log_display(String s);
void setup_display_new();
void dp_printf(uint16_t x, uint16_t y, uint8_t font, uint8_t inv,
               const char *format, ...);

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
