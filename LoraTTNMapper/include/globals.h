
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 10           // Watchdog time out 3 seconds

//--------------------------------------------------------------------------
// Device Settings
//--------------------------------------------------------------------------
#define DEVICE_ID  2

#if DEVICE_ID == 1                 // TBEAM-01 Device EU ID = DE00000000000010
#include "device_01.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 2                 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_02.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 3                 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_03.h"
#include "../src/hal/ttgobeam10.h"
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

#if (USE_WEBSERVER || USE_CAYENNE || USE_MQTT || USE_WIFI )
#include "WiFi.h"
extern WiFiClient wifiClient;
#endif


#if (USE_WEBSERVER)
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

#include "gps.h"

#include "esp_log.h"
//#include <Preferences.h>

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
  uint32_t bat_ChargeCoulomb = 0;
  uint32_t bat_DischargeCoulomb = 0;
  float    bat_DeltamAh = 0;
  bool  wlan;
  bool  pictureLoop = true;
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
  double yaw = 0;
  double pitch = 0;
  double roll = 0;
  String ip_address;
  uint8_t operation_mode = 0;
  esp_sleep_wakeup_cause_t wakeup_reason;
  TinyGPSLocation gps;
  TinyGPSLocation gps_old;
  double gps_distance;  
} deviceStatus_t;


typedef struct
{
  uint8_t sleep_time;
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

#if (HAS_INA)
extern SDL_Arduino_INA3221 ina3221;
#endif

#if (USE_MQTT)
#include "mqtt.h"
#endif

#if (USE_BUTTON)
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

#if (USE_BLE_SCANNER)
#include "ble_scanner.h"
#endif


#if (USE_FASTLED)
#include <Led.h>
#endif

#if (USE_POTI)
#include <AnalogSmooth.h>
#include "poti.h"
#endif

#endif