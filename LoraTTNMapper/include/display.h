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


#define PAGE_TBEAM 1
#define PAGE_LORA 2
#define PAGE_GPS 3
#define PAGE_SOLAR 4
#define PAGE_BAT 5
#define PAGE_SENSORS 6
#define PAGE_GYRO 7
#define PAGE_POTI 8
#define PAGE_MODULS 9

#define PAGE_SLEEP 10         // Pages > 10 are not in the picture loop

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 6

#include <U8g2lib.h>

extern HAS_DISPLAY u8g2;             // 
extern U8G2LOG u8g2log;             // Create a U8g2log object
extern int PageNumber; 

// allocate memory
extern uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

void log_display(String s);
void setup_display(void);
void t_moveDisplayRTOS(void *pvParameters);
void t_moveDisplay(void);

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
    String to_json();
    deviceStatus_t data ;
    deviceSettings_t settings;
    
    const char* getError() const { return _error; }

  private:
    char* _error;
   
};

extern DataBuffer dataBuffer;


#endif