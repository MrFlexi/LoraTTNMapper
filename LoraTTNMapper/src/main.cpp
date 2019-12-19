// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 14
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

#define displayRefreshIntervall 5    // every x second
#define sendMessagesIntervall 90     // every x seconds
#define LORAsendMessagesIntervall 60 // every x seconds

//const float sleepPeriod = 2; //seconds
#define SEALEVELPRESSURE_HPA (1013.25)

char buf[32];

#include "globals.h"

//--------------------------------------------------------------------------
// OTA Settings
//--------------------------------------------------------------------------
#include "SecureOTA.h"
const uint16_t OTA_CHECK_INTERVAL = 3000; // ms
uint32_t _lastOTACheck = 0;
bool wifi_connected = false;

//--------------------------------------------------------------------------
// Wifi Settings
//--------------------------------------------------------------------------

//const char ssid[] = "MrFlexi";
//const char wifiPassword[] = "Linde-123";
//WiFiClient wifiClient;

//--------------------------------------------------------------------------
// Initialize globals
//--------------------------------------------------------------------------
PayloadConvert payload(PAYLOAD_BUFFER_SIZE);

SemaphoreHandle_t I2Caccess;
int runmode = 0;

uint8_t msgWaiting = 0;

//--------------------------------------------------------------------------
// Tasks/Ticker
//--------------------------------------------------------------------------

Ticker sleepTicker;
Ticker displayTicker;
Ticker sendMessageTicker;
Ticker LORAsendMessageTicker;

//--------------------------------------------------------------------------
// Sensors
//--------------------------------------------------------------------------

#if (USE_BME280)
Adafruit_BME280 bme; // I2C   PIN 21 + 22
#endif

//--------------------------------------------------------------------------
// Cayenne MyDevices Integration
//--------------------------------------------------------------------------

#if (USE_CAYENNE)
#define CAYENNE_PRINT Serial

#include <CayenneMQTTESP32.h>
char username[] = "ecddac20-a0eb-11e9-94e9-493d67fd755e";
char password[] = "0010d05f8ccd918d0f8a45451950f8b80200e594";
char clientID[] = "44257070-b074-11e9-80af-177b80d8d7b2"; // DE001-Balkon

CAYENNE_CONNECTED()
{
  Serial.println("Cayenne connected...");
}

CAYENNE_DISCONNECTED()
{
  Serial.println("Cayenne connection lost...");
  bool disconnected = true;
  while (disconnected)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Wifi is back...");
      disconnected = false;
    }
    else
    {
      Serial.println("No wifi...");
    }
    delay(2000);
  }
}

CAYENNE_OUT_DEFAULT()
{
}

