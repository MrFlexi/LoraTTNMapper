
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include "esp_system.h"
#include "esp_spi_flash.h"


#define USE_OTA 0
#define USE_BME280 1

#define HAS_LORA 1
#define USE_MQTT 0
#define HAS_INA 0
#define USE_DASH 0
#define USE_GPS 1
#define USE_DISPLAY 1
#define USE_INTERRUPTS 1
#define USE_BLE 0
#define USE_SERIAL_BT 0

#define USE_WIFI 1
#define USE_WEBSERVER   1
#define USE_WEBSOCKET   1
#define USE_CAYENNE 1

#define USE_GYRO  1
#define WAKEUP_BY_MOTION 1

#define USE_FASTLED 1
#define FASTLED_SHOW_DEGREE 0
#define USE_POTI 1

#define displayRefreshIntervall 2       // every x second
#define displayMoveIntervall 5          // every x second

#define LORAenqueueMessagesIntervall 90 // every x seconds
#define LORA_TX_INTERVAL 30

#define sendCayenneIntervall 60 // every x seconds
#define sendWebserverIntervall 10 // every x seconds

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 51
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#define I2CMUTEXREFRES_MS 40

#define I2C_MUTEX_LOCK() \
  (xSemaphoreTake(I2Caccess, pdMS_TO_TICKS(I2CMUTEXREFRES_MS)) == pdTRUE)

#define I2C_MUTEX_UNLOCK() (xSemaphoreGive(I2Caccess))

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1              // Main switch
#define uS_TO_S_FACTOR 1000000   //* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10        // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  6 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 10 // after n Lora TX events

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Ticker.h>
#include "esp_sleep.h"
#include <Wire.h>

#if (USE_WEBSERVER || USE_CAYENNE)
#include "WiFi.h"
#endif


#if (USE_WEBSERVER)
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "webserver.h"
#include "websocket.h"
#endif

#if (USE_BME280)
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#endif

#if (USE_SERIAL_BT)
#include "BluetoothSerial.h"
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
  float raw_temperature;    // raw temperature signal
  float raw_humidity;       // raw humidity signal
  float gas;                // raw gas sensor signal
  uint8_t aliveCounter;     // aliveCounter
  uint8_t LoraQueueCounter; // aliveCounter
  uint8_t sleepCounter;     // aliveCounter
  uint8_t MotionCounter;     // aliveCounter
  uint8_t bootCounter;
  uint8_t txCounter;        // aliveCounter
   uint8_t rxCounter;        // aliveCounter
  uint8_t runmode;          // aliveCounter
  uint32_t freeheap;        // free memory
  uint8_t tx_ack_req;       // request TTN to acknowlede a TX
  uint16_t potentiometer_a;   //
  bool  wlan;
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
  uint32_t bat_ChargeCoulomb = 0;
  uint32_t bat_DischargeCoulomb = 0;
  double yaw = 0;
  double pitch = 0;
  double roll = 0;
  String ip_address;
  uint8_t operation_mode = 0;
  esp_sleep_wakeup_cause_t wakeup_reason;
} deviceStatus_t;

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

#include "../src/hal/ttgobeam10.h"
#include "power.h"
#include "display.h"
#include "gps.h"
#include "i2cscan.h"
#include "irqhandler.h"

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

#if (USE_GYRO)
#include "gyro.h"
#endif

#if (USE_BLE)
#include <ble.h>
#endif

#if (USE_FASTLED)
#include <Led.h>
#endif

#if (USE_POTI)
#include <AnalogSmooth.h>
#include "poti.h"
#endif

#endif