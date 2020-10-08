#ifndef _BLESCANNER_H
#define _BLESCANNER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

typedef struct
{
  String Address;
  int8_t Rssi;
  unsigned long millis;
  uint8_t alive;
} BLECoronaDevice_t;

void ble_setup();
void ble_loop();
uint8_t getCoronaDeviceCount();
uint8_t getBleDeviceCount();

#endif