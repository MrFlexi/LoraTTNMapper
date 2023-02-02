//--------------------------------------------------------------------------
// BLE Server
//--------------------------------------------------------------------------
#pragma once

#include <Arduino.h>
#include "globals.h"

#if (USE_BLE_SERVER)
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//BLE server name
#define bleServerName "ESP32_02"

void setup_ble();
void ble_send();
#endif