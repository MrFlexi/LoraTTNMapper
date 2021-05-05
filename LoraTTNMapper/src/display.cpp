#include "globals.h"
#include "display.h"

HAS_DISPLAY u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA); // ESP32 Thing, HW I2C with pin remapping
U8G2LOG u8g2log;
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
int PageNumber = 0;
char sbuf[32];
uint8_t page_array[10];
uint8_t max_page_counter;
uint8_t page_counter = 0;

void displayRegisterPages()
{

  max_page_counter = 0;

  page_array[max_page_counter] = PAGE_TBEAM;
  max_page_counter++;
  page_array[max_page_counter] = PAGE_MODULS;

#if (HAS_LORA)
  max_page_counter++;
  page_array[max_page_counter] = PAGE_LORA;
#endif

#if (HAS_INA)
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
}

void log_display(String s)
{
  Serial.println(s);
  //Serial.print("Runmode:");Serial.println(dataBuffer.data.runmode);

#if (USE_SERIAL_BT)
  SerialBT.println(s);
#endif

  if (dataBuffer.data.runmode < 1)
  {
    u8g2log.print(s);
    u8g2log.print("\n");
  }
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

void t_moveDisplay(void)
{
#if (USE_DISPLAY)

  if (dataBuffer.data.pictureLoop)
  {
    if (page_counter < max_page_counter)
    {
      page_counter++;
    }
    else
    {
      page_counter = 0;
    }
    PageNumber = page_array[page_counter];
  }
#endif
}

void setup_display(void)
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_mf); // set the font for the terminal window
  u8g2.setContrast(255);
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0);                               // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);                                     // 0: Update screen with newline, 1: Update screen for every char
  u8g2.enableUTF8Print();
  log_display("SAP GTT");
  log_display("TTN-ABP-Mapper");
  displayRegisterPages();
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
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
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
    u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
    u8g2.drawGlyph(x, y, 225);
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

    // u8g2_font_profont15_tr  W7 H15
    // u8g2_font_ncenB08_tr W12 H13
    u8g2.clearBuffer();
    uint8_t icon = 0;

    switch (page)
    {
    case PAGE_TBEAM:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.setCursor(1, 15);
      u8g2.printf( "Firmware:  %.2f", dataBuffer.data.firmware_version);
     
      u8g2.setCursor(1, 30);
      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.printf("Deep Sleep in: %2d ", dataBuffer.data.MotionCounter );
  
      u8g2.setCursor(1, 45);
      u8g2.printf("Deep Sleep for: %3d", dataBuffer.settings.sleep_time);


      u8g2.setCursor(1, 60);
      u8g2.printf("BootCnt: %2d", dataBuffer.data.bootCounter);


      break;
    case PAGE_MODULS:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      sprintf(sbuf, "Moduls");
      u8g2.drawStr(1, 15, sbuf);

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      if (dataBuffer.data.wlan)
        u8g2.setDrawColor(0);
      else
        u8g2.setDrawColor(1);
      u8g2.printf("WLAN");

      u8g2.setDrawColor(1);
      u8g2.setCursor(64, 30);
      u8g2.printf("LORA");
      //u8g2.setCursor(1, 45);
      //u8g2.printf("Exp: %s", dataBuffer.settings.experiment);
      //u8g2.setCursor(1, 60);
      //u8g2.printf("RX RSSI %d SNR %.1d", dataBuffer.data.lmic.rssi, dataBuffer.data.lmic.snr);

#if (USE_OTA)
      availableModules = availableModules + "OTA ";
#endif

#if (USE_BLE_SCANNER)
      availableModules = availableModules + "BLE ";
#endif

#if (USE_MQTT)
      availableModules = availableModules + "MQTT ";
#endif

#if (USE_CAYENNE)
      availableModules = availableModules + "CAY ";
#endif

#if (USE_GYRO)
      availableModules = availableModules + "GYRO ";
#endif

#if (USE_WEBSERVER)
      availableModules = availableModules + "WEB ";
#endif

      if (dataBuffer.data.wlan)
      {
        availableModules = availableModules + "WLAN ";
      }

      
      //sprintf(sbuf, "%s", availableModules);
      //u8g2.drawStr(1, 20, sbuf);
      //ESP_LOGI(TAG, "Modules: %s", sbuf);

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

    case PAGE_GPS:

      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "GPS");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Sats:%.2d", gps.tGps.satellites.value());
      u8g2.setCursor(64, 30);
      u8g2.printf("%02d:%02d:%02d", gps.tGps.time.hour(), gps.tGps.time.minute(), gps.tGps.time.second());

      u8g2.setCursor(1, 40);
      u8g2.printf("Alt:%.4g m", gps.tGps.altitude.meters());
      u8g2.setCursor(1, 60);
      u8g2.printf("Dist:%.0f m Lat %.0f", dataBuffer.data.gps_distance, dataBuffer.data.gps_old.lat());
      break;

    case PAGE_SOLAR:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Solar Panel");
      u8g2.setFont(u8g2_font_profont11_tr);

