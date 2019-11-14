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
QueueHandle_t LoraSendQueue;
SemaphoreHandle_t I2Caccess;
int runmode = 0;

uint8_t msgWaiting = 0;

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
char s[32]; // used to sprintf for Serial output
uint8_t txBuffer[9];



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

//--------------------------------------------------------------------------
// MQTT
//--------------------------------------------------------------------------




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
// LORA Settings
//--------------------------------------------------------------------------
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = {0x88, 0x06, 0xDA, 0xCF, 0x30, 0xFB, 0x44, 0xDC, 0x69, 0x0E, 0x15, 0xF8, 0xAD, 0xCB, 0x40, 0x6C};

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = {0xB3, 0xB1, 0x59, 0x5D, 0x24, 0xBD, 0xD2, 0xF5, 0x6A, 0x17, 0x0A, 0x94, 0xF2, 0xED, 0xDB, 0xC2};
static u4_t DEVADDR = 0x260118B7; // <-- Change this address for every node!

void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN, // was "14,"
    .dio = {26, 33, 32},
};

void do_send(osjob_t *j)
{

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {
    if (gps.checkGpsFix())
    {
      // Prepare upstream data transmission at the next possible time.
      gps.buildPacket(txBuffer);
      LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), 0);
      Serial.println(F("Packet queued"));
      digitalWrite(BUILTIN_LED, HIGH);
    }
    else
      ESP_LOGV(TAG, "GPS no fix");
  }
}

void t_LORA_send_from_queue(osjob_t *j)
{
  MessageBuffer_t SendBuffer;
  ESP_LOGI(TAG, "Send Lora MSG from Queue");

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {

    if (LoraSendQueue == 0)
    {
      ESP_LOGE(TAG, "LORA send queue not initalized. Aborting.");
    }
    else
    {
      if (xQueueReceive(LoraSendQueue, &SendBuffer, portMAX_DELAY) != pdTRUE)
      {
        ESP_LOGE(TAG, "Queue is empty...");
      }
      else
      {
        ESP_LOGI(TAG, "LORA package queued: Port %d, Size %d", SendBuffer.MessagePort, SendBuffer.MessageSize);
        ESP_LOGI(TAG, "SendBuffer[0..8]: %d %d %d %d %d %d %d %d ", SendBuffer.Message[0], SendBuffer.Message[1], SendBuffer.Message[2], SendBuffer.Message[3], SendBuffer.Message[4], SendBuffer.Message[5], SendBuffer.Message[6], SendBuffer.Message[7]);
        LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
        ESP_LOGI(TAG, "done...");
      }
    }
    ESP_LOGE(TAG, "New callback scheduled...");
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), t_LORA_send_from_queue);
  }
}

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

  gps.encode();
  gps.checkGpsFix();

  // Refresh Display
  showPage(PAGE_VALUES);
}

void t_enqueue_LORA_messages()
{
  String stringOne;

  if (LoraSendQueue == 0)
  {
    ESP_LOGE(TAG, "LORA send queue not initalized. Aborting.");
  }
  else
  {

#if (USE_GPS)
    if (gps.checkGpsFix())
    {
      payload.reset();
      payload.addGPS(gps.tGps); // TTN-Mapper format will be generated in TTN Payload converter
      payload.enqueue_port(1);
    }
    else
    {
      ESP_LOGV(TAG, "GPS no fix");
    }
#endif

#if (USE_BME280)
    payload.reset();
    payload.addBMETemp(2, dataBuffer); // Cayenne format will be generated in TTN Payload converter
    payload.enqueue_port(2);
#endif

    //payload.reset();
    //payload.addTemperature(1, 5.11);
    //payload.enqueue_port(2);

#if (HAS_PMU)
    //payload.reset();
    //payload.addVoltage(20, 2.45);
    //payload.enqueue_port(2);

    payload.reset();
    payload.addVoltage(30, dataBuffer.data.bat_voltage);
    //payload.enqueue_port(2);

    //payload.reset();
    payload.addVoltage(31, dataBuffer.data.bat_charge_current);
    //payload.enqueue_port(2);

    //payload.reset();
    payload.addVoltage(32, dataBuffer.data.bat_discharge_current);
    payload.enqueue_port(2);
#endif

    msgWaiting = uxQueueMessagesWaiting(LoraSendQueue);
    ESP_LOGI(TAG, "Lora Message Queue: %d", msgWaiting);
  }
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

void onEvent(ev_t ev)
{
  switch (ev)
  {
  case EV_SCAN_TIMEOUT:
    Serial.println(F("EV_SCAN_TIMEOUT"));
    break;
  case EV_BEACON_FOUND:
    Serial.println(F("EV_BEACON_FOUND"));
    break;
  case EV_BEACON_MISSED:
    Serial.println(F("EV_BEACON_MISSED"));
    break;
  case EV_BEACON_TRACKED:
    Serial.println(F("EV_BEACON_TRACKED"));
    break;
  case EV_JOINING:
    Serial.println(F("EV_JOINING"));
    break;
  case EV_JOINED:
    Serial.println(F("EV_JOINED"));
    // Disable link check validation (automatically enabled
    // during join, but not supported by TTN at this time).
    LMIC_setLinkCheckMode(0);
    break;
  case EV_RFU1:
    Serial.println(F("EV_RFU1"));
    break;
  case EV_JOIN_FAILED:
    Serial.println(F("EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    Serial.println(F("EV_REJOIN_FAILED"));
    break;
  case EV_TXCOMPLETE:
    log_display("EV_TXCOMPLETE");
    dataBuffer.data.txCounter++;
    Serial.println(F("EV_TXCOMPLETE (waiting for RX windows)"));
    digitalWrite(BUILTIN_LED, LOW);
    if (LMIC.txrxFlags & TXRX_ACK)
    {
      Serial.println(F("Received Ack"));
    }
    if (LMIC.dataLen)
    {
      sprintf(s, "Received %i bytes payload", LMIC.dataLen);
      Serial.println(s);
      dataBuffer.data.lmic = LMIC;
      sprintf(s, "RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
      Serial.println(s);
      Serial.println("");
      Serial.println("Payload");
      for (int i = 0; i < LMIC.dataLen; i++)
      {
        if (LMIC.frame[LMIC.dataBeg + i] < 0x10)
        {
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
        }
      }
    }
    // Schedule next transmission
    //esp_sleep_enable_timer_wakeup(TX_INTERVAL*1000000);
    //esp_deep_sleep_start();
    log_display("Next TX started");
    // Next TX is scheduled after TX_COMPLETE event.
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), t_LORA_send_from_queue);
    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    Serial.println(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    Serial.println(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    Serial.println(F("EV_LINK_sleep"));
    break;
  default:
    Serial.println(F("Unknown event"));
    break;
  }
}

void setup_lora()
{
  log_display("Setup LORA");
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7, 14);

  t_LORA_send_from_queue(&sendjob);
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