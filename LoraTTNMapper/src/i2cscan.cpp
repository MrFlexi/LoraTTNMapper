// Basic config
//#include "globals.h"
#include <Wire.h>
#include "i2cscan.h"

// Local logging tag
//static const char TAG[] = __FILE__;

#define SSD1306_PRIMARY_ADDRESS (0x3D)
#define SSD1306_SECONDARY_ADDRESS (0x3C)
#define BME_PRIMARY_ADDRESS (0x77)
#define BME_SECONDARY_ADDRESS (0x76)
#define AXP192_PRIMARY_ADDRESS (0x34)
#define MCP_24AA02E64_PRIMARY_ADDRESS (0x50)
#define QUECTEL_GPS_PRIMARY_ADDRESS (0x10)


int i2c_scan(void) {

  int i2c_ret, addr;
  int devices = 0;

  Serial.println( "Scanning I2C");
  for (addr = 8; addr <= 119; addr++) {

    // scan i2c bus with no more to 100KHz
    Wire.begin(SDA, SCL, 100000);
    Wire.beginTransmission(addr);
    Wire.write(addr);
    i2c_ret = Wire.endTransmission();

    if (i2c_ret == 0) {
      devices++;

      switch (addr) {

      case SSD1306_PRIMARY_ADDRESS:
      case SSD1306_SECONDARY_ADDRESS:
        ESP_LOGI(TAG, "0x%X: SSD1306 Display", addr);
        break;

      
      case AXP192_PRIMARY_ADDRESS:
        ESP_LOGI(TAG, "0x%X: AXP192 power management", addr);
        break;


      case MCP_24AA02E64_PRIMARY_ADDRESS:
        ESP_LOGI(TAG, "0x%X: 24AA02E64 serial EEPROM", addr);
        break;

      default:
        ESP_LOGI(TAG, "0x%X: Unknown device", addr);
        break;
      }
    } // switch
  }   // for loop

  ESP_LOGI(TAG, "scan done, %u devices", devices);

  return devices;
}