#if (HAS_INA)
      u8g2.setCursor(1, 30);
      u8g2.printf("Sol: %.2fV %.0fmA ", dataBuffer.data.panel_voltage, dataBuffer.data.panel_current);
#endif

#if (HAS_PMU)
      u8g2.setCursor(1, 40);
      u8g2.printf("Bus+: %.2fV %.0fmA ", dataBuffer.data.bus_voltage, dataBuffer.data.bus_current);
#else
      u8g2.setCursor(1, 40);
      u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif

      break;

    case PAGE_BAT:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Battery");
      u8g2.setFont(u8g2_font_profont11_tr);

#if (HAS_PMU)
      u8g2.setCursor(1, 30);
      u8g2.printf("Bat+: %.2fV %.0fmA ", dataBuffer.data.bat_voltage, dataBuffer.data.bat_charge_current);

      u8g2.setCursor(1, 40);
      u8g2.printf("Bat-: %.2fV %.0fmA ", dataBuffer.data.bat_voltage, dataBuffer.data.bat_discharge_current);

      u8g2.setCursor(1, 50);
      u8g2.printf("Fuel: %.0f mAh ", dataBuffer.data.bat_DeltamAh);
#else
      u8g2.setCursor(1, 40);
      u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif

      break;

    case PAGE_SENSORS:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Sensors");

      u8g2.setFont(u8g2_font_profont12_tr);
      u8g2.setCursor(1, 30);
      u8g2.printf("Temp: %.2f C %.0f hum ", dataBuffer.data.temperature, dataBuffer.data.humidity);
      u8g2.setCursor(1, 45);
      u8g2.printf("CPU Temp: %.2f C ", dataBuffer.data.cpu_temperature);
      u8g2.setCursor(1, 60);
      u8g2.printf("ADC: %d", dataBuffer.data.potentiometer_a);
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

    case PAGE_CORONA:

      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Corona Count");

      u8g2.setFont(u8g2_font_ncenB24_tr);
      u8g2.setCursor(40, 52);
      u8g2.printf("%i", dataBuffer.data.CoronaDeviceCount);

      break;

    case PAGE_SLEEP:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 15, "Sleep");

      u8g2.setFont(u8g2_font_profont11_tr);

#if (HAS_PMU)
      u8g2.setCursor(1, 25);
      u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
      u8g2.setCursor(1, 35);
      u8g2.printf("Fuel: %.0f mAh ", dataBuffer.data.bat_DeltamAh);
#else
      u8g2.setCursor(1, 25);
      u8g2.printf("Bat: %.2fV", dataBuffer.data.bat_voltage);
#endif

      if (dataBuffer.data.MotionCounter <= 0)
      {
        u8g2.drawStr(1, 55, "Inactivity");
      }

      if (dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT)
      {
        u8g2.drawStr(1, 55, "TX ");
      }

      u8g2.setCursor(1, 64);
      u8g2.printf("Sleeping for %3d min", dataBuffer.settings.sleep_time);

      drawSymbol(60, 12, SUN);
      break;
    }

    //---------------------------------
    //----------   Footer   -----------
    //---------------------------------
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.setCursor(100, 64);
    u8g2.printf("%2d", dataBuffer.data.MotionCounter);

    u8g2.setCursor(115, 64);
    if (dataBuffer.data.pictureLoop)
      u8g2.printf("P");

    u8g2.sendBuffer();
    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }
}

DataBuffer::DataBuffer()
{
}

void DataBuffer::set(deviceStatus_t input)
{
  data = input;
}

void DataBuffer::get()
{
}

DataBuffer dataBuffer;
deviceStatus_t sensorValues;