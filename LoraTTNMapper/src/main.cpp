// T-Beam specific hardware
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

//---------------------------------------------------------
// Upload data to ESP 32 SPIFFS
// 0. !! close Serial Monitor  !!
// 1. Open Platform IO extension --> Miscellaneous
// 2. Open PlatformIO Core CLI
// 3. pio run -t uploadfs
//---------------------------------------------------------
#include "globals.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <tasks.h>
#if (USE_OTA)
#include <ArduinoOTA.h>
#endif

// Defaults to window size 10
#if (USE_POTI)
// AnalogSmooth Poti_A = AnalogSmooth();
#endif

// static const char TAG[] = __FILE__;
static const char TAG[] = "";

AnalogSmooth smooth_temp = AnalogSmooth();
AnalogSmooth smooth_discur = AnalogSmooth();
AnalogSmooth smooth_batvol = AnalogSmooth();

#if (USE_GPS)
#include <SparkFun_Ublox_Arduino_Library.h> //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS myGPS;
int state = 0; // steps through states
HardwareSerial SerialGPS(1);

void setup_gps_reset()
{

  SerialGPS.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
  ESP_LOGI(TAG, "Configure GPS Module");
  delay(100);

  // http://aprs.gids.nl/nmea/#vtg    NEMA Message overview
  if (myGPS.begin(SerialGPS))
  {
    Serial.println("Connected to GPS");
    myGPS.setUART1Output(COM_TYPE_NMEA); // Set the UART port to output NMEA only

    Serial.println("GPS serial connected, output set to NMEA");
    myGPS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_UART1);
    // myGPS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_UART1);
    myGPS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_UART1);
    myGPS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_UART1);

    // myGPS.disableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);
    myGPS.enableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);

    myGPS.enableNMEAMessage(UBX_NMEA_GGA, COM_PORT_UART1);

    myGPS.disableNMEAMessage(UBX_NMEA_ZDA, COM_PORT_UART1);
    // myGPS.enableNMEAMessage(UBX_NMEA_ZDA, COM_PORT_UART1);
    myGPS.saveConfiguration(); // Save the current settings to flash and BBR
  }
  delay(1000);
}
#endif

//---------------------------------------------------------------------------------------------------------
//   SPIFF File system
//---------------------------------------------------------------------------------------------------------


void setup_filesystem()
{

  const bool delete_files = false;

  ESP_LOGI(TAG, "Mounting SPIFF Filesystem");
  // External File System Initialisation
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "An Error has occurred while mounting SPIFFS");
    return;
  }
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  // Loop through all files in the root directory

  while (file)
  {
    String fileName = file.name();
    ESP_LOGI(TAG, "%s", file.name());
    if (delete_files)
    {
      if (fileName.startsWith("maclist"))
      {
        // Delete file

        if (SPIFFS.remove("/" + fileName))
        {
          Serial.println("Deleted file: " + fileName);
        }
        else
        {
          Serial.println("Failed to delete file: " + fileName);
        }
      }
    }
    file = root.openNextFile();
  }
}

#if (USE_SPIFF_LOGGING)
static char log_print_buffer[512];

int vprintf_into_spiffs(const char *szFormat, va_list args)
{
  // write evaluated format string into buffer
  int ret = vsnprintf(log_print_buffer, sizeof(log_print_buffer), szFormat, args);

  dataBuffer.settings.log_print_buffer = log_print_buffer;

  Serial.print("via link");
  Serial.println(log_print_buffer);

  // output is now in buffer. write to file.
  if (ret >= 0)
  {
    if (!SPIFFS.exists("/LOGS.txt"))
    {
      File writeLog = SPIFFS.open("/LOGS.txt", FILE_WRITE);
      if (!writeLog)
        Serial.println("Couldn't open spiffs_log.txt");
      delay(50);
      writeLog.close();
    }

    File spiffsLogFile = SPIFFS.open("/LOGS.txt", FILE_APPEND);
    // debug output
    // printf("[Writing to SPIFFS] %.*s", ret, log_print_buffer);
    spiffsLogFile.write((uint8_t *)log_print_buffer, (size_t)ret);
    // to be safe in case of crashes: flush the output
    spiffsLogFile.flush();
    spiffsLogFile.close();
  }
  return ret;
}
#endif

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
TaskHandle_t t_cyclic1m_HandlerTask = NULL;
TaskHandle_t t_cyclic2m_HandlerTask = NULL;
TaskHandle_t t_sunTracker_HandlerTask = NULL;

