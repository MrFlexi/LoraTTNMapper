// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 14
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

#define displayRefreshIntervall 5    // every second
#define sendMessagesIntervall 60     // every minute
#define LORAsendMessagesIntervall 20 // every 20 seconds

//const float sleepPeriod = 2; //seconds
#define SEALEVELPRESSURE_HPA (1013.25)


#include "globals.h"

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
Adafruit_BME280 bme; // I2C   PIN 21 + 22


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
  //Cayenne.virtualWrite(12, ina3221.getBusVoltage_V(1)*ina3221.getCurrent_mA(1), "pow", "Watts");

  Cayenne.virtualWrite(20, dataBuffer.data.bus_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(21, dataBuffer.data.bus_current, "current", "Milliampere");
  //Cayenne.virtualWrite(22, pmu.getVbusCurrent()/1000*pmu.getVbusVoltage(), "pow", "Watts");

  Cayenne.virtualWrite(30, dataBuffer.data.bat_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(31, dataBuffer.data.bat_charge_current, "current", "Milliampere");
  //Cayenne.virtualWrite(32, pmu.getBattChargeCurrent()*pmu.getBattVoltage()/1000, "pow", "Watts");
  Cayenne.virtualWrite(33, dataBuffer.data.bat_discharge_current, "current", "Milliampere");
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

  Serial.println("h");
}
#endif



void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.print("WakeUp caused by: ");
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("ULP program");
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

#ifdef HAS_PMU
  dataBuffer.data.bus_voltage = pmu.getVbusVoltage() / 1000;
  dataBuffer.data.bus_current = pmu.getVbusCurrent();

  dataBuffer.data.bat_voltage = pmu.getBattVoltage() / 1000;
  dataBuffer.data.bat_charge_current = pmu.getBattChargeCurrent();
  dataBuffer.data.bat_discharge_current = pmu.getBattDischargeCurrent();
  AXP192_showstatus();
#endif

#if (HAS_INA)
  print_ina();
  dataBuffer.data.panel_voltage = ina3221.getBusVoltage_V(1);
  dataBuffer.data.panel_current = ina3221.getCurrent_mA(1);
#endif

#if (HAS_LORA)
  if (LoraSendQueue != 0)
  {
    dataBuffer.data.LoraQueueCounter=  uxQueueMessagesWaiting(LoraSendQueue);
  } 
  else
  {
  dataBuffer.data.LoraQueueCounter= 0;
  }    
#endif

  gps.encode();
  gps.checkGpsFix();

  // Refresh Display
  showPage(PageNumber);
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
    AXP192_power_gps(OFF);
    AXP192_power_lora(OFF);
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

    ESP_LOGI(TAG, "ESP32 Deep Sleep started");
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

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    ESP_LOGI(TAG, "Connecting to WiFi..");
  }
  ESP_LOGV(TAG, String(WiFi.localIP()));
  log_display(String(WiFi.localIP()));
  delay(2000);
#endif
}





void setup()
{
  Serial.begin(115200);
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

  setup_display();
  setup_sensors();
  setup_wifi();
  calibrate_voltage();

  //Turn off WiFi and Bluetooth
  //log_display("Stop Bluethooth");
  //WiFi.mode(WIFI_OFF);
  btStop();

#if (USE_MQTT)
  setup_mqtt();
#endif

#if (USE_CAYENNE)
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  u8g2log.print("Cayenne connected...");
  u8g2log.print("\n");
#endif

//---------------------------------------------------------------
// Deep sleep settings
//---------------------------------------------------------------
#if (ESP_SLEEP)
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
  Serial.println("Setup ESP32 to wake-up via timer after " + String(TIME_TO_SLEEP) +
                 " Minutes");
#endif

  gps.init();
  //gps.softwareReset();
  gps.wakeup();
  //gps.ecoMode();

  delay(2000); // Wait for GPS beeing stable

#if (HAS_LORA)
  setup_lora();
  lora_queue_init();
#endif

#if (USE_DASH)
  create_web_dash();
#endif

#ifdef HAS_BUTTON
  button_init(HAS_BUTTON);
#endif

  // Tasks
  ESP_LOGV(TAG, "-- Starting Tasks --");

  sleepTicker.attach(60, t_sleep);
  displayTicker.attach(displayRefreshIntervall, t_cyclic);
  sendMessageTicker.attach(sendMessagesIntervall, t_enqueue_LORA_messages);
  //LORAsendMessageTicker.attach(LORAsendMessagesIntervall, t_LORA_send_from_queue);

  ESP_LOGV(TAG, "-- Setup done --");

  runmode = 1; // Switch from Terminal Mode to page Display
  showPage(1);
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
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
#endif

#ifdef HAS_BUTTON
  readButton();
#endif
}