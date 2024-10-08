#include "globals.h"
#include "display.h"

static const char TAG[] = "";

//-------------------------------------------------------------------------------
// 128*64 Pixel --> Center = 64*32
//-------------------------------------------------------------------------------
HAS_DISPLAY u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA); // ESP32 Thing, HW I2C with pin remapping
U8G2LOG u8g2log;
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
int PageNumber = 0;
char sbuf[32];
uint8_t page_array[13];
uint8_t max_page_counter;
uint8_t page_counter = 0;

#if (HAS_TFT_DISPLAY)
// Depend TFT_eSPI library ,See  https://github.com/Bodmer/TFT_eSPI
// goto pio->libsdeps-->usb-->TFT_eSPI-->User_Setup_Select.h and comment line 22
//                                                               uncomment line 72
TFT_eSPI tft = TFT_eSPI();
// The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_HEIGHT 16    // Height of text to be printed and scrolled
#define BOT_FIXED_AREA 0  // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define TOP_FIXED_AREA 16 // Number of lines in top fixed area (lines counted from top of screen)
#define YMAX 320          // Bottom of screen area

// The initial y coordinate of the top of the scrolling area
uint16_t yStart = TOP_FIXED_AREA;
// yArea must be a integral multiple of TEXT_HEIGHT
uint16_t yArea = YMAX - TOP_FIXED_AREA - BOT_FIXED_AREA;
// The initial y coordinate of the top of the bottom text line
uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;

// Keep track of the drawing x coordinate
uint16_t xPos = 0;

#endif

#if (DEVICE_ID == SUN_TRACKER)
void displayRegisterPagesSunTracker()
{

  max_page_counter = 0;
  // page_array[max_page_counter] = PAGE_TBEAM;

  // #if (HAS_LORA)
  //   max_page_counter++;
  //   page_array[max_page_counter] = PAGE_LORA;
  // #endif

#if (HAS_INA3221 || HAS_INA219)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SOLAR;
#endif

#if (HAS_PMU)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_BAT;
#endif

#if (USE_GPS)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_GPS;
#endif

#if (USE_BME280)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SENSORS;
#endif

#if (USE_SUN_POSITION)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SUN;
#endif
}
#endif

void displayRegisterPages()
{

  max_page_counter = 0;
  page_array[max_page_counter] = PAGE_TBEAM;

  // max_page_counter++;
  // page_array[max_page_counter] = PAGE_MODULS;

  max_page_counter++;
  page_array[max_page_counter] = PAGE_SYSTEM;

#if (HAS_LORA)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_LORA;
#endif

#if (HAS_INA3221 || HAS_INA219)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SOLAR;
#endif

#if (HAS_PMU)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_BAT;
#endif

#if (USE_GPS)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_GPS;
#endif

#if (USE_BME280)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SENSORS;
#endif

#if (USE_SUN_POSITION)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SUN;
#endif

#if (USE_SOIL_MOISTURE)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_SPRINKLER;
#endif

#if (USE_WIFICOUNTER)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_WIFICOUNTER;
#endif
}

void log_display(String s)
{
  Serial.println(s);
  // Serial.print("Runmode:");Serial.println(dataBuffer.data.runmode);

#if (USE_SERIAL_BT)
  SerialBT.println(s);
#endif

  // if (dataBuffer.data.runmode < 1)
  //{
  //   u8g2log.print(s);
  //   u8g2log.print("\n");
  // }

#if (HAS_TFT_DISPLAY)
  tft.println(s);
#endif
}

void t_moveDisplayRTOS(void *pvParameters)
{
#if (USE_DISPLAY)

  for (;;)
  {

    t_moveDisplay();
    vTaskDelay(displayMoveIntervall * 1000 / portTICK_PERIOD_MS);
  }
#endif
}

#if (HAS_TFT_DISPLAY)

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height())
    return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

bool setupTFTDisplay()
{
  ESP_LOGI(TAG, "Setup TFT Display");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLUE);

  // tft.setTextDatum(MC_DATUM);
  // tft.drawString("MrFlexi PlantServ", 20, 10);
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(1);
  tft.setCursor(0, 0, 1);
  tft.println("Plantserve");
  tft.println("Wifi... enabled");
  tft.println("MQTT... enabled");
  tft.println("Pushing foto to Netcup");

  // Setup scroll area
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  delay(3000);
  return true;
}
#endif

