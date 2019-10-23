
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>

#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#define I2CMUTEXREFRES_MS  40

#define I2C_MUTEX_LOCK()                                                       \
  (xSemaphoreTake(I2Caccess, pdMS_TO_TICKS(I2CMUTEXREFRES_MS)) == pdTRUE)

#define I2C_MUTEX_UNLOCK() (xSemaphoreGive(I2Caccess))

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1            // Main switch
#define uS_TO_S_FACTOR 1000000 //* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1        // sleep for 1 minute
#define TIME_TO_NEXT_SLEEP  2      // sleep after n minutes or
#define SLEEP_AFTER_N_TX_COUNT 2   // after n Lora TX events


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
#include <Preferences.h>

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
  uint8_t sleepCounter;   // aliveCounter 
  uint8_t txCounter;   // aliveCounter    
  uint8_t bytesReceived;   
  lmic_t  lmic;
  float    bat_voltage = 0;
  float    bat_current = 0;
} deviceStatus_t;


extern int runmode;
extern SemaphoreHandle_t I2Caccess;

#include "../src/hal/ttgobeam10.h"
#include "power.h"
#include "display.h"
#include "gps.h"
#include "i2cscan.h"
#include "INA3221.h"

#endif