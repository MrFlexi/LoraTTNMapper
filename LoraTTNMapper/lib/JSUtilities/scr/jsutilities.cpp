
#include "jsutilities.h"

// Local logging tag
//static const char TAG[] = __FILE__;

#define SSD1306_PRIMARY_ADDRESS (0x3D)
#define SSD1306_SECONDARY_ADDRESS (0x3C)
#define BME_PRIMARY_ADDRESS (0x77)
#define BME_SECONDARY_ADDRESS (0x76)
#define AXP192_PRIMARY_ADDRESS (0x34)
#define MCP_24AA02E64_PRIMARY_ADDRESS (0x50)
#define QUECTEL_GPS_PRIMARY_ADDRESS (0x10)
#define ADXL345 (0x53)


int i2c_scan(void) {

  int i2c_ret, addr;
  int devices = 0;

  Serial.println( "Scanning I2C");
  ESP_LOGI(TAG, "Starting I2C bus scan...");

  for (addr = 8; addr <= 119; addr++) {

    // scan i2c bus with no more to 100KHz
    Wire.begin(SDA, SCL, 100000);
    Wire.beginTransmission(addr);
    Wire.write(addr);
    i2c_ret = Wire.endTransmission();

    if (i2c_ret == 0) {
      devices++;

      switch (addr) {
      case INA3221_ADDRESS:
      ESP_LOGI(TAG, "0x%X: INA 3221 Voltage+Current detector", addr);
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

      default:
        ESP_LOGI(TAG, "0x%X: Unknown device", addr);
        break;
      }
    } // switch
  }   // for loop

  ESP_LOGI(TAG, "I2C scan done, %u devices found.", devices);

  return devices;
}


/**************************************************************************/
/*! 
    @brief  Sends a single command byte over I2C
*/
/**************************************************************************/
void SDL_Arduino_INA3221::wireWriteRegister (uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(INA3221_i2caddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // Register
    Wire.write((value >> 8) & 0xFF);       // Upper 8-bits
    Wire.write(value & 0xFF);              // Lower 8-bits
  #else
    Wire.send(reg);                        // Register
    Wire.send(value >> 8);                 // Upper 8-bits
    Wire.send(value & 0xFF);               // Lower 8-bits
  #endif
  Wire.endTransmission();
}

/**************************************************************************/
/*! 
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
void SDL_Arduino_INA3221::wireReadRegister(uint8_t reg, uint16_t *value)
{

  Wire.beginTransmission(INA3221_i2caddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // Register
  #else
    Wire.send(reg);                        // Register
  #endif
  Wire.endTransmission();
  
  delay(1); // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(INA3221_i2caddr, (uint8_t)2);  
  #if ARDUINO >= 100
    // Shift values to create properly formed integer
    *value = ((Wire.read() << 8) | Wire.read());
  #else
    // Shift values to create properly formed integer
    *value = ((Wire.receive() << 8) | Wire.receive());
  #endif
}

//
void SDL_Arduino_INA3221::INA3221SetConfig(void)
{
 
 
  // Set Config register to take into account the settings above
  uint16_t config = INA3221_CONFIG_ENABLE_CHAN1 |
                    INA3221_CONFIG_ENABLE_CHAN2 |
                    INA3221_CONFIG_ENABLE_CHAN3 |
                    INA3221_CONFIG_AVG1 |
                    INA3221_CONFIG_VBUS_CT2 |
                    INA3221_CONFIG_VSH_CT2 |
                    INA3221_CONFIG_MODE_2 |
                    INA3221_CONFIG_MODE_1 |
                    INA3221_CONFIG_MODE_0;
  wireWriteRegister(INA3221_REG_CONFIG, config);
}

/**************************************************************************/
/*! 
    @brief  Instantiates a new SDL_Arduino_INA3221 class
*/
/**************************************************************************/
SDL_Arduino_INA3221::SDL_Arduino_INA3221(uint8_t addr, float shuntresistor) {
    
    INA3221_i2caddr = addr;
    INA3221_shuntresistor = shuntresistor;
 
}

/**************************************************************************/
/*! 
    @brief  Setups the HW (defaults to 32V and 2A for calibration values)
*/
/**************************************************************************/
void SDL_Arduino_INA3221::begin() {
  Wire.begin();    
  // Set chip to known config values to start
  INA3221SetConfig();
    
   // Serial.print("shut resistor="); Serial.println(INA3221_shuntresistor);
       // Serial.print("address="); Serial.println(INA3221_i2caddr);
    
}

/**************************************************************************/
/*! 
    @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
int16_t SDL_Arduino_INA3221::getBusVoltage_raw(int channel) {
  uint16_t value;
  wireReadRegister(INA3221_REG_BUSVOLTAGE_1+(channel -1) *2, &value);
//    Serial.print("BusVoltage_raw=");
//    Serial.println(value,HEX);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)(value );
}

/**************************************************************************/
/*! 
    @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
int16_t SDL_Arduino_INA3221::getShuntVoltage_raw(int channel) {
  uint16_t value;
  wireReadRegister(INA3221_REG_SHUNTVOLTAGE_1+(channel -1) *2, &value);
   // Serial.print("ShuntVoltage_raw=");
   // Serial.println(value,HEX);
  return (int16_t)value;
}


 
/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage in mV (so +-168.3mV)
*/
/**************************************************************************/
float SDL_Arduino_INA3221::getShuntVoltage_mV(int channel) {
  int16_t value;
  value = getShuntVoltage_raw(channel);
  return value * 0.005;
}

/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage in volts
*/
/**************************************************************************/
float SDL_Arduino_INA3221::getBusVoltage_V(int channel) {
  int16_t value = getBusVoltage_raw(channel);
  return value * 0.001;
}

/**************************************************************************/
/*! 
    @brief  Gets the current value in mA, taking into account the
            config settings and current LSB
*/
/**************************************************************************/
float SDL_Arduino_INA3221::getCurrent_mA(int channel) {
    float valueDec = getShuntVoltage_mV(channel)/INA3221_shuntresistor;
  
  return valueDec;
}


/**************************************************************************/
/*! 
    @brief  Gets the Manufacturers ID
*/
/**************************************************************************/
int SDL_Arduino_INA3221::getManufID()
{
  uint16_t value;
  wireReadRegister(0xFE, &value);
  return value;

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