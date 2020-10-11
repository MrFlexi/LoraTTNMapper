// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 14
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

#define SEALEVELPRESSURE_HPA (1013.25)

#include "globals.h"

// Defaults to window size 10
#if (USE_POTI)
AnalogSmooth Poti_A = AnalogSmooth();
#endif

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

#if (USE_WEBSERVER || USE_CAYENNE || USE_MQTT || USE_WIFI)
WiFiClient wifiClient;
#endif

//--------------------------------------------------------------------------
// Lora Helper
//--------------------------------------------------------------------------
const char *getSfName(rps_t rps)
{
  const char *const t[] = {"FSK", "SF7", "SF8", "SF9",
                           "SF10", "SF11", "SF12", "SF?"};
  return t[getSf(rps)];
}

const char *getBwName(rps_t rps)
{
  const char *const t[] = {"BW125", "BW250", "BW500", "BW?"};
  return t[getBw(rps)];
}

const char *getCrName(rps_t rps)
{
  const char *const t[] = {"CR 4/5", "CR 4/6", "CR 4/7", "CR 4/8"};
  return t[getCr(rps)];
}

//--------------------------------------------------------------------------
// Initialize globals
//--------------------------------------------------------------------------
PayloadConvert payload(PAYLOAD_BUFFER_SIZE);

SemaphoreHandle_t I2Caccess;

uint8_t msgWaiting = 0;

RTC_DATA_ATTR uint16_t bootCount = 0;
touch_pad_t touchPin;

//--------------------------------------------------------------------------
// Tasks/Ticker
//--------------------------------------------------------------------------

TaskHandle_t irqHandlerTask = NULL;
TaskHandle_t task_broadcast_message = NULL;
TaskHandle_t moveDisplayHandlerTask = NULL;
TaskHandle_t t_cyclic_HandlerTask = NULL;

Ticker sleepTicker;
Ticker displayTicker;
Ticker displayMoveTicker;
Ticker sendMessageTicker;
Ticker sendCycleTicker;
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
  log_display("Cayenne connected...");
}

CAYENNE_DISCONNECTED()
{
  log_display("Cayenne connection lost...");
  bool disconnected = true;
  while (disconnected)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      log_display("Wifi is back...");
      disconnected = false;
    }
    else
    {
      log_display("No wifi...");
    }
    delay(100);
  }
}

CAYENNE_OUT_DEFAULT()
{
}

