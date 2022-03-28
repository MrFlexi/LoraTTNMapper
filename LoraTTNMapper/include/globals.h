
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
// #include <esp_task_wdt.h>

#define WDT_TIMEOUT 10          // Watchdog time out x seconds
#define uS_TO_S_FACTOR 1000000UL  //* Conversion factor for micro seconds to seconds */
#define SEALEVELPRESSURE_HPA (1013.25)
#define LORA_DATARATE   DR_SF7

//--------------------------------------------------------------------------
// GPIO
//--------------------------------------------------------------------------
// 5,18,19,23,26,27,s 33, 32    --> LORA
// 35                           --> PMU Interrupt
// 12,34                        --> GPS
// 36                           --> ADC Channel 0 --> Poti, Soil Moist Sensor
//
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Device Settings
//--------------------------------------------------------------------------

#define DEVICE_ID 1

#if DEVICE_ID == 1                 // TBEAM-01 Device EU ID = DE00000000000010
#include "device_01.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 2                 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_02.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 3                 // 
#include "device_03.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 4                 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_04.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == camera_01                 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_cam_01.h"
#include "../src/hal/ttgoCameraPlus.h"
#endif

#define I2CMUTEXREFRES_MS 40
#define I2C_MUTEX_LOCK() \
  (xSemaphoreTake(I2Caccess, pdMS_TO_TICKS(I2CMUTEXREFRES_MS)) == pdTRUE)
#define I2C_MUTEX_UNLOCK() (xSemaphoreGive(I2Caccess))


#include "SPIFFS.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Ticker.h>
#include "esp_sleep.h"
#include <Wire.h>
#include "settings.h"
#include <AnalogSmooth.h>

#if (USE_WEBSERVER || USE_CAYENNE || USE_MQTT || USE_WIFI )
#include "WiFi.h"
extern WiFiClient wifiClient;
#endif

#if (USE_SERVO)
#include <ESP32Servo.h>
// Published values for SG90 servos; adjust if needed
extern int minUs;
extern int maxUs;

#ifdef SERVO1_PIN
extern Servo servo1;
#endif

#ifdef SERVO2_PIN
extern Servo servo2;
#endif

#endif

#if (USE_WEBSERVER)
#include "ESPAsyncWebServer.h"
#include "webserver.h"
#include "websocket.h"
#endif

#include "gps.h"

#include "esp_log.h"

//--------------------------------------------------------------------------
// Wifi Settings
//--------------------------------------------------------------------------
const char ssid[] = "MrFlexi";
const char wifiPassword[] = "Linde-123";
extern bool wifi_connected;

extern volatile bool mpuInterrupt;

enum pmu_power_t
{
  pmu_power_on,
  pmu_power_off,
  pmu_power_sleep
};

typedef struct
{
  float iaq;                // IAQ signal
  uint8_t iaq_accuracy;     // accuracy of IAQ signal
  float temperature;        // temperature signal
  float humidity;           // humidity signal
  float pressure;           // pressure signal
  float cpu_temperature;    // raw temperature signal
  float raw_temperature;    // raw temperature signal
  float raw_humidity;       // raw humidity signal
  float gas;                // raw gas sensor signal
  uint8_t aliveCounter;     // aliveCounter
  uint8_t LoraQueueCounter; // aliveCounter
  uint8_t sleepCounter;     // aliveCounter
  uint8_t MotionCounter;     // aliveCounter
  uint16_t bootCounter;
  uint8_t txCounter;        // aliveCounter
   uint8_t rxCounter;        // aliveCounter
  uint8_t runmode;          // aliveCounter
  uint8_t CoronaDeviceCount;
  uint32_t freeheap;        // free memory
  uint8_t tx_ack_req;       // request TTN to acknowlede a TX
  uint16_t potentiometer_a;   //
  uint16_t adc0;              // Voltage in mVolt
  bool potentiometer_a_changed;
  uint32_t bat_ChargeCoulomb = 0;
  uint32_t bat_DischargeCoulomb = 0;
  float    bat_DeltamAh = 0;
  uint8_t  bat_max_charge_curr = 0;
  bool  wlan;
  bool  pictureLoop = true;
  float firmware_version;
  uint8_t bytesReceived;
  lmic_t lmic;
  bool pmu_data_available;
  float panel_voltage = 0;
  float panel_current = 0;
  float bus_voltage = 0;
  float bus_current = 0;
  float bat_voltage = 0;
  float bat_charge_current = 0;
  float bat_discharge_current = 0;
  float soil_moisture = 0;
  double yaw = 0;
  double pitch = 0;
  double roll = 0;
  String ip_address;
  uint8_t operation_mode = 0;
  esp_sleep_wakeup_cause_t wakeup_reason;
  TinyGPSLocation gps;
  TinyGPSLocation gps_old;
  double gps_distance;
  char gps_datetime[32];
} deviceStatus_t;


typedef struct
{
  uint8_t sleep_time;
  const char* log_print_buffer;
  const char* experiment;
  uint8_t bat_max_charge_current;  
} deviceSettings_t;


// Struct holding payload for data send queue
typedef struct
{
  uint8_t MessageSize;
  uint8_t MessagePort;
  uint8_t MessagePrio;
  uint8_t Message[PAYLOAD_BUFFER_SIZE];
} MessageBuffer_t;

extern SemaphoreHandle_t I2Caccess;
extern TaskHandle_t irqHandlerTask;
extern TaskHandle_t moveDisplayHandlerTask;
extern TaskHandle_t t_cyclic_HandlerTask;
extern QueueHandle_t LoraSendQueue;

#include "power.h"
#include "jsutilities.h"
#include "display.h"
#include "irqhandler.h"
#include "payload.h"

#if (HAS_INA3221 || HAS_INA219 || USE_BME280 )
#include "i2c_sensors.h"
#endif

#if (USE_MQTT)
#include "mqtt.h"
#endif

#if (USE_BUTTON)
#include "button.h"
#endif

#if (HAS_LORA)
#include <lmic.h>
#include "lora.h"
#include "payload.h"
#endif

#if (USE_OTA)
#include "SecureOTA.h"
#endif

#if (USE_FASTLED)
#include <Led.h>
#endif

#if (USE_POTI || USE_SOIL_MOISTURE)
#include "poti.h"
#endif

#if (USE_CAMERA)
#include <camera.h>
#endif

#endif