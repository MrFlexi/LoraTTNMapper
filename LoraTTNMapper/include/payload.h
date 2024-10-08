#pragma once

#include "globals.h"
#include "databuffer.h"
#include "gps.h"

extern void SendPayload(uint8_t port);
extern void lora_queue_init(void);
void AXP192_powerevent_IRQ(void);

#define LPP_GPS_CHANNEL 20
#define LPP_COUNT_WIFI_CHANNEL 21
#define LPP_COUNT_BLE_CHANNEL 22
#define LPP_BATT_CHANNEL 23
#define LPP_BUTTON_CHANNEL 24
#define LPP_ADR_CHANNEL 25
#define LPP_TEMPERATURE_CHANNEL 26
#define LPP_ALARM_CHANNEL 27
#define LPP_MSG_CHANNEL 28
#define LPP_HUMIDITY_CHANNEL 29
#define LPP_BAROMETER_CHANNEL 30
#define LPP_AIR_CHANNEL 31
#define LPP_BOOTCOUNT_CHANNEL 40
#define LPP_FIRMWARE_CHANNEL 99
#define LPP_SOIL_CHANNEL1    101


// MyDevices CayenneLPP 2.0 types for Packed Sensor Payload, not using channels,
// but different FPorts
#define LPP_GPS 136          // 3 byte lon/lat 0.0001 °, 3 bytes alt 0.01m
#define LPP_TEMPERATURE 103  // 2 bytes, 0.1°C signed MSB
#define LPP_DIGITAL_INPUT 0  // 1 byte
#define LPP_DIGITAL_OUTPUT 1 // 1 byte
#define LPP_ANALOG_INPUT 2   // 2 bytes, 0.01 signed
#define LPP_LUMINOSITY 101   // 2 bytes, 1 lux unsigned
#define LPP_PRESENCE 102     //	1 byte
#define LPP_HUMIDITY 104     // 1 byte, 0.5 % unsigned
#define LPP_BAROMETER 115    // 2 bytes, hPa unsigned MSB

#define LPP_PMU 201           // 12 bytes PMU values
#define LPP_SOIL_MOISTURE 202
#define LPP_BME 203           // 4 bytes
#define LPP_DEVICE 204        // 2 byte metadata, 4 bytes payload   device info
#define LPP_HCSR04_DISTANCE   205        // 2 byte metadata, 2 bytes payload   device info
#define LPP_WIFI_COUNT   206  // 2 byte metadata, 2 bytes payload 

class PayloadConvert {

public:
  PayloadConvert(uint8_t size);
  ~PayloadConvert();

  void enqueue_port(uint8_t port);
  void reset(void);
  uint8_t getSize(void);
  uint8_t *getBuffer(void);  
  void addFloat(uint8_t channel, float value);
  void addFloatN(uint8_t channel, uint8_t sensor, float value);
  void addCount(uint8_t channel, uint16_t value);
  void addStatus(uint16_t voltage, uint64_t uptime, float cputemp, uint32_t mem,
                 uint8_t reset1, uint8_t reset2);
  void addAlarm(int8_t rssi, uint8_t message);
  void addVoltage(uint8_t channel, float value);
  void addCurrent(uint8_t channel, float value);
  void addTemperature(uint8_t channel, float value);
  void addBMETemp(uint8_t channel);
  void addDeviceData(uint8_t channel);
  void addWifiCount(uint8_t channel);
  void addPMU(uint8_t channel);
  void addGPS_TTN(TinyGPSPlus tGps);
  void addGPS_LPP(uint8_t channel, TinyGPSPlus tGps); 
  void addButton(uint8_t value);
  void addSensor(uint8_t[]);
  void addTime(time_t value);

private:
  uint8_t *buffer;
  uint8_t maxsize;
  uint8_t cursor;
};

extern PayloadConvert payload;