void setup_display(void)
{
  Serial.println();
  ESP_LOGI(TAG, "-----------  Setup display   -----------");
#if (HAS_TFT_DISPLAY)
  setupTFTDisplay();
#else
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_mf); // set the font for the terminal window
  u8g2.setContrast(255);
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0);                               // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);                                     // 0: Update screen with newline, 1: Update screen for every char
  u8g2.enableUTF8Print();
#endif

  switch (DEVICE_ID)
  {
  case DEVICE_SUN_TRACKER:
    // displayRegisterPagesSunTracker();
    break;
  default:
    displayRegisterPages();
  }
}

void drawSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
  // fonts used:
  // u8g2_font_open_iconic_embedded_6x_t
  // u8g2_font_open_iconic_weather_6x_t
  // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic

  switch (symbol)
  {
  case SUN:
    u8g2.setFont(u8g2_font_open_iconic_weather_8x_t);
    u8g2.drawGlyph(x, y, 69);
    break;
  case SUN_CLOUD:
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
    u8g2.drawGlyph(x, y, 65);
    break;
  case CLOUD:
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
    u8g2.drawGlyph(x, y, 64);
    break;
  case RAIN:
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
    u8g2.drawGlyph(x, y, 67);
    break;
  case THUNDER:
    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
    u8g2.drawGlyph(x, y, 67);
    break;
  case SLEEP:
    u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
    u8g2.drawGlyph(x, y, 72);
    break;
  case ICON_NOTES:
    u8g2.setFont(u8g2_font_open_iconic_all_4x_t); // 32x32 Pixel
    u8g2.drawGlyph(x, y, 225);
    break;
  case ICON_BOOT:
    u8g2.setFont(u8g2_font_open_iconic_all_4x_t); // 32x32 Pixel
    u8g2.drawGlyph(x, y, 145);
    break;
  case ICON_SMILE:
    u8g2.setFont(u8g2_font_emoticons21_tr);
    u8g2.drawGlyph(x, y, 17);
    break;
  }
}

void showPage(int page)
{

  String IP_String = "";
  String availableModules = "";

  // block i2c bus access
  if (!I2C_MUTEX_LOCK())
    ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else
  {

    u8g2.clearBuffer();
    u8g2.clearDisplay();
    uint8_t icon = 0;

    // ESP_LOGI(TAG, "Display page: %d", page);

    switch (page)
    {

    case PAGE_BOOT:
      // drawSymbol(30, 40, SUN);
      // drawSymbol(30, 40, ICON_SMILE);
      drawSymbol(48, 32, ICON_BOOT); // place in the center of display

      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 60, "booting...");
      break;

    case PAGE_TBEAM:
      u8g2.setFont(u8g2_font_ncenB12_tr);      
      u8g2.setCursor(1, 15);
      u8g2.printf("%s", DEVICE_NAME);

      u8g2.setFont(u8g2_font_profont12_tr);  
      u8g2.setCursor(1, 30);
      u8g2.printf("Deep Sleep: %2d / %3d ", dataBuffer.data.MotionCounter, dataBuffer.settings.sleep_time);

      u8g2.setCursor(1, 45);
      u8g2.printf("BootCnt: %2d ", dataBuffer.data.bootCounter);      
      break;

    case PAGE_OTA:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.setCursor(1, 15);
      u8g2.printf("OTA Update");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("reboot after success");

      break;

    case PAGE_MODULS:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      sprintf(sbuf, "Moduls");
      u8g2.drawStr(1, 15, sbuf);

      u8g2.setFont(u8g2_font_profont12_tr);

      // WLAN
      u8g2.setCursor(1, 30);
      if (dataBuffer.data.wlan)
        u8g2.printf("WLAN");

      // BLE
      u8g2.setCursor(40, 30);
      if (dataBuffer.data.ble_device_connected)
        u8g2.printf("BLE");

      u8g2.setDrawColor(1);
      u8g2.setCursor(64, 30);
      u8g2.printf("LORA");
      break;

    case PAGE_LORA:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "LORA TX/RX");
      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("TX:%.3d", dataBuffer.data.txCounter);
      u8g2.setCursor(64, 30);
      u8g2.printf("TX Que:%.2d", dataBuffer.data.LoraQueueCounter);
      u8g2.setCursor(1, 45);
      u8g2.printf("RX %.3d Len:%.2d", dataBuffer.data.rxCounter, dataBuffer.data.lmic.dataLen);
      u8g2.setCursor(1, 60);
      u8g2.printf("RX RSSI %d SNR %.1d", dataBuffer.data.lmic.rssi, dataBuffer.data.lmic.snr);
      break;

#if (USE_GPS)
    case PAGE_GPS:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "GPS");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Sats:%.2d", gps.tGps.satellites.value());
      u8g2.setCursor(64, 30);  // GPS Time
      u8g2.printf("%02d:%02d:%02d", gps.tGps.time.hour(), gps.tGps.time.minute(), gps.tGps.time.second());
      u8g2.setCursor(1, 40);
      u8g2.printf("Alt:%.4g m", gps.tGps.altitude.meters());
      u8g2.setCursor(1, 60);
      u8g2.printf("Dist:%.0f m Lat %.0f", dataBuffer.data.gps_distance, dataBuffer.data.gps_old.lat());
      break;
