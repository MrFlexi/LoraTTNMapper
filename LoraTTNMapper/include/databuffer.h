//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <globals.h>
#include <gps.h>


typedef struct
{
  int	day;
  int	month;
  int	year; 
} ty_mytime;

#if (HAS_PMU)
typedef struct
{
  int pmu_charge_setting;
  float panel_voltage = 0;
  float panel_current = 0;
  float bus_voltage = 0;
  uint16_t bus_current = 0;
  float bat_voltage = 0;
  uint16_t bat_charge_current = 0;
  uint16_t bat_charge_power = 0;
} ty_mpp;
#endif

#if (HAS_INA219)
typedef struct
{
  float voltage = 0;
  float shuntVoltage = 0;
  uint16_t current = 0;
  uint16_t power = 0;
} ty_ina219;
#endif

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
  uint8_t MotionCounter;    // aliveCounter
  uint16_t bootCounter;
  uint8_t txCounter; // aliveCounter
  uint8_t rxCounter; // aliveCounter
  uint8_t runmode;   // aliveCounter
  uint8_t CoronaDeviceCount;
  uint32_t freeheap;        // free memory
  uint8_t tx_ack_req;       // request TTN to acknowlede a TX
  uint16_t potentiometer_a; //
  uint16_t adc0;            // Voltage in mVolt
  bool potentiometer_a_changed;

  uint8_t bat_max_charge_curr = 0;

#if (HAS_PMU)
  int mpp_max_charge_setting = 0;
  float mpp_max_bat_charge_power = 0;
  tm mpp_last_timeinfo;
  ty_mpp mpp_values[13];
  float bus_voltage = 0;
  float bus_current = 0;
  uint32_t bat_ChargeCoulomb = 0;
  uint32_t bat_DischargeCoulomb = 0;
  float bat_DeltamAh = 0;

  float bat_charge_current = 0;
  float bat_discharge_current = 0;
#endif
  float bat_voltage = 0;
  bool wlan;
  bool pictureLoop = true;
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

  float soil_moisture = 0;
  float yaw = 0;
  float pitch = 0;
  float roll = 0;
  uint16_t LidarDistanceMM = 0;
  String ip_address;
  String image_url = "/get_last_detected";
  uint8_t operation_mode = 0;
  esp_sleep_wakeup_cause_t wakeup_reason;
  uint16_t hcsr04_distance = 0; // in cm
  bool distance_changed;
  TinyGPSLocation gps;
  TinyGPSLocation gps_old;
  double gps_distance;
  char gps_datetime[32];
  tm timeinfo;
  ty_mytime time;
  bool ble_device_connected = false;
  uint16_t wifi_count;
  uint16_t wifi_count5;
  bool wificounter_active = false;
#if (HAS_INA219)
  ty_ina219 ina219[2];
#endif

#if (USE_CAMERA)
  String image_buffer = "";
#endif
} deviceStatus_t;

typedef struct
{
  uint8_t sleep_time;
  bool sunTrackerPositionAdjusted = false;
  const char *log_print_buffer;
  const char *experiment;
  uint8_t bat_max_charge_current;
} deviceSettings_t;

class DataBuffer
{
public:
  DataBuffer();
  void set(deviceStatus_t input);
  void get();
  String to_json();
  String to_json_web();
  deviceStatus_t data;
  deviceSettings_t settings;
  const char *getError() const { return _error; }

private:
  char *_error;
};

extern DataBuffer dataBuffer;