
#ifndef _GLOBALS_H
#define _GLOBALS_H

#define GPS_TX          12
#define GPS_RX          15

#define HAS_DISPLAY     U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Ticker.h>
#include "esp_sleep.h"
#include <Wire.h>
#include "WiFi.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "esp_log.h"



typedef struct {
  float iaq;             // IAQ signal
  uint8_t iaq_accuracy;  // accuracy of IAQ signal
  float temperature;     // temperature signal
  float humidity;        // humidity signal
  float pressure;        // pressure signal
  float raw_temperature; // raw temperature signal
  float raw_humidity;    // raw humidity signal
  float gas;             // raw gas sensor signal
  uint8_t aliveCounter;   // aliveCounter   
  char message[20];
} bmeStatus_t;


#include "display.h"
#include "gps.h"
#include "i2cscan.h"


  
#endif