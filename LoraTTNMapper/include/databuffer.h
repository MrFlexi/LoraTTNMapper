//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <globals.h>
#include <gps.h>

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
  uint8_t servo1;
  uint8_t servo2;
  double sun_azimuth;
  double sun_elevation;
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
  String image_url = "/get_last_detected";
  uint8_t operation_mode = 0;
  esp_sleep_wakeup_cause_t wakeup_reason;
  float distance = 0;
  bool distance_changed;
  TinyGPSLocation gps;
  TinyGPSLocation gps_old;
  double gps_distance;
  char gps_datetime[32];
  tm timeinfo;
  bool ble_device_connected = false;
#if (USE_CAMERA)
  String image_buffer = "";
#endif
} deviceStatus_t;


typedef struct
{
  uint8_t sleep_time;
  bool sunTrackerPositionAdjusted = false;
  const char* log_print_buffer;
  const char* experiment;
  uint8_t bat_max_charge_current;  
} deviceSettings_t;

class DataBuffer
{
  public:
    DataBuffer();
    void set( deviceStatus_t input );
    void get();
    String to_json();
    String to_json_web();
    deviceStatus_t data ;
    deviceSettings_t settings;
    const char* getError() const { return _error; }
  private:
    char* _error;   
};

extern DataBuffer dataBuffer;