void Cayenne_send(void)
{

  ESP_LOGI(TAG, "Cayenne send data");

  Cayenne.celsiusWrite(1, dataBuffer.data.temperature);
  Cayenne.virtualWrite(2, dataBuffer.data.humidity, "rel_hum", "p");

  Cayenne.virtualWrite(10, dataBuffer.data.panel_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(12, dataBuffer.data.panel_current, "current", "Milliampere");
  Cayenne.virtualWrite(20, dataBuffer.data.bus_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(21, dataBuffer.data.bus_current, "current", "Milliampere");

  Cayenne.virtualWrite(30, dataBuffer.data.bat_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(31, dataBuffer.data.bat_charge_current, "current", "Milliampere");

  Cayenne.virtualWrite(33, dataBuffer.data.bat_discharge_current, "current", "Milliampere");
  Cayenne.virtualWrite(40, dataBuffer.data.LoraQueueCounter, "counter", "Counter");
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  ESP_LOGI(TAG, "Cayenne data received");
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

#endif

//--------------------------------------------------------------------------
// Store preferences in NVS Flash
//--------------------------------------------------------------------------
//Preferences preferences;
char lastword[10];

unsigned long uptime_seconds_old;
unsigned long uptime_seconds_new;
unsigned long uptime_seconds_actual;

String stringOne = "";

static const char TAG[] = __FILE__;

void display_chip_info()
{
  // print chip information on startup if in verbose mode after coldstart

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  ESP_LOGI(TAG,
           "This is ESP32 chip with %d CPU cores, WiFi%s%s, silicon revision "
           "%d, %dMB %s Flash",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
           chip_info.revision, spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                         : "external");
  ESP_LOGI(TAG, "Internal Total heap %d, internal Free Heap %d",
           ESP.getHeapSize(), ESP.getFreeHeap());
#ifdef BOARD_HAS_PSRAM
  ESP_LOGI(TAG, "SPIRam Total heap %d, SPIRam Free Heap %d",
           ESP.getPsramSize(), ESP.getFreePsram());
#endif
  ESP_LOGI(TAG, "ChipRevision %d, Cpu Freq %d, SDK Version %s",
           ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
  ESP_LOGI(TAG, "Flash Size %d, Flash Speed %d", ESP.getFlashChipSize(),
           ESP.getFlashChipSpeed());

#if (HAS_LORA)
  ESP_LOGI(TAG, "IBM LMIC version %d.%d.%d", LMIC_VERSION_MAJOR,
           LMIC_VERSION_MINOR, LMIC_VERSION_BUILD);
  ESP_LOGI(TAG, "Arduino LMIC version %d.%d.%d.%d",
           ARDUINO_LMIC_VERSION_GET_MAJOR(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_MINOR(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_PATCH(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_LOCAL(ARDUINO_LMIC_VERSION));
#endif // HAS_LORA

#if (HAS_GPS)
  ESP_LOGI(TAG, "TinyGPS+ version %s", TinyGPSPlus::libraryVersion());
#endif
}

#if (HAS_INA)
void print_ina()
{
  Serial.println("");
  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;

  busvoltage1 = ina3221.getBusVoltage_V(1);
  shuntvoltage1 = ina3221.getShuntVoltage_mV(1);
  current_mA1 = -ina3221.getCurrent_mA(1); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

  Serial.print("Bus Voltage:");
  Serial.print(busvoltage1);
  Serial.println(" V");
  Serial.print("Shunt Voltage:");
  Serial.print(shuntvoltage1);
  Serial.println(" mV");
  Serial.print("Battery Load Voltage:");
  Serial.print(loadvoltage1);
  Serial.println(" V");
  Serial.print("Battery Current 1:");
  Serial.print(current_mA1);
  Serial.println(" mA");
  Serial.println("");
}
#endif

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.print(F("WakeUp caused by: "));
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println(F("external signal using RTC_IO"));
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println(F("external signal using RTC_CNTL"));
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println(F("touchpad"));
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println(F("ULP program"));
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void setup_sensors()
{
#if (USE_BME280)
  ESP_LOGI(TAG, "BME280 Setup...");
  unsigned status;

  status = bme.begin(0x76);
  if (!status)
  {
    ESP_LOGI(TAG, "Could not find a valid BME280 sensor");
  }
  else
  {
    Serial.println();
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    Serial.println();
  }
#endif
}

void t_cyclic()
{
  String stringOne;

// Temperatur
#if (USE_BME280)
  dataBuffer.data.temperature = bme.readTemperature();
  dataBuffer.data.humidity = bme.readHumidity();
  ESP_LOGI(TAG, "BME280  %.1f C/%.1f%", dataBuffer.data.temperature, dataBuffer.data.humidity);
#endif

#if (HAS_PMU)
  dataBuffer.data.bus_voltage = pmu.getVbusVoltage() / 1000;
  dataBuffer.data.bus_current = pmu.getVbusCurrent();

  dataBuffer.data.bat_voltage = pmu.getBattVoltage() / 1000;
  dataBuffer.data.bat_charge_current = pmu.getBattChargeCurrent();
  dataBuffer.data.bat_discharge_current = pmu.getBattDischargeCurrent();
  // AXP192_showstatus();
#endif

#if (HAS_INA)
  //print_ina();
  dataBuffer.data.panel_voltage = ina3221.getBusVoltage_V(1);
  dataBuffer.data.panel_current = ina3221.getCurrent_mA(1);
#endif

#if (HAS_LORA)
  if (LoraSendQueue != 0)
  {
    dataBuffer.data.LoraQueueCounter = uxQueueMessagesWaiting(LoraSendQueue);
  }
  else
  {
    dataBuffer.data.LoraQueueCounter = 0;
  }
#endif

  gps.encode();
  gps.checkGpsFix();

// Refresh Display
#if (USE_DISPLAY)
  showPage(PageNumber);
#endif

#if (USE_DASH)
  if (WiFi.status() == WL_CONNECTED)
    update_web_dash();
#endif

#if ( USE_SERIAL_BT)
  sprintf(buf, "Lora Queue:", dataBuffer.data.LoraQueueCounter);
  log_display(buf);

  sprintf(buf, "Firmware:", VERSION);
  log_display(buf);
#endif

}

void t_sleep()
{
  //-----------------------------------------------------
  // Deep sleep
  //-----------------------------------------------------

#if (ESP_SLEEP)
  dataBuffer.data.sleepCounter--;
  if (dataBuffer.data.sleepCounter <= 0 || dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT)
  {
#if (HAS_PMU)
    AXP192_power_gps(OFF);
    AXP192_power_lora(OFF);
#endif
    delay(1000);
    t_cyclic(); // Aktuelle Messwerte anzeigen
    delay(5000);
    runmode = 0;
    // gps.enable_sleep();
    Serial.flush();
    showPage(PAGE_SLEEP);

    //AXP192_power(OFF);   // funktioniert nicht, I2C Bus wird nicht freigegeben
    //delay(100);
    //Wire.flush();
    //Wire.~TwoWire();
    //pinMode(SDA, INPUT); // needed because Wire.end() enables pullups, power Saving
    //pinMode(SCL, INPUT);

    log_display("ESP32 Deep Sleep started");
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }
#endif
}

void setup_wifi()
{

#if (USE_WIFI)
  // WIFI Setup
  WiFi.begin(ssid, wifiPassword);

  ESP_LOGI(TAG, "Connecting to WiFi..");
  int i = 0;
  wifi_connected = false;
  while ((WiFi.status() != WL_CONNECTED) && (i < 10))
  {
    delay(1000);
    i++;
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_connected = true;
    ESP_LOGV(TAG, String(WiFi.localIP()));
    log_display(String(WiFi.localIP()));
    delay(2000);
  }
  else
  {

    //Turn off WiFi if no connection was made
    log_display("WIFI OFF");
    WiFi.mode(WIFI_OFF);
  }

#endif
}

#if (USE_BLE)
void setup_BLE()
{
  Serial.println("Setup BLE");

  BLEDevice::init("SolarServer");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("SAP GTT");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("SAP GTT Monitor");
}
#endif

void setup()
{
  Serial.begin(115200);

#if (USE_SERIAL_BT)
  setup_serial_bt();
#endif

  display_chip_info();
  print_wakeup_reason();

  // create some semaphores for syncing / mutexing tasks
  I2Caccess = xSemaphoreCreateMutex(); // for access management of i2c bus
  assert(I2Caccess != NULL);
  I2C_MUTEX_UNLOCK();

  //---------------------------------------------------------------
  // Get preferences from Flash
  //---------------------------------------------------------------
  //preferences.begin("config", false); // NVS Flash RW mode
  //preferences.getULong("uptime", uptime_seconds_old);
  //Serial.println("Uptime old: " + String(uptime_seconds_old));
  //preferences.getString("info", lastword, sizeof(lastword));

  ESP_LOGI(TAG, "Starting..");
  Serial.println(F("TTN Mapper"));
  i2c_scan();

#if (HAS_PMU)
  AXP192_init();
  AXP192_showstatus();
  AXP192_power_gps(ON);
#endif

#if (HAS_INA)
  ina3221.begin();
  Serial.print("Manufact. ID=0x");
  int MID;
  MID = ina3221.getManufID();
  Serial.println(MID, HEX);
  print_ina();
#endif

  dataBuffer.data.txCounter = 0;
  dataBuffer.data.sleepCounter = TIME_TO_NEXT_SLEEP;
  dataBuffer.data.firmware_version = VERSION;

#if (USE_DISPLAY)
  setup_display();
#endif

  setup_sensors();
  setup_wifi();
  calibrate_voltage();

  //Turn off Bluetooth
  //log_display("Stop Bluethooth");
  //btStop();

#if (USE_MQTT)
  setup_mqtt();
#endif

#if (USE_CAYENNE)
  if (WiFi.status() == WL_CONNECTED)
  {
    Cayenne.begin(username, password, clientID, ssid, wifiPassword);
    log_display("Cayenne connected...");
  }
#endif

  //---------------------------------------------------------------
  // OTA Update
  //---------------------------------------------------------------

#if (USE_OTA)
  if (WiFi.status() == WL_CONNECTED)
  {
    _lastOTACheck = millis();
    checkFirmwareUpdates();
    delay(1000);
  }
#endif

//---------------------------------------------------------------
// Deep sleep settings
//---------------------------------------------------------------
#if (ESP_SLEEP)
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
  log_display("ESP32 wake-up timer " + String(TIME_TO_SLEEP) +
              " min");
#endif

  gps.init();
  //gps.softwareReset();
  gps.wakeup();
  //gps.ecoMode();

  delay(2000); // Wait for GPS beeing stable

#if (USE_BLE)
  setup_BLE();
#endif

#if (HAS_LORA)
  setup_lora();
  lora_queue_init();
#endif

#if (USE_DASH)
  if (WiFi.status() == WL_CONNECTED)
  {
    create_web_dash();
  }
#endif

#ifdef HAS_BUTTON
  button_init(HAS_BUTTON);
#endif

  // Tasks
  log_display("-- Starting Tasks --");

  sleepTicker.attach(60, t_sleep);
  displayTicker.attach(displayRefreshIntervall, t_cyclic);
  sendMessageTicker.attach(sendMessagesIntervall, t_enqueue_LORA_messages);

  log_display("-- Setup done --");

  runmode = 1; // Switch from Terminal Mode to page Display
#if (USE_DISPLAY)
  showPage(1);
#endif

  //-------------------------------------------------------------
  // Call all tasks once after startup, next call via Timer
  //-------------------------------------------------------------
  t_cyclic();

#if (USE_CAYENNE)
  if (WiFi.status() == WL_CONNECTED)
    Cayenne_send();
#endif

#if (HAS_LORA)
  t_enqueue_LORA_messages();
#endif
}

void loop()
{
#if (HAS_LORA)
  os_runloop_once();
#endif

#if (USE_CAYENNE)
  if (WiFi.status() == WL_CONNECTED)
  {
    Cayenne.loop();
  }
#endif

#if (USE_MQTT)
  // MQTT Connection
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
  }
#endif

#ifdef HAS_BUTTON
  readButton();
#endif
}