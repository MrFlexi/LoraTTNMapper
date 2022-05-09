#include <Arduino.h>
#include "globals.h"

bool deviceConnected = false;
#define envService BLEUUID((uint16_t)0x181A)
#define BatteryService BLEUUID((uint16_t)0x180F)

// Temperature Characteristic and Descriptor

BLECharacteristic bmeTemperatureCharacteristics(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic BatteryChargeCharacteristic("c530390d-cb2a-46c3-87c4-2f302a2f371e", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BatteryChargeDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic SunAzimuthCharacteristic("738be241-bccb-47d0-9149-ef3024d4324c", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor SunAzimuthDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic SunElevationCharacteristic("e20ce7ec-4ed7-40f4-b149-c9a209e21e92", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor SunElevationDescriptor(BLEUUID((uint16_t)0x2901));

int n = 0;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    ESP_LOGI(TAG, "BLE device connected");
  };
  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pServer->startAdvertising();
    ESP_LOGI(TAG, "BLE device disconnected");
  }
};

// Create the BLE Device
void setup_ble()
{
  Serial.println("BLE init...");

  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  //--------------------------------------------------------------------------------------
  // Environmental Service
  //--------------------------------------------------------------------------------------
  BLEService *bmeService = pServer->createService(envService);

  // Temperature
  bmeService->addCharacteristic(&bmeTemperatureCharacteristics);
  bmeTemperatureDescriptor.setValue("BME temperature Celsius");
  bmeTemperatureCharacteristics.addDescriptor(&bmeTemperatureDescriptor);
  bmeTemperatureCharacteristics.addDescriptor(new BLE2902());

  // Humidity
  bmeService->addCharacteristic(&bmeHumidityCharacteristics);
  bmeHumidityDescriptor.setValue("BME humidity 0-100%");
  bmeHumidityCharacteristics.addDescriptor(&bmeHumidityDescriptor);
  bmeHumidityCharacteristics.addDescriptor(new BLE2902());

  // Sun azimuth
  bmeService->addCharacteristic(&SunAzimuthCharacteristic);
  SunAzimuthDescriptor.setValue("Sun Azimuth");
  SunAzimuthCharacteristic.addDescriptor(&SunAzimuthDescriptor);
  SunAzimuthCharacteristic.addDescriptor(new BLE2902());

  // Sun elevation
  bmeService->addCharacteristic(&SunElevationCharacteristic);
  SunElevationDescriptor.setValue("Sun Elevation");
  SunElevationCharacteristic.addDescriptor(&SunElevationDescriptor);
  SunElevationCharacteristic.addDescriptor(new BLE2902());

  //--------------------------------------------------------------------------------------
  // Battery Service
  //--------------------------------------------------------------------------------------
  BLEService *pBattery = pServer->createService(BatteryService);

  pBattery->addCharacteristic(&BatteryLevelCharacteristic);
  BatteryLevelDescriptor.setValue("Percentage 0 - 100");
  BatteryLevelCharacteristic.addDescriptor(&BatteryLevelDescriptor);
  BatteryLevelCharacteristic.addDescriptor(new BLE2902());

  pBattery->addCharacteristic(&BatteryChargeCharacteristic);
  BatteryChargeDescriptor.setValue("Charge/Discharge in mA");
  BatteryChargeCharacteristic.addDescriptor(&BatteryChargeDescriptor);
  BatteryChargeCharacteristic.addDescriptor(new BLE2902());

 

  pServer->getAdvertising()->addServiceUUID(BatteryService);
  pBattery->start();

  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(envService);

  pServer->getAdvertising()->addServiceUUID(envService);
  pServer->getAdvertising()->start();

  ESP_LOGI(TAG, "BLE waiting for client to connect...");
}

void ble_send()
{
  if (deviceConnected)
  {
    ESP_LOGI(TAG, "BLE notify start");

    // Battery Service
    BatteryLevelCharacteristic.setValue(n);
    BatteryLevelCharacteristic.notify();

    int32_t current = dataBuffer.data.bat_charge_current;
    BatteryChargeCharacteristic.setValue(current);
    BatteryChargeCharacteristic.notify();

    // Environmental Service
    bmeTemperatureCharacteristics.setValue(n);
    bmeTemperatureCharacteristics.notify();

    bmeHumidityCharacteristics.setValue(n);
    bmeHumidityCharacteristics.notify();

    SunAzimuthCharacteristic.setValue(dataBuffer.data.sun_azimuth);
    SunAzimuthCharacteristic.notify();

    SunElevationCharacteristic.setValue(dataBuffer.data.sun_elevation);
    SunElevationCharacteristic.notify();
    n++;
  }
  else
  {
    ESP_LOGI(TAG, "BLE no device connected");
  }
}