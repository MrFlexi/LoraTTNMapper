#define USE_WIFI        0
#define USE_BME280      1
#define USE_CAYENNE     0
#define HAS_LORA        1

#define HAS_PMU         0
#define PMU_INT         35  


#define HAS_DISPLAY
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "globals.h"

#if (HAS_PMU)
#include "axp20x.h"
#endif

int runmode = 0;
int aliveCounter = 0;
String stringOne = "";

static const char TAG[] = __FILE__;


//----------------------------------------------------------
// T Beam Power Management
//----------------------------------------------------------
#if (HAS_PMU)
  #define AXP192_PRIMARY_ADDRESS (0x34)

 AXP20X_Class axp;
#endif


#define SEALEVELPRESSURE_HPA (1013.25)

// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 21

char s[32]; // used to sprintf for Serial output
uint8_t txBuffer[9];
//gps gps;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
// #define SLEEP_ESP32

const char ssid[] = "MrFlexi";
const char wifiPassword[] = "Linde-123";

Ticker aliveTicker;
const float alivePeriod = 30; //seconds

//--------------------------------------------------------------------------
// Sensors
//--------------------------------------------------------------------------
Adafruit_BME280 bme; // I2C   PIN 21 + 22

//--------------------------------------------------------------------------
// LORA Settings
//--------------------------------------------------------------------------
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = { 0x88, 0x06, 0xDA, 0xCF, 0x30, 0xFB, 0x44, 0xDC, 0x69, 0x0E, 0x15, 0xF8, 0xAD, 0xCB, 0x40, 0x6C };

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = { 0xB3, 0xB1, 0x59, 0x5D, 0x24, 0xBD, 0xD2, 0xF5, 0x6A, 0x17, 0x0A, 0x94, 0xF2, 0xED, 0xDB, 0xC2 };
static u4_t DEVADDR = 0x260118B7; // <-- Change this address for every node!

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 120;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN, // was "14,"
  .dio = {26, 33, 32},
};

void do_send(osjob_t* j) {  

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
    {
      //try again in 3 seconds
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(10), do_send);
    }
  }
  // Next TX is scheduled after TX_COMPLETE event.
}


void setup_sensors()
{

 #if (USE_BME280) 
  ESP_LOGI(TAG, "BME280 Setup...");   
     unsigned status;      
     
    // https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/issues/62

    //bool wire_status = Wire1.begin( GPIO_NUM_4, GPIO_NUM_15);
    //if(!wire_status)
    //{
    //  Serial.println("Could not finitialize Wire1"); 
    //}


     //status = bme.begin(0x76, &Wire1);  
     status = bme.begin(0x76); 
     if (!status) { 
         Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"); 
         Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16); 
         Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"); 
         Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n"); 
         Serial.print("        ID of 0x60 represents a BME 280.\n"); 
         Serial.print("        ID of 0x61 represents a BME 680.\n"); 
         while (1); 
     } 
     
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
#endif

  }



void t_alive() {
  static char volbuffer[20];

  String stringOne;
  aliveCounter++;

  dataBuffer.data.aliveCounter = aliveCounter;

  stringOne = "Alive: ";
  stringOne = stringOne + aliveCounter;   
  log_display(stringOne);

  // Battery
  #if (HAS_PMU)
  if (axp.isBatteryConnect()) {
        snprintf(volbuffer, sizeof(volbuffer), "%.2fV/%.2fmA", axp.getBattVoltage() / 1000.0, axp.isChargeing() ? axp.getBattChargeCurrent() : axp.getBattDischargeCurrent());
         log_display(volbuffer);
    }
  #endif
  // Temperatur
  #if (USE_BME280) 
  snprintf(volbuffer, sizeof(volbuffer), "%.1fC/%.1f%", bme.readTemperature(), bme.readHumidity());
         log_display(volbuffer);
  #endif

  gps.encode();
  showPage( 1 );

}


void setup_wifi()
{

#if (USE_WIFI) 
  // WIFI Setup
  WiFi.begin(ssid, wifiPassword );

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
      ESP_LOGI(TAG, "Connecting to WiFi..");    
  }
ESP_LOGI(TAG, WiFi.localIP() );  
#endif
}


void onEvent (ev_t ev) {
  switch (ev) {
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
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      digitalWrite(BUILTIN_LED, LOW);
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("Received Ack"));
      }
      if (LMIC.dataLen) {
        sprintf(s, "Received %i bytes of payload", LMIC.dataLen);
        Serial.println(s);
        sprintf(s, "RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
        Serial.println(s);
      }
      // Schedule next transmission
      //esp_sleep_enable_timer_wakeup(TX_INTERVAL*1000000);
      //esp_deep_sleep_start();
      log_display("Next TX started");
      do_send(&sendjob);
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
      Serial.println(F("EV_LINK_ALIVE"));
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
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14); 

  do_send(&sendjob);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  }

#if (HAS_PMU)
void PMU_init()
{
ESP_LOGI(TAG, "AXP192 PMU initialization");

  if (axp.begin(Wire, AXP192_PRIMARY_ADDRESS))
    ESP_LOGW(TAG, "AXP192 PMU initialization failed");
  else {

    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
    axp.setDCDC1Voltage(3300);
    axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    //axp.setChgLEDMode(AXP20X_LED_OFF);
    axp.adc1Enable(AXP202_BATT_CUR_ADC1, 1);

#ifdef PMU_INT
    pinMode(PMU_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PMU_INT),
                    [] {
                    log_display("Power source changed");
                      /* put your code here */
                    },
                    FALLING);
    axp.enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ |
                      AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ,
                  1);
    axp.clearIRQ();
#endif // PMU_INT

    ESP_LOGI(TAG, "AXP192 PMU initialized.");
  }
}

#endif

void setup() {
  Serial.begin(115200);
  //esp_log_level_set("*", LOG_LOCAL_LEVEL);
  ESP_LOGI(TAG, "Starting..");    
  Serial.println(F("TTN Mapper"));
  i2c_scan();
  #if (HAS_PMU)
  PMU_init();
  #endif
  setup_display();
  setup_sensors();
  setup_wifi();
  
  //Turn off WiFi and Bluetooth
  //WiFi.mode(WIFI_OFF);
  //btStop();
  gps.init();

  #if (HAS_LORA)
  setup_lora();
  #endif
  aliveTicker.attach(alivePeriod, t_alive);   
   
  runmode = 1;    // Switch from Terminal Mode to page Display
  showPage( 1 );
  delay(5000);
}


void loop() {
  #if (HAS_LORA)
    os_runloop_once();
  #endif
}