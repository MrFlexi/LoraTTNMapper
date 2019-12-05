
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>

#define USE_WIFI 1
#define USE_OTA 1
#define USE_BME280 0
#define USE_CAYENNE 0
#define HAS_LORA 1
#define USE_MQTT 0
#define HAS_INA 0
#define USE_DASH 0
#define USE_GPS 1
#define USE_BLE 1

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 51
#define SEND_QUEUE_SIZE 10

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#define I2CMUTEXREFRES_MS 40

#define I2C_MUTEX_LOCK() \
  (xSemaphoreTake(I2Caccess, pdMS_TO_TICKS(I2CMUTEXREFRES_MS)) == pdTRUE)

#define I2C_MUTEX_UNLOCK() (xSemaphoreGive(I2Caccess))

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 0              // Main switch
#define uS_TO_S_FACTOR 1000000   //* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 2          // sleep for n minute
#define TIME_TO_NEXT_SLEEP 10    // sleep after n minutes or
#define SLEEP_AFTER_N_TX_COUNT 3 // after n Lora TX events

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Ticker.h>
#include "esp_sleep.h"
#include <Wire.h>
#include "WiFi.h"

#if (HAS_BME)
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#endif

#include "esp_log.h"
//#include <Preferences.h>

//--------------------------------------------------------------------------
// Wifi Settings
//--------------------------------------------------------------------------
const char ssid[] = "MrFlexi";
const char wifiPassword[] = "Linde-123";
extern bool wifi_connected;
extern WiFiClient wifiClient;

typedef struct
{
  float iaq;                // IAQ signal
  uint8_t iaq_accuracy;     // accuracy of IAQ signal
  float temperature;        // temperature signal
  float humidity;           // humidity signal
  float pressure;           // pressure signal
  float raw_temperature;    // raw temperature signal
  float raw_humidity;       // raw humidity signal
  float gas;                // raw gas sensor signal
  uint8_t aliveCounter;     // aliveCounter
  uint8_t LoraQueueCounter; // aliveCounter
  uint8_t sleepCounter;     // aliveCounter
  uint8_t txCounter;        // aliveCounter
  float firmware_version;
  uint8_t bytesReceived;
  lmic_t lmic;
  float panel_voltage = 0;
  float panel_current = 0;
  float bus_voltage = 0;
  float bus_current = 0;
  float bat_voltage = 0;
  float bat_charge_current = 0;
  float bat_discharge_current = 0;
} deviceStatus_t;

// Struct holding payload for data send queue
typedef struct
{
  uint8_t MessageSize;
  uint8_t MessagePort;
  uint8_t MessagePrio;
  uint8_t Message[PAYLOAD_BUFFER_SIZE];
} MessageBuffer_t;

extern int runmode;
extern SemaphoreHandle_t I2Caccess;
extern QueueHandle_t LoraSendQueue;

#include "../src/hal/ttgobeam10.h"
#include "power.h"
#include "display.h"
#include "gps.h"
#include "i2cscan.h"

#if (HAS_INA)
#include "INA3221.h"
#endif

#include "payload.h"

#if (USE_MQTT)
#include "mqtt.h"
#endif

#ifdef HAS_BUTTON
#include "button.h"
#endif

#if (USE_DASH)
#include "dash.h"
#endif

#if (HAS_LORA)
#include "lora.h"
#endif

#if (USE_OTA)
#include "SecureOTA.h"
#endif

#if (USE_BLE)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#endif

#endif