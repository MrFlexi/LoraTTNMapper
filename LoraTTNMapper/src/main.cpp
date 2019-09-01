#define USE_WIFI        0
#define USE_BME280      1
#define USE_CAYENNE     1

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Ticker.h>
#include "esp_sleep.h"
#include "WiFi.h"
#include "gps.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)

// T-Beam specific hardware
#undef BUILTIN_LED
#define BUILTIN_LED 21

char s[32]; // used to sprintf for Serial output
uint8_t txBuffer[9];
gps gps;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
// #define SLEEP_ESP32


const char ssid[] = "MrFlexi";
const char wifiPassword[] = "Linde-123";


Ticker aliveTicker;
const float alivePeriod = 30; //seconds

int runmode = 0;
int aliveCounter = 0;
String stringOne = "";

//--------------------------------------------------------------------------
// U8G2 Display Setup
//--------------------------------------------------------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // ESP32 Thing, HW I2C with pin remapping
// Create a U8g2log object
U8G2LOG u8g2log;

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 6

// allocate memory
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

Adafruit_BME280 bme; // I2C   PIN 21 + 22

// local Tag for logging
static const char TAG[] = __FILE__;




// LoRaWAN NwkSKey, network session key // msb

static PROGMEM u1_t NWKSKEY[16] = { 0x0A, 0x5C, 0x54, 0x63, 0x63, 0x6B, 0x57, 0x78, 0x96, 0xF3, 0x99, 0x92, 0x3E, 0xB8, 0xBB, 0x16 };

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = { 0x68, 0xD8, 0x63, 0x68, 0x86, 0xC5, 0xB2, 0xAC, 0x27, 0xC9, 0xF7, 0x63, 0x80, 0xE4, 0xB7, 0x88 };
static u4_t DEVADDR = 0x26011B09 ; // <-- Change this address for every node!

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
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(3), do_send);
    }
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void log_display(String s)
{
  Serial.println(s);
  if (runmode < 1)
  {
    u8g2log.print(s);
    u8g2log.print("\n");
  }
}

void setup_display(void)
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_mf);                         // set the font for the terminal window
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0);                               // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);                                     // 0: Update screen with newline, 1: Update screen for every char
  u8g2.enableUTF8Print();
  log_display("Display loaded...");
  log_display("TTN-ABP-Mapper");
  Serial.println( SDA );
  Serial.println( SCL );

}


void i2c_scan()
{

Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin(21,22);  // for T-Beam pass SDA and SCL GPIO pins
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");

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







void alive() {
  String stringOne;
  aliveCounter++;
  stringOne = "Alive: ";
  stringOne = stringOne + aliveCounter;   
  log_display(stringOne);
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
      esp_sleep_enable_timer_wakeup(TX_INTERVAL*1000000);
      esp_deep_sleep_start();
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



void setup() {
  Serial.begin(115200);
  Serial.println(F("TTN Mapper"));
  i2c_scan();

  setup_display();
  setup_sensors();
  setup_wifi();
  
  //Turn off WiFi and Bluetooth
  //WiFi.mode(WIFI_OFF);
  btStop();
  gps.init();

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

void loop() {
    os_runloop_once();
}