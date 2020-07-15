
#include <LinkedList.h>
#include "ble_scanner.h"

BLECoronaDevice_t node;

int scanTime = 30; //In seconds
uint8_t BleDeviceCount = 0;
BLEScan *pBLEScan;
LinkedList<BLECoronaDevice_t> myList = LinkedList<BLECoronaDevice_t>();


void print(BLECoronaDevice_t r)
{
  Serial.print(r.Address);
  Serial.print("  ");
  Serial.print(r.Rssi);
  Serial.print("  ");
  Serial.print(r.millis);
  Serial.print("  ");
  Serial.print(r.alive);
}


void printDeviceList()
{
  Serial.println();
  Serial.println(" ---------- Device List --------- ");
  Serial.printf("Count: %i Time Now %i \n", myList.size(), millis());
  for (int i = 0; i < myList.size(); i++)
  {
    print(myList[i]);
    Serial.println();
  }
}

void cleanDeviceList()
{

  if (millis() > 6000)
  {
    for (int i = 0; i < myList.size(); i++)
    {
      if (myList[i].millis < (millis() - 40000))
      {
        myList[i].alive = 0;
      };
    }
  }
}


uint8_t getBleDeviceCount()
{
    return BleDeviceCount;
}

uint8_t getCoronaDeviceCount()
{
uint8_t count = 0;
  
  for (int i = 0; i < myList.size(); i++)
    {
      if (myList[i].alive == 1) 
        {
            count++;
        }
    }
  return count;
}


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {

    String address;

    // The remote service we wish to connect to.
    static BLEUUID Corona_serviceUUID("0000fd6f-0000-1000-8000-00805f9b34fb");
    unsigned long time_now;

    Serial.println();
    Serial.println("-------Advertising received --------");
    Serial.printf("%s \n", advertisedDevice.toString().c_str());
    // Serial.println(advertisedDevice.getServiceDataUUID().toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(Corona_serviceUUID))
    {

      time_now = millis();
      address = advertisedDevice.getAddress().toString().c_str();
      Serial.printf("Corona App found --> Address: %s   Rssi: %i \n", address.c_str(), advertisedDevice.getRSSI());
   
      node.Address  = address;
      node.Rssi     = advertisedDevice.getRSSI();
      node.millis   = time_now;
      node.alive    = 1;

      bool device_found = false;
      int i = myList.size();

      // Check if device already exists in the list
      // Yes --> Update last heard / millis
      // No --> Apend new device to list
      while (!device_found && i > 0)
      {
        if (myList[i - 1].Address == address)
        {
          Serial.println("Ping");
          myList[i - 1].millis = time_now;
          myList[i - 1].alive = 1;
          device_found = true;
        }
        i--;
      }
      
      if (!device_found)
      {
        myList.add(node);
        Serial.println("Device added.");
      }
    }
  }
};




void ble_setup()
{
  
  Serial.println("BLE Scanner");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}
void ble_loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  BleDeviceCount = foundDevices.getCount();

  Serial.println();
  Serial.println("-------BLE scanning --------");
  Serial.print("Devices found: ");
  Serial.println(BleDeviceCount);
 
  for (int i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    Serial.printf("%s Rssi: %i \n", device.toString().c_str(), device.getRSSI());
  }

  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

  cleanDeviceList();
  printDeviceList();
}