
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include "freertos/ringbuf.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
// #include <esp_task_wdt.h>

#define WDT_TIMEOUT 10           // Watchdog time out x seconds
#define uS_TO_S_FACTOR 1000000UL //* Conversion factor for micro seconds to seconds */
#define SEALEVELPRESSURE_HPA (1013.25)
#define LORA_DATARATE DR_SF7

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

// Choose Application
// #define DEVICE_ID 1
#define DEVICE_CAMERA_01   10 
#define DEVICE_SOIL_SENSOR 11


//#define DEVICE_ID DEVICE_CAMERA_01
#define DEVICE_ID DEVICE_SOIL_SENSOR


#if DEVICE_ID == 1 // TBEAM-01 Device EU ID = DE00000000000010
#include "device_01.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 2 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_02.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 3 //
#include "device_03.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == 4 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_04.h"
#include "../src/hal/ttgobeam10.h"
#endif

#if DEVICE_ID == DEVICE_CAMERA_01 // TBEAM-02 Device EU ID = DE00000000000011
#include "device_cam_01.h"
#include "../src/hal/ttgoCameraPlus.h"
#endif

#if DEVICE_ID == DEVICE_SOIL_SENSOR  // TBEAM-01 Device EU ID = DE00000000000010
#include "soil_sensor.h"
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
#include <AnalogSmooth.h>
#include "databuffer.h"

#if (USE_WEBSERVER || USE_CAYENNE || USE_MQTT || USE_WIFI)
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

#if (HAS_INA3221 || HAS_INA219 || USE_BME280)
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

#if (USE_HCSR04)        // Ultrasonic distance sensor
#include "sensor_hcsr04.h"
#endif

#if (USE_I2C_MICROPHONE)
#include "sensor_sound.h"
#endif

#if (USE_PWM_SERVO)
#include "servo.h"
#endif

#endif