
#include "jsutilities.h"

// Local logging tag
static const char TAG[] = "";

#define SSD1306_PRIMARY_ADDRESS (0x3D)
#define SSD1306_SECONDARY_ADDRESS (0x3C)
#define PCA9685_PRIMARY_ADDRESS (0x40)
#define BME_PRIMARY_ADDRESS (0x77)
#define BME_SECONDARY_ADDRESS (0x76)
#define AXP192_PRIMARY_ADDRESS (0x34)
#define MCP_24AA02E64_PRIMARY_ADDRESS (0x50)
#define QUECTEL_GPS_PRIMARY_ADDRESS (0x10)
#define ADXL345 (0x53)
#define IP5306_ADDR (0X75)
#define INA219_PRIMARY_ADDRESS (0X40)
#define INA219_SECONDARY_ADDRESS (0X41)

int i2c_scan(void)
{

  int i2c_ret, addr;
  int devices = 0;

  ESP_LOGI(TAG, "Starting I2C bus scan...");

  for (addr = 8; addr <= 119; addr++)
  {
    // scan i2c bus with no more to 100KHz
    Wire.beginTransmission(addr);
    Wire.write(addr);
    i2c_ret = Wire.endTransmission();
    if (i2c_ret == 0)
    {
      devices++;

      switch (addr)
      {
      case INA219_PRIMARY_ADDRESS:
        ESP_LOGI(TAG, "0x%X: INA219 Voltage+Current detector", addr);
        break;

        switch (addr)
        {
        case INA219_SECONDARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: INA219 Voltage+Current detector", addr);
          break;

          // switch (addr) {
          // case INA3221_ADDRESS:
          // ESP_LOGI(TAG, "0x%X: INA 3221 Voltage+Current detector", addr);
          //   break;

        case PCA9685_PRIMARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: PCA9685 PWM Servo driver", addr);
          break;

        case SSD1306_PRIMARY_ADDRESS:
        case SSD1306_SECONDARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: SSD1306 Display controller", addr);
          break;

        case BME_PRIMARY_ADDRESS:
        case BME_SECONDARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: Bosch BME MEMS", addr);
          break;

        case AXP192_PRIMARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: AXP192 power management", addr);

          break;

        case QUECTEL_GPS_PRIMARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: Quectel GPS", addr);
          break;

        case MCP_24AA02E64_PRIMARY_ADDRESS:
          ESP_LOGI(TAG, "0x%X: 24AA02E64 serial EEPROM", addr);
          break;

        case ADXL345:
          ESP_LOGI(TAG, "0x%X: ADXL345 3 Axis Accel", addr);
          break;

        case IP5306_ADDR:
          ESP_LOGI(TAG, "0x%X: IP5306 power management", addr);
          break;

        default:
          ESP_LOGI(TAG, "0x%X: Unknown device", addr);
          break;
        }
      } // switch
    }   // for loop

    // Set back to 400KHz
    Wire.setClock(400000);

    ESP_LOGI(TAG, "I2C scan done, %u devices found.", devices);

    return devices;
  }
}

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

#if (BOARD_HAS_PSRAM)
    ESP_LOGI(TAG, "SPIRam Total heap %d, SPIRam Free Heap %d",
             ESP.getPsramSize(), ESP.getFreePsram());

#endif

    ESP_LOGI(TAG, "ChipRevision %d, Cpu Freq %d, SDK Version %s",
             ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    ESP_LOGI(TAG, "Flash Size %d, Flash Speed %d", ESP.getFlashChipSize(),
             ESP.getFlashChipSpeed());
  }

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

  void print_wakeup_touchpad()
  {
    touch_pad_t pin;
    switch (esp_sleep_get_touchpad_wakeup_status())
    {
    case 0:
      Serial.println("Touch detected on GPIO 4");
      break;
    case 1:
      Serial.println("Touch detected on GPIO 0");
      break;
    case 2:
      Serial.println("Touch detected on GPIO 2");
      break;
    case 3:
      Serial.println("Touch detected on GPIO 15");
      break;
    case 4:
      Serial.println("Touch detected on GPIO 13");
      break;
    case 5:
      Serial.println("Touch detected on GPIO 12");
      break;
    case 6:
      Serial.println("Touch detected on GPIO 14");
      break;
    case 7:
      Serial.println("Touch detected on GPIO 27");
      break;
    case 8:
      Serial.println("Touch detected on GPIO 33");
      break;
    case 9:
      Serial.println("Touch detected on GPIO 32");
      break;
    default:
      Serial.println("Wakeup not by touchpad");
      break;
    }
  }