Ticker sleepTicker;
Ticker displayTicker;
Ticker displayMoveTicker;
Ticker sendMessageTicker;
Ticker sendCycleTicker;
Ticker LORAsendMessageTicker;
Ticker sunTicker;

#if (USE_OTA)
void setup_ota()
{

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(DEVICE_NAME);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type);
#if (USE_DISPLAY)
                 showPage(PAGE_OTA);
#endif
               })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
#endif

void touch_callback()
{
  // placeholder callback function
}

//---------------------------------------------------------------------------------
// Send Messages via Cayenne or MQTT
//---------------------------------------------------------------------------------

void t_mqtt_cycle()
{

#if (USE_CAYENNE)
  if (WiFi.status() == WL_CONNECTED)
    Cayenne_send();
#endif

#if (USE_MQTT)
  if (WiFi.status() == WL_CONNECTED)
  {

#if (USE_MQTT_SENSORS)
    mqtt_send();
#endif
#if (USE_MQTT_TRAIN)
    if (dataBuffer.data.potentiometer_a_changed)
    {
      mqtt_send_lok(1, dataBuffer.data.potentiometer_a, 1);
      dataBuffer.data.potentiometer_a_changed = false;
    }
#endif
#endif
  }

  //---------------------------------------------------------------------------------
  // Get sensor values and update display
  //---------------------------------------------------------------------------------
  void t_sunTracker() // Intervall: Display Refresh
  {
    Serial.println();
    Serial.println();
    ESP_LOGI(TAG, "------------------------------------------------");
    ESP_LOGI(TAG, "Sun Tracker");
    ESP_LOGI(TAG, "------------------------------------------------");

#if (HAS_PMU)
    AXP192_get_mpp();
#endif

#if (USE_SUN_POSITION)
#if (USE_PWM_SERVO)
    servo_move_to_sun();
    Serial.println();
    Serial.println();
#endif
#endif
  }

  void t_cyclic() // Intervall: Display Refresh
  {

    Serial.println();
    Serial.println();
    Serial.println();

    handle_time(); // get time and write to dataBuffer

#if (USE_BLE_SERVER)
    ble_send();
#endif

    dataBuffer.data.freeheap = ESP.getFreeHeap();
    dataBuffer.data.cpu_temperature = (temprature_sens_read() - 32) / 1.8;
    // ESP_LOGI(TAG, "ESP free heap: %d", dataBuffer.data.freeheap);
    dataBuffer.data.aliveCounter++;

    //   I2C opperations
    if (!I2C_MUTEX_LOCK())
      ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    else
    {
#if (USE_BME280)
      float temp = bme.readTemperature();
      if (temp < 60)
      {
        dataBuffer.data.temperature = smooth_temp.smooth(temp);
      }
      dataBuffer.data.humidity = bme.readHumidity();
      ESP_LOGI(TAG, "Temp %.2f Temp smooth %.2f Humidity %.2fV", temp, dataBuffer.data.temperature, dataBuffer.data.humidity);
#endif

#if (HAS_PMU)
      dataBuffer.data.bus_voltage = pmu.getVbusVoltage() / 1000;
      dataBuffer.data.bus_current = pmu.getVbusCurrent();
      dataBuffer.data.bat_voltage = smooth_batvol.smooth(pmu.getBattVoltage() / 1000);
      dataBuffer.data.bat_charge_current = pmu.getBattChargeCurrent();
      dataBuffer.data.bat_discharge_current = pmu.getBattDischargeCurrent();

      // recalculate charge current
      if (dataBuffer.data.bat_charge_current == 0)
      {
        if (dataBuffer.data.bat_discharge_current > 0)
        {
          dataBuffer.data.bat_charge_current = dataBuffer.data.bat_discharge_current * -1;
        }
      }
      dataBuffer.data.bat_ChargeCoulomb = pmu.getBattChargeCoulomb() / 3.6;
      dataBuffer.data.bat_DischargeCoulomb = pmu.getBattDischargeCoulomb() / 3.6;
      dataBuffer.data.bat_DeltamAh = pmu.getCoulombData();
      dataBuffer.data.bat_max_charge_curr = pmu.getChargeControlCur();
      ESP_LOGI(TAG, "PMU BusVoltage %.2fV BatVoltage %.2fV Fuel: %.0f mAh  BatCharge: %.0f mAh", dataBuffer.data.bus_voltage, dataBuffer.data.bat_voltage, dataBuffer.data.bat_DeltamAh, dataBuffer.data.bat_charge_current);

      if ((dataBuffer.data.bus_voltage != 0) || (dataBuffer.data.bat_voltage != 0))
        dataBuffer.data.pmu_data_available = true;
      else
        dataBuffer.data.pmu_data_available = false;
#else
    dataBuffer.data.bat_voltage = read_voltage() / 1000;
#endif

#if (HAS_INA3221)
      print_ina3221();
      dataBuffer.data.panel_voltage = ina3221.getBusVoltage_V(1);
      dataBuffer.data.panel_current = ina3221.getCurrent_mA(1);
#endif

#if (HAS_INA219)
      if (ina219.getBusVoltage_V() < 50)
      {
        ESP_LOGI(TAG, "Cut-Off Solar Panel at: %.2fV", dataBuffer.data.ina219[0].voltage);
        panel_switch_off();
        delay(400);
        ESP_LOGI(TAG, "Solar Panel OFF: %.2fV ", ina219.getBusVoltage_V());
        panel_switch_on();
        ESP_LOGI(TAG, "Solar Panel ON: %.2fV ", ina219.getBusVoltage_V());
      }

      print_ina219();
      dataBuffer.data.ina219[0].voltage = ina219.getBusVoltage_V();
      dataBuffer.data.ina219[0].current = ina219.getCurrent_mA();
      dataBuffer.data.ina219[0].power = ina219.getPower_mW();
      dataBuffer.data.ina219[0].shuntVoltage = ina219.getShuntVoltage_mV();

#endif

#if (USE_MPU6050)

      // unsigned long startTime = millis(); // Zeit vor dem Funktionsaufruf
      get_mpu6050_data();
      // unsigned long endTime = millis();                  // Zeit nach dem Funktionsaufruf
      // unsigned long executionTime = endTime - startTime; // Berechne die Laufzeit der Funktion
      // Serial.print("Laufzeit get_mpu6050_data() ");
      // Serial.print(executionTime);
      // Serial.println(" Millisekunden");

#endif

#if (USE_VL53L1X)
      // startTime = millis(); // Zeit vor dem Funktionsaufruf
      get_VL53L1X_data();
      // endTime = millis();                  // Zeit nach dem Funktionsaufruf
      // executionTime = endTime - startTime; // Berechne die Laufzeit der Funktion
      // Serial.print("Laufzeit get_VL53L1X_data() ");
      // Serial.print(executionTime);
      // Serial.println(" Millisekunden");

#endif

#if (USE_CAMERA)
      showCameraImageTFT();
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

// Calculate Soil Moisture
#if (USE_SOIL_MOISTURE)
    if (dataBuffer.data.potentiometer_a_changed)
    {
      dataBuffer.data.soil_moisture = (float)dataBuffer.data.potentiometer_a / 1000;
      dataBuffer.data.potentiometer_a_changed = false;
      ESP_LOGI(TAG, "Soil moisture changed %.2f ", dataBuffer.data.soil_moisture);
    }
    ESP_LOGI(TAG, "Soil moisture %.2f ", dataBuffer.data.soil_moisture);
#endif

    // esp_log_write(ESP_LOG_INFO, TAG, "BME280  %.1f C/%.1f% \n", dataBuffer.data.temperature, dataBuffer.data.humidity);

#if (CYCLIC_SHOW_LOG)
    ESP_LOGI(TAG, "Runmode %d", dataBuffer.data.runmode);
    ESP_LOGI(TAG, "Poti %.2f", dataBuffer.data.potentiometer_a);
    ESP_LOGI(TAG, "BME280  %.1f C/%.1f%", dataBuffer.data.temperature, dataBuffer.data.humidity);
#if (HAS_PMU)
    AXP192_showstatus();
#endif
#endif

#if (USE_WIFICOUNTER)
    if (dataBuffer.wificounter.active)
    {
      dataBuffer.wificounter.count = wifi_count_get();
      dataBuffer.wificounter.count5 = getMacListCountlastMinutes(5);
    }
#endif
  }

  //---------------------------------------------------------------------------------
  // Check deep sleep conditions
  //---------------------------------------------------------------------------------
  void t_sleep()
  {
    printLocalTime();
#if (USE_GPS_MOTION)
    gps.getDistance();
#endif

//------------------------------------------------------------
// Calc deep sleep time
//------------------------------------------------------------
#if (ESP_SLEEP)
#if (AUTO_POWER_SAVE)
#if (TIME_TO_SLEEP_BAT_HIGH)
#if (BAT_HIGH)
#if (BAT_LOW)
    if ((dataBuffer.data.bat_voltage * 10) < BAT_LOW)
    {
      dataBuffer.settings.sleep_time = TIME_TO_SLEEP_BAT_LOW;
    }
    else
    {
      if ((dataBuffer.data.bat_voltage * 10) > BAT_HIGH)
      {
        dataBuffer.settings.sleep_time = TIME_TO_SLEEP_BAT_HIGH;
      }
      else
      {
        dataBuffer.settings.sleep_time = TIME_TO_SLEEP_BAT_MID;
      }
    }
#endif
#endif
#endif
#else
    // Use default deep sleep time
    dataBuffer.settings.sleep_time = TIME_TO_SLEEP;
#endif
#endif

//-----------------------------------------------------------------------------
// Determin if device should go to DeepSleep
//-----------------------------------------------------------------------------
#if (ESP_SLEEP)
    dataBuffer.data.MotionCounter = dataBuffer.data.MotionCounter - 1;

#if (USE_FASTLED)
    if (dataBuffer.data.MotionCounter < TIME_TO_NEXT_SLEEP_WITHOUT_MOTION)
    {
      LED_showSleepCounter();
    }
#endif

// Check if if solar panel is adjusted
#if (USE_SUN_POSITION)
    if (dataBuffer.settings.sunTrackerPositionAdjusted)
      dataBuffer.data.MotionCounter = 0;
#endif

    // Check if number of Lora-TX events has been reached
    if (dataBuffer.data.txCounter >= SLEEP_AFTER_N_TX_COUNT)
      dataBuffer.data.MotionCounter = 0;

// Check if GPS position has been changed
#if (USE_GPS_MOTION)
    if (dataBuffer.data.gps_distance > GPS_MOTION_DISTANCE)
    {
      dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;
      gps.resetDistance();
    }
#endif

#if (USE_BLE_SERVER)
    if (dataBuffer.data.ble_device_connected)
    {
      ESP_LOGI(TAG, "No Deep Sleep, BLE still connected");
      dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;
    }
#endif

    // Goto Deep Sleep
    ESP_LOGI(TAG, "Deep sleep in %d min.", dataBuffer.data.MotionCounter);
    if (dataBuffer.data.MotionCounter <= 0)
    {
      ESP32_sleep();
    }

#endif
  }

  void setup_wifi()
  {
#if (USE_WIFI)
    // Set your Static IP address
    // IPAddress local_IP(192, 168, 1, 111);
    // IPAddress gateway(192, 168, 1, 1);
    // IPAddress subnet(255, 255, 0, 0);

    Serial.println();
    ESP_LOGE(TAG, "-----------  Setup Wifi   -----------");
    IPAddress ip;
    // Configures static IP address
    // if (!WiFi.config(local_IP, gateway, subnet))
    //{
    //  Serial.println("STA Failed to configure");
    //}

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, wifiPassword);

    ESP_LOGE(TAG, "Connecting to WiFi..");
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
      dataBuffer.data.ip_address = ip.toString();
      Serial.print("IP: ");
      Serial.println(dataBuffer.data.ip_address);
    }
    else
    {
      // Turn off WiFi if no connection was made
      ESP_LOGE(TAG, "WIFI OFF");
      dataBuffer.data.wlan = false;
      WiFi.mode(WIFI_OFF);
    }
#endif
  }

  void setup_wifi_AP()
  {
#if (USE_WIFI_AP)
    const char *ssid_ap = "ESP32-Access-Point";
    const char *password_ap = "Linde+123";
    ESP_LOGI(TAG, "-----------  Setup WIFI Access Point  -----------");
    WiFi.softAP(ssid_ap, password_ap);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
#endif
  }

  void setup()
  {
    Serial.begin(115200);

// LED Sunrise
#ifdef HAS_LED
    ledcSetup(0, 10000, 8);
    ledcAttachPin(HAS_LED, 0);
    for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
    {
      ledcWrite(0, dutyCycle);
      delay(5);
    }
#endif

    //--------------------------------------------------------------------
    // Load Settings
    //--------------------------------------------------------------------
    setup_filesystem();
    // loadConfiguration();

    //--------------------------------------------------------------------
    // Logging
    //--------------------------------------------------------------------
#if (USE_SPIFF_LOGGING)

    if (SPIFFS.exists("/LOGS.txt"))
    {
      SPIFFS.remove("/LOGS.txt");
    }
    esp_log_set_vprintf(&vprintf_into_spiffs);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // write into log
    esp_log_write(ESP_LOG_INFO, TAG, "Hello World2\n");
    esp_log_write(ESP_LOG_INFO, TAG, "starting...\n");
#endif

    dataBuffer.data.runmode = 0;

    // Increment boot number and print it every reboot
    ++bootCount;
    dataBuffer.data.bootCounter = bootCount;

    // Aditional Info
    print_wakeup_reason();
    display_chip_info();
    // Serial.println(dataBuffer.to_json());
    // Serial.println(dataBuffer.getError());

    // I2c access management for RTOS bus access
    I2Caccess = xSemaphoreCreateMutex(); //
    assert(I2Caccess != NULL);
    I2C_MUTEX_UNLOCK();

// Setup I2C Communication
#if (I2C_SDA)
    Wire.begin(I2C_SDA, I2C_SCL, 100000);
#else
  Wire.begin(SDA, SCL, 400000);
#endif
    delay(500);

#if (HAS_IP5306)
    setupPowerIP5306();
#endif

#if (HAS_PMU)
    AXP192_init();
    delay(100);
    AXP192_showstatus();
    delay(100);
    AXP192_power_gps(ON);
    delay(1000);
#endif

    i2c_scan();

#if (HAS_INA3221 || HAS_INA219 || USE_BME280)
    ESP_LOGI(TAG, "-----------  Setup I2c devices   -----------");
    setup_i2c_sensors();
#endif

    dataBuffer.data.txCounter = 0;
    dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;
    dataBuffer.data.firmware_version = VERSION;
    dataBuffer.data.tx_ack_req = 0;

#if (USE_DISPLAY)
    setup_display();
    showPage(PAGE_BOOT);
#endif

#if (USE_WIFI_AP)
    setup_wifi_AP();
#else
  setup_wifi();
#endif
    calibrate_voltage();
    delay(100);

#if (USE_SERIAL_BT || USE_BLE_SCANNER || USE_BLE_SERVER)
#else
  // Turn off Bluetooth
  ESP_LOGI(TAG, "Bluetooth OFF");
  btStop();
#endif

#if (USE_MQTT)
    setup_mqtt();
#endif

#if (USE_GYRO)
    setup_gyro();
#endif

#if (USE_GPS)
#if (HAS_GPS)
    ESP_LOGI(TAG, "TinyGPS+ version %s", TinyGPSPlus::libraryVersion());
#endif

    // setup_gps_reset(); // Hard reset
    gps.init();
    // gps.softwareReset();
    gps.wakeup();
    delay(500); // Wait for GPS beeing stable
#endif

#if (HAS_LORA)
    setup_lora();
    lora_queue_init();
    delay(500);
#endif

#if (USE_BUTTON)
#ifdef BUTTON_PIN
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    button_init(BUTTON_PIN);
#endif
#endif

#if (USE_WEBSERVER)
    setup_webserver();
    if (WiFi.status() == WL_CONNECTED)
    {
      ws.onEvent(onWsEvent);
      server.addHandler(&ws);
    }
#endif

#if (USE_FASTLED)
    setup_FastLed();
    delay(50);
    LED_wakeup();
#endif

#if (HAS_LORA)
    t_enqueue_LORA_messages();
#endif

#if (USE_POTI)
    poti_setup_RTOS();
#endif

#if (USE_DISTANCE_SENSOR_HCSR04)
    setup_hcsr04_rtos();
#endif

#if (USE_MPU6050)
    setup_mpu6050();
#endif

#if (USE_VL53L1X)
    setup_VL53L1X();
#endif

#if (USE_I2C_MICROPHONE)
    setup_sound_rtos();
#endif

#if (USE_CAMERA)
    setupCam();
#endif

#if (USE_PWM_SERVO)
    setup_servo_pwm();
    panel_switch_on();
    servo_move_to_last_position();
#endif

#if (USE_BLE_SERVER)
    setup_ble();
#endif

    // Get date/time from Internet or GPS
    setup_time();

#if (USE_WIFICOUNTER)
    pinMode(WIFICOUNTER_ACTIVE_PIN, INPUT_PULLUP);      // Set GPIO2 as an input
    int pinState = digitalRead(WIFICOUNTER_ACTIVE_PIN); // Read the state of GPIO2

    if (pinState == HIGH)
    {
      Serial.println("GPIO2 is HIGH");
    }
    else
    {
      Serial.println("GPIO2 is LOW: Activate WIFI Counter");
      dataBuffer.wificounter.active = true;
      setup_wifi_counter();
    }

#endif

#if (USE_OTA)
    setup_ota();
#endif
    // get sensor values once
    t_cyclic();   

    //-------------------------------------------------------------------------------
    // Setup Tasks
    //-------------------------------------------------------------------------------
    log_display("Starting Tasks");

    sleepTicker.attach(60, t_sleep);
    displayTicker.attach(displayRefreshIntervall, t_cyclic);
    displayMoveTicker.attach(displayMoveIntervall, t_moveDisplay);

#if (USE_MQTT)
    sendCycleTicker.attach(sendMqttIntervall, t_mqtt_cycle);
#endif

#if (USE_SUN_POSITION)
    sunTicker.attach(sunTrackerRefreshIntervall, t_sunTracker);
#endif

#if (HAS_LORA)
    sendMessageTicker.attach(LORAenqueueMessagesIntervall, t_enqueue_LORA_messages);
#endif

    //-------------------------------------------------------------------------------
    // Websocket Task
    //-------------------------------------------------------------------------------
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

    ESP_LOGI(TAG, "Starting cyclic tasks every 2 minutes...");
    xTaskCreatePinnedToCore(t_cyclicRtos2m,      // task function
                            "t_cyclicRtos2m",    // name of task
                            4096,            // stack size of task
                            (void *)1,       // parameter of the task
                            2,               // priority of the task
                            &t_cyclic2m_HandlerTask, // task handle
                            0);              // CPU core

    dataBuffer.data.runmode = 1; // Switch from Terminal Mode to page Display
    
    Serial.println();
    ESP_LOGI(TAG, "#----------------------------------------------------------#");
    ESP_LOGI(TAG, "Setup done");
    ESP_LOGI(TAG, "#----------------------------------------------------------#");
    Serial.println();

    //---------------------------------------------------------------
    // Watchdog
    //---------------------------------------------------------------
    // esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    // esp_task_wdt_add(NULL); //add current thread to WDT watch
    // enableLoopWDT();
    // ESP_LOGI(TAG, "Watchdog timeout %d seconds", WDT_TIMEOUT);
  }

  //---------------------------------------------------------------
  // MAIN LOOP
  //---------------------------------------------------------------
  void loop()
  {
    // esp_task_wdt_reset(); //reset timer ...feed watchdog
    // feedLoopWDT();

#if (HAS_LORA)
    os_runloop_once();
#endif

#if (USE_MQTT)
#if (USE_WIFI_AP)
#else
    mqtt_loop();
#endif
#endif

#if (USE_BUTTON)
    readButton();
#endif

#if (USE_OTA)
    ArduinoOTA.handle();
#endif

#if (USE_WIFICOUNTER)
    if (dataBuffer.wificounter.active)
    {
      wifi_counter_loop();
    }

#endif
  }