#endif

    case PAGE_SOLAR:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Solar Panel");

#if (HAS_INA3221 || HAS_INA219)
      u8g2.setCursor(1, 30);
      u8g2.printf("In: %.2f V  %.0f mA ", dataBuffer.data.ina219[0].voltage, dataBuffer.data.ina219[0].current);
      u8g2.setCursor(1, 45);
      u8g2.printf("In: %.2f mW", dataBuffer.data.ina219[0].power);
#endif

#if (HAS_PMU)
      u8g2.setCursor(1, 40);
      u8g2.printf("PMU Bus: %.2fV  %.0fmA ", dataBuffer.data.bus_voltage, dataBuffer.data.bus_current);
#else
      u8g2.setCursor(1, 40);
      // u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif
      break;

    case PAGE_BAT:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Battery");
      u8g2.setFont(u8g2_font_profont12_tr);

#if (HAS_PMU)
      u8g2.setCursor(1, 30);
      u8g2.printf("Volt:   %.2fV", dataBuffer.data.bat_voltage);

      // u8g2.setCursor(1, 40);
      // u8g2.printf("Bat-: %.2fV %.0fmA ", dataBuffer.data.bat_voltage, dataBuffer.data.bat_discharge_current);

      u8g2.setCursor(1, 45);
      u8g2.printf("Charge: %.0fmA ", dataBuffer.data.bat_charge_current);

      u8g2.setCursor(1, 60);
      u8g2.printf("Fuel:   %.0fmAh ", dataBuffer.data.bat_DeltamAh);