void Cayenne_send(void)
{

  log_display("Cayenne send");

  Cayenne.celsiusWrite(1, dataBuffer.data.temperature);
  //Cayenne.virtualWrite(11, dataBuffer.data.gps.lat(), dataBuffer.data.gps.lng(),dataBuffer.data.gps.tGps.altitude.meters(),"gps","m");
  Cayenne.virtualWrite(2, dataBuffer.data.humidity, "rel_hum", "p");

  Cayenne.virtualWrite(10, dataBuffer.data.panel_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(12, dataBuffer.data.panel_current, "current", "Milliampere");

  Cayenne.virtualWrite(20, dataBuffer.data.bus_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(21, dataBuffer.data.bus_current, "current", "Milliampere");

  Cayenne.virtualWrite(30, dataBuffer.data.bat_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(31, dataBuffer.data.bat_charge_current, "current", "Milliampere");
  Cayenne.virtualWrite(32, dataBuffer.data.bat_discharge_current, "current", "Milliampere");
  Cayenne.virtualWrite(33, dataBuffer.data.bat_DeltamAh, "current", "Milliampere");

  Cayenne.virtualWrite(40, dataBuffer.data.bootCounter, "counter", "Analog");
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  log_display("Cayenne data received");
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
  switch (request.channel)
  {
  case 1:
    Serial.println("Cayenne: Reset Coulomb Counter");
    pmu.ClearCoulombcounter();
    break;
  }
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

#if (HAS_INA)

SDL_Arduino_INA3221 ina3221;

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

void touch_callback()
{
  //placeholder callback function
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

void t_send_cycle()
{

#if (USE_CAYENNE)
  if (WiFi.status() == WL_CONNECTED)
    Cayenne_send();
#endif

#if (USE_BLE)
  ble_send();
#endif

#if (USE_DASH)
  if (WiFi.status() == WL_CONNECTED)
    update_web_dash();
#endif

#if (USE_MQTT)
  if (WiFi.status() == WL_CONNECTED)
    mqtt_send();
#endif
}

void t_cyclicRTOS(void *pvParameters)
{

  DataBuffer foo;

  while (1)
  {
#if (USE_BLE_SCANNER)
    ble_loop();

    // Werte holen
    foo = *((DataBuffer *)pvParameters);

    Serial.printf("Corona Count/Ble Count = : %i / %i \n", getCoronaDeviceCount(), getBleDeviceCount());
    foo.data.CoronaDeviceCount = getCoronaDeviceCount();

    // Werte wieder zurückschreiben
    *(DataBuffer *)pvParameters = foo;

    vTaskDelay(10000 / portTICK_PERIOD_MS);
#endif
  }
}

void t_cyclic() // Intervall: Display Refresh
{

  dataBuffer.data.freeheap = ESP.getFreeHeap();
  dataBuffer.data.cpu_temperature = ( temprature_sens_read() - 32 ) / 1.8;

  dataBuffer.data.aliveCounter++;

  #if (USE_GPS)
  gps.getDistance();
  #endif

  //   I2C opperations
  if (!I2C_MUTEX_LOCK())
    ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else
  {
#if (USE_BME280)
    dataBuffer.data.temperature = bme.readTemperature();
    dataBuffer.data.humidity = bme.readHumidity();
#endif

#if (HAS_PMU)
    dataBuffer.data.bus_voltage = pmu.getVbusVoltage() / 1000;
    dataBuffer.data.bus_current = pmu.getVbusCurrent();

    dataBuffer.data.bat_voltage = pmu.getBattVoltage() / 1000;
    dataBuffer.data.bat_charge_current = pmu.getBattChargeCurrent();
    dataBuffer.data.bat_discharge_current = pmu.getBattDischargeCurrent();
    dataBuffer.data.bat_ChargeCoulomb = pmu.getBattChargeCoulomb() / 3.6;
    dataBuffer.data.bat_DischargeCoulomb = pmu.getBattDischargeCoulomb() / 3.6;
    dataBuffer.data.bat_DeltamAh = pmu.getCoulombData();

    //ESP_LOGI(TAG, "Bat+ %d",dataBuffer.data.bat_ChargeCoulomb);
    //ESP_LOGI(TAG, "Bat- %d",dataBuffer.data.bat_DischargeCoulomb);
    //ESP_LOGI(TAG, "delta %.2f mAh", dataBuffer.data.bat_DeltamAh);

#else
    dataBuffer.data.bat_voltage = read_voltage() / 1000;
#endif

#if (HAS_INA)
    //print_ina();
    dataBuffer.data.panel_voltage = ina3221.getBusVoltage_V(1);
    dataBuffer.data.panel_current = ina3221.getCurrent_mA(1);
#endif

    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }

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

  // Refresh Display

#if (USE_DISPLAY)
  if (dataBuffer.data.runmode > 0)
    showPage(PageNumber);
#endif



#if (CYCLIC_SHOW_LOG)
  ESP_LOGI(TAG, "Runmode %d", dataBuffer.data.runmode);
  ESP_LOGI(TAG, "Poti %.2f", dataBuffer.data.potentiometer_a);
  ESP_LOGI(TAG, "BME280  %.1f C/%.1f%", dataBuffer.data.temperature, dataBuffer.data.humidity);
  #if (HAS_PMU)
  AXP192_showstatus();
  #endif
#endif


}

void t_sleep()
{
  //-----------------------------------------------------
  // Deep sleep
  //-----------------------------------------------------

  gps.getDistance();

#if (ESP_SLEEP)
  dataBuffer.data.MotionCounter = dataBuffer.data.MotionCounter - 1;

#if (USE_FASTLED)
  if (dataBuffer.data.MotionCounter < TIME_TO_NEXT_SLEEP_WITHOUT_MOTION)
  {
    LED_showSleepCounter();
  }
#endif

  if (dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT || dataBuffer.data.MotionCounter <= 0)
  {

#if (USE_GPS_MOTION)
    if (dataBuffer.data.gps_distance > GPS_MOTION_DISTANCE)
    {
      dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;
      gps.resetDistance();
    }
#endif

    if (dataBuffer.data.MotionCounter <= 0)
      ESP32_sleep();
  }
#endif
}

void setup_wifi()
{

#if (USE_WIFI)
  IPAddress ip;
  // WIFI Setup
  WiFi.begin(ssid, wifiPassword);

  ESP_LOGI(TAG, "Connecting to WiFi..");
  int i = 0;
  wifi_connected = false;
  while ((WiFi.status() != WL_CONNECTED) && (i < 50))
  {
    delay(200);
    i++;
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_connected = true;
    dataBuffer.data.wlan = true;
    ip = WiFi.localIP();
    Serial.println(ip);
    dataBuffer.data.ip_address = ip.toString();
  }
  else
  {

    //Turn off WiFi if no connection was made
    log_display("WIFI OFF");
    dataBuffer.data.wlan = false;
    WiFi.mode(WIFI_OFF);
  }

#endif
}

void createRTOStasks()
{

#if (USE_BLE_SCANNER)
  xTaskCreatePinnedToCore(t_cyclicRTOS,          // task function
                          "t_cyclic",            // name of task
                          4096,                  // stack size of task
                          (void *)&dataBuffer,   // parameter of the task
                          2,                     // priority of the task
                          &t_cyclic_HandlerTask, // task handle
                          1);                    // CPU core
#endif
}

void setup()
{

  Serial.begin(115200);
  dataBuffer.data.runmode = 0;
  Serial.println("Runmode: " + String(dataBuffer.data.runmode));

  //Increment boot number and print it every reboot
  ++bootCount;
  dataBuffer.data.bootCounter = bootCount;

  Serial.println("Boot number: " + String(bootCount));

  print_wakeup_reason();
  display_chip_info();
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

  // create some semaphores for syncing / mutexing tasks
  I2Caccess = xSemaphoreCreateMutex(); // for access management of i2c bus
  assert(I2Caccess != NULL);
  I2C_MUTEX_UNLOCK();
  delay(100);

  // Bluethooth Serial + BLE
#if (USE_SERIAL_BT)
  SerialBT.begin("T-BEAM_01"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  delay(100);
#endif

  ESP_LOGI(TAG, "Starting..");
  Serial.println(F("TTN Mapper"));
  i2c_scan();
  delay(100);

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
  dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;
  dataBuffer.data.firmware_version = VERSION;
  dataBuffer.data.tx_ack_req = 0;

  setup_display();
  setup_sensors();
  setup_wifi();
  calibrate_voltage();

#if (USE_SERIAL_BT || USE_BLE_SCANNER)
#else
  //Turn off Bluetooth
  log_display("BLUETHOOTH OFF");
  btStop();
#endif

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
  }
#endif

#if (USE_GYRO)
  setup_gyro();
#endif


#if (USE_GPS)
  gps.init();
  gps.wakeup();
  delay(50); // Wait for GPS beeing stable
#endif

#if (HAS_LORA)
  setup_lora();
  lora_queue_init();
  delay(50);
#endif

#if (USE_DASH)
  if (WiFi.status() == WL_CONNECTED)
  {
    create_web_dash();
  }
#endif

#if (USE_BUTTON)
#ifdef BUTTON_PIN
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button_init(BUTTON_PIN);
#endif
#endif

#if (USE_WEBSERVER)
  ESP_LOGI(TAG, "Mounting SPIFF Filesystem");
  // External File System Initialisation
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "An Error has occurred while mounting SPIFFS");
    return;
  }
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file)
  {

    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
  delay(100);
#endif

#if (USE_WEBSERVER)
  if (WiFi.status() == WL_CONNECTED)
  {
    server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Index requested");
      request->send(SPIFFS, "/index.html", "text/html");
    });
    server.begin();
    server.serveStatic("/", SPIFFS, "/");
  }
#endif

#if (USE_WEBSERVER)
  if (WiFi.status() == WL_CONNECTED)
  {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
  }
#endif

  // get sensor values once
  t_cyclic();

#if (USE_FASTLED)
  setup_FastLed();
  delay(50);
  LED_wakeup();
#endif

  //-------------------------------------------------------------------------------
  // Tasks
  //-------------------------------------------------------------------------------
  log_display("Starting Tasks");

  sleepTicker.attach(60, t_sleep);
  displayTicker.attach(displayRefreshIntervall, t_cyclic);
  displayMoveTicker.attach(displayMoveIntervall, t_moveDisplay);
  
  sendCycleTicker.attach(sendCycleIntervall, t_send_cycle);

#if (HAS_LORA)
  sendMessageTicker.attach(LORAenqueueMessagesIntervall, t_enqueue_LORA_messages);
#endif


#if (USE_WEBSERVER)
  if (WiFi.status() == WL_CONNECTED)
  {
    xTaskCreate(
        t_broadcast_message,      /* Task function. */
        "Broadcast Message",      /* String with name of task. */
        10000,                    /* Stack size in bytes. */
        NULL,                     /* Parameter passed as input of the task */
        10,                       /* Priority of the task. */
        &task_broadcast_message); /* Task handle. */
  }
#endif

// Interrupt ISR Handler
#if (USE_INTERRUPTS)
  ESP_LOGI(TAG, "Starting Interrupt Handler...");
  xTaskCreatePinnedToCore(irqHandler,      // task function
                          "irqhandler",    // name of task
                          4096,            // stack size of task
                          (void *)1,       // parameter of the task
                          2,               // priority of the task
                          &irqHandlerTask, // task handle
                          1);              // CPU core
#endif

#if (HAS_LORA)
  t_enqueue_LORA_messages();
#endif

#if (USE_POTI)
  poti_setup_RTOS();
#endif

#if (USE_BLE_SCANNER)
  ble_setup();
  ble_loop();
#endif

//---------------------------------------------------------------
// Deep sleep settings
//---------------------------------------------------------------
#if (ESP_SLEEP)
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
  log_display("Deep Sleep " + String(TIME_TO_SLEEP) +
              " min");

#if (USE_BUTTON)
  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0); //1 = High, 0 = Low
#endif

#if (WAKEUP_BY_MOTION)
#if (USE_GYRO)
#ifdef GYRO_INT_PIN
  esp_sleep_enable_ext0_wakeup(GYRO_INT_PIN, 0); //1 = High, 0 = Low
#endif
#endif
#endif
#endif

  //---------------------------------------------------------------
  // RTOS Tasks
  //---------------------------------------------------------------

  createRTOStasks();
  log_display("Setup done");
  dataBuffer.data.runmode = 1; // Switch from Terminal Mode to page Display
  Serial.println("Runmode5: " + String(dataBuffer.data.runmode));

  Serial.print("CPU Temperature: ");
  Serial.print((temprature_sens_read() - 32) / 1.8);
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
  mqtt_loop();
#endif

#if (USE_BUTTON)
  readButton();
#endif
}