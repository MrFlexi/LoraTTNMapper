//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#pragma once

#include "globals.h"
#include "databuffer.h"

#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define SLEEP 10
#define ICON_NOTES 11
#define ICON_SMILE 12
#define ICON_BOOT  13


#define PAGE_TBEAM 0
#define PAGE_MODULS 1
#define PAGE_LORA 2
#define PAGE_GPS 3
#define PAGE_SOLAR 4
#define PAGE_BAT 5
#define PAGE_SENSORS 6
#define PAGE_GYRO 7
#define PAGE_POTI 8
#define PAGE_SUN 9
#define PAGE_SPRINKLER 10
#define PAGE_WIFICOUNTER 11
#define PAGE_SYSTEM 12

#define PAGE_SLEEP 20         // Pages > 20 are not in the picture loop
#define PAGE_BOOT 21
#define PAGE_OTA 22

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

#if (HAS_TFT_DISPLAY)

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
extern TFT_eSPI tft;

#endif
