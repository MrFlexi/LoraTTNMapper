#define USE_WIFI 0
#define USE_BME280 0
#define USE_CAYENNE 0
#define HAS_LORA 1
#define USE_MQTT 0

#define HAS_INA 0

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// I2C bus access control
#include <FreeRTOS.h>

#include "globals.h"

SemaphoreHandle_t I2Caccess;

//--------------------------------------------------------------------------
// Store preferences in NVS Flash
//--------------------------------------------------------------------------
Preferences preferences;
char lastword[10];

unsigned long uptime_seconds_old;
unsigned long uptime_seconds_new;
unsigned long uptime_seconds_actual;

#define display_refresh 5   // every second

int runmode = 0;
String stringOne = "";

static const char TAG[] = __FILE__;

#define SEALEVELPRESSURE_HPA (1013.25)

// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 21

char s[32]; // used to sprintf for Serial output
uint8_t txBuffer[9];

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

//--------------------------------------------------------------------------
// Wifi Settings
//--------------------------------------------------------------------------
const char ssid[] = "MrFlexi";
const char wifiPassword[] = "Linde-123";
WiFiClient wifiClient;

#if (HAS_INA)
void print_ina()
{
  Serial.println("------------------------------");
  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;

  busvoltage1 = ina3221.getBusVoltage_V(1);
  shuntvoltage1 = ina3221.getShuntVoltage_mV(1);
  current_mA1 = -ina3221.getCurrent_mA(1); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

  Serial.print("LIPO_Battery Bus Voltage:   ");
  Serial.print(busvoltage1);
  Serial.println(" V");
  Serial.print("LIPO_Battery Shunt Voltage: ");
  Serial.print(shuntvoltage1);
  Serial.println(" mV");
  Serial.print("LIPO_Battery Load Voltage:  ");
  Serial.print(loadvoltage1);
  Serial.println(" V");
  Serial.print("LIPO_Battery Current 1:       ");
  Serial.print(current_mA1);
  Serial.println(" mA");
  Serial.println("");

  Serial.println("h");
}
#endif

//--------------------------------------------------------------------------
// MQTT
//--------------------------------------------------------------------------
#if (USE_MQTT)
#include <PubSubClient.h>
#endif
//const char *mqtt_server = "192.168.1.144"; // Laptop
//const char *mqtt_server = "test.mosquitto.org"; // Laptop
const char *mqtt_server = "192.168.1.100"; // Raspberry
const char *mqtt_topic = "mrflexi/solarserver/";

#if (USE_MQTT)
PubSubClient client(wifiClient);
long lastMsgAlive = 0;
long lastMsgDist = 0;
#endif

#if (USE_MQTT)
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");

  u8g2log.print(topic);
  u8g2log.print("\n");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    u8g2log.print((char)payload[i]);
  }
  Serial.println();
  u8g2log.print("\n");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Mqtt Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("MrFlexi/nodemcu", "connected");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_mqtt()
{

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!client.connected())
  {
    reconnect();
  }

  log_display("Mqtt connected");
  client.publish("mrflexi/solarserver/info", "ESP32 is alive...");
}
#endif

Ticker sleepTicker;
Ticker displayTicker;

const float sleepPeriod = 2; //seconds

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
  }
}

void save_uptime()
{
  uptime_seconds_new = uptime_seconds_old + uptime_seconds_actual;
  preferences.putULong("uptime", uptime_seconds_new);
  Serial.println("ESP32 total uptime" + String(uptime_seconds_new) + " Seconds");
}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
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
    ESP_LOGI(TAG, "Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
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

void t_sleep()
{
  dataBuffer.data.sleepCounter--;

  //-----------------------------------------------------
  // Deep sleep
  //-----------------------------------------------------
#if (ESP_SLEEP)
  if (dataBuffer.data.sleepCounter <= 0 || dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT)
  {
    runmode = 0;
    gps.enable_sleep();
    Serial.flush();
    showPage(PAGE_SLEEP);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }
#endif
}

void t_display()
{
  static char volbuffer[20];

  String stringOne;
  // dataBuffer.data.bat_voltage = pmu.getBattVoltage() / 1000.0

// Temperatur
#if (USE_BME280)
  snprintf(volbuffer, sizeof(volbuffer), "%.1fC/%.1f%", bme.readTemperature(), bme.readHumidity());
  log_display(volbuffer);
#endif

// read battery voltage into global variable
#if (defined BAT_MEASURE_ADC || defined HAS_PMU)
  uint16_t batt_voltage = read_voltage();
  if (batt_voltage == 0xffff)
    ESP_LOGI(TAG, "Battery: external power");
  else
    ESP_LOGI(TAG, "Battery: %dmV", batt_voltage);
#ifdef HAS_PMU
  AXP192_showstatus();
#endif
#endif

#if (HAS_INA)
  print_ina();
#endif

  //gps.encode();
  
  //showPage(PAGE_VALUES);
  
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
  //ESP_LOGI(TAG, String(WiFi.localIP()) );
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
    Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
    digitalWrite(BUILTIN_LED, LOW);
    if (LMIC.txrxFlags & TXRX_ACK)
    {
      Serial.println(F("Received Ack"));
    }
    if (LMIC.dataLen)
    {
      sprintf(s, "Received %i bytes of payload", LMIC.dataLen);
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
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
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

  do_send(&sendjob);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
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

#if (HAS_INA)
  ina3221.begin();
  Serial.print("Manufactures ID=0x");
  int MID;
  MID = ina3221.getManufID();
  Serial.println(MID, HEX);
  print_ina();
#endif

 #if (HAS_PMU)
  AXP192_init();
  AXP192_showstatus();
  #endif

  dataBuffer.data.txCounter = 0;
  dataBuffer.data.sleepCounter = TIME_TO_NEXT_SLEEP;

  setup_display();
  setup_sensors();
  setup_wifi();
  calibrate_voltage();

  //Turn off WiFi and Bluetooth
  //WiFi.mode(WIFI_OFF);
  //btStop();

#if (HAS_LORA)
  setup_lora();
#endif




#if (USE_MQTT)
  setup_mqtt();
#endif

  // Tasks
  sleepTicker.attach(60, t_sleep);
  displayTicker.attach(display_refresh, t_display);


  runmode = 1; // Switch from Terminal Mode to page Display
  showPage(1);
  ESP_LOGI(TAG, "Status");
  AXP192_showstatus();

  //---------------------------------------------------------------
  // Deep sleep settings
  //---------------------------------------------------------------
  #if (ESP_SLEEP)
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
  Serial.println("Setup ESP32 to wake-up via timer after " + String(TIME_TO_SLEEP) +
                 " Minutes");
  #endif               

  gps.init();
  gps.wakeup();
  gps.ecoMode();
}

void loop()
{
#if (HAS_LORA)
  os_runloop_once();
#endif

#if (USE_MQTT)
  // MQTT Connection
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
#endif
}