#else
      u8g2.setCursor(1, 40);
      // u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif

      break;

    case PAGE_SENSORS:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Sensors");
      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Temp: %.2fC %.0f hum ", dataBuffer.data.temperature, dataBuffer.data.humidity);
      u8g2.setCursor(1, 45);
      u8g2.printf("CPU Temp: %.2f C ", dataBuffer.data.cpu_temperature);
      u8g2.setCursor(1, 60);
      break;

    case PAGE_GYRO:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "GYRO");
      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Yaw  :%.2f", dataBuffer.data.yaw);
      u8g2.setCursor(1, 45);
      u8g2.printf("Pitch:%.2f", dataBuffer.data.pitch);
      u8g2.setCursor(1, 60);
      u8g2.printf("Roll  :%.2f", dataBuffer.data.roll);
      break;

    case PAGE_POTI:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Soil Moisture");
      u8g2.setFont(u8g2_font_ncenB24_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("GPIO36: %d", dataBuffer.data.potentiometer_a);
      u8g2.setCursor(1, 45);
      u8g2.printf("Soil: %.0f", dataBuffer.data.soil_moisture);
      break;

    case PAGE_WIFICOUNTER:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Wifi Counter");      
      if (dataBuffer.wificounter.active)
      {
        u8g2.setFont(u8g2_font_profont12_tr);
        u8g2.setCursor(1, 30);
        u8g2.printf("%d/%d", dataBuffer.wificounter.count,dataBuffer.wificounter.count5 );        
        u8g2.setCursor(1, 45);
        u8g2.printf("Size %02d kB", dataBuffer.wificounter.fileSize );        
      }      
      else
      {
        u8g2.setFont(u8g2_font_profont12_tr);        
        u8g2.setCursor(1, 30);        
        u8g2.printf("%s", dataBuffer.data.ip_address.c_str());
        u8g2.drawStr(1, 45, "Sniffer: OFF");       
      }
      break;

      case PAGE_SYSTEM:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "System");            
      u8g2.setFont(u8g2_font_profont12_tr);        
      u8g2.setCursor(1, 30);        
      u8g2.printf("%s", dataBuffer.data.ip_address.c_str()); 
      u8g2.setCursor(1, 40);  // Real Time from WIFI or GPS
      u8g2.printf("%02d:%02d:%02d", dataBuffer.data.timeinfo.tm_hour, dataBuffer.data.timeinfo.tm_min, dataBuffer.data.timeinfo.tm_sec );     
      u8g2.setCursor(1, 60);  // Free SPIFFS File space
      u8g2.printf("free SPIFF %02d kB", dataBuffer.data.spiffsfreeKBytes );     
      break;
      
    case PAGE_SUN:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "SunTracker");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
#if (USE_GPS)
      u8g2.printf("Sats:%.2d", gps.tGps.satellites.value());
      u8g2.setCursor(64, 30);
      u8g2.printf("%02d:%02d:%02d", gps.tGps.time.hour(), gps.tGps.time.minute(), gps.tGps.time.second());
#endif

      u8g2.setCursor(1, 40);
      u8g2.printf("Sun: %.1f %.1f", dataBuffer.data.sun_azimuth, dataBuffer.data.sun_elevation);
      u8g2.setCursor(1, 60);
      u8g2.printf("Servo: %d  %d", dataBuffer.data.servo1, dataBuffer.data.servo2);
      break;

    case PAGE_SPRINKLER:

      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Sprinkler");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Soil moistr: %.0f", dataBuffer.data.soil_moisture);
      u8g2.setCursor(1, 40);
      u8g2.printf("Water level: %d ", dataBuffer.data.hcsr04_distance);
      break;

    case PAGE_SLEEP:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Sleep");
      u8g2.setFont(u8g2_font_profont12_tr);

#if (HAS_PMU)
      u8g2.setCursor(1, 25);
      u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
      u8g2.setCursor(1, 35);
      u8g2.printf("Fuel: %.0fmAh ", dataBuffer.data.bat_DeltamAh);
#else
      u8g2.setCursor(1, 25);
      // u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif

      if (dataBuffer.data.MotionCounter <= 0)
      {
        u8g2.drawStr(1, 55, "Inactivity");
      }

#ifdef SLEEP_AFTER_N_TX_COUNT 4 // after n Lora TX events
      if (dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT)
      {
        u8g2.drawStr(1, 55, "TX ");
      }
#endif

      u8g2.setCursor(1, 64);
      u8g2.printf("Sleeping for %i min", dataBuffer.settings.sleep_time);
      drawSymbol(60, 12, THUNDER);

      break;
    }
    //---------------------------------
    //----------   Footer   -----------
    //---------------------------------

    if (page < 20)
    {
      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(96, 64);
      u8g2.printf("%i min", dataBuffer.data.MotionCounter);
    }

    u8g2.sendBuffer();
    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }
}

void t_moveDisplay(void)
{
#if (USE_DISPLAY)

  if (dataBuffer.data.pictureLoop)
  {
    if (page_counter < max_page_counter)
    {
      page_counter++;
      // ESP_LOGI(TAG, "P counter: %d", page_counter);
    }
    else
    {
      page_counter = 0;
      ESP_LOGI(TAG, "P counter set to 0");
    }
    PageNumber = page_array[page_counter];

    // Refresh Display
    showPage(PageNumber);
  }
#endif
}
