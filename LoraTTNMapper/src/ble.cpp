/*
#include "globals.h"
#include "ble.h"


BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
uint8_t i;
const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.


BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic RxCountCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor RxCountDescriptor(BLEUUID((uint16_t)0x2901));


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();
        // Do stuff based on the command received from the app
        
      }
    }
};



void setup_ble() {

  // Create the BLE Device
  BLEDevice::init("T-BEAM-01 BLE"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  //-------------------------------------------------------------------
  // Service 1
  //-------------------------------------------------------------------
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);                      
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());  
  pService->start();
    
  
  //-------------------------------------------------------------------
  // Service 2
  //-------------------------------------------------------------------  
  BLEService *pBattery = pServer->createService(BatteryService);
  pBattery->addCharacteristic(&BatteryLevelCharacteristic);
  BatteryLevelDescriptor.setValue("Percentage 0 - 100");
  BatteryLevelCharacteristic.addDescriptor(&BatteryLevelDescriptor);
  BatteryLevelCharacteristic.addDescriptor(new BLE2902());

  pServer->getAdvertising()->addServiceUUID(BatteryService);
  pBattery->start(); 

//-------------------------------------------------------------------
  // Lora Service
  //-------------------------------------------------------------------

  BLEService *pLora = pServer->createService(LORAService);
  pLora->addCharacteristic(&RxCountCharacteristic);
  RxCountDescriptor.setValue("Receiveded ");
  RxCountCharacteristic.addDescriptor(&RxCountDescriptor);
  RxCountCharacteristic.addDescriptor(new BLE2902());

  pServer->getAdvertising()->addServiceUUID(LORAService);
  pLora->start(); 

  
  //-------------------------------------------------------------------
  // BLE Server starten
  //-------------------------------------------------------------------  
  pServer->getAdvertising()->start();


  Serial.println("Waiting for BLE Client...");
}


void ble_send(){
if (deviceConnected) {
    
//    pCharacteristic->setValue(&txValue, 1); // To send the integer value
       pCharacteristic->setValue("Hello!"); // Sending a test message
       pCharacteristic->setValue(dataBuffer.data.firmware_version);
    
    pCharacteristic->notify(); // Send the value to the app!
    Serial.print("BLE Device connected");  


    i = dataBuffer.data.aliveCounter;
    BatteryLevelCharacteristic.setValue(&i, 1);
    BatteryLevelCharacteristic.notify();

    i = dataBuffer.data.rxCounter;
    RxCountCharacteristic.setValue(&i, 1);
    RxCountCharacteristic.notify();
    
  }
}
*/
