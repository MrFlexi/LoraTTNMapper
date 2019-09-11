#include "globals.h"

HAS_DISPLAY u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // ESP32 Thing, HW I2C with pin remapping
U8G2LOG u8g2log;
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

void log_display(String s)
{
  Serial.println(s);
  //if (runmode < 1)
  //{
    u8g2log.print(s);
    u8g2log.print("\n");
  //}
}

void setup_display(void)
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_mf);                         // set the font for the terminal window
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0);                               // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);                                     // 0: Update screen with newline, 1: Update screen for every char
  u8g2.enableUTF8Print();
  log_display("TTN-ABP-Mapper");
}



void showPage(int page)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0,20,"Hello World!");
  u8g2.sendBuffer();
}
  