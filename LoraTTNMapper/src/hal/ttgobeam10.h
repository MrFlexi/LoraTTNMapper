// clang-format off
// upload_speed 921600
// board ttgo-t-beam

// T-Beam V10 has on board power management by AXP192 PMU chip:
//
// DCDC1 0.7-3.5V @ 1200mA -> OLED
// DCDC3 0.7-3.5V @ 700mA -> ESP32 (keep this on!)
// LDO1 30mA -> GPS Backup
// LDO2 200mA -> LORA
// LDO3 200mA -> GPS

// Wiring for I2C OLED display:
//
// Signal     Header   OLED
// 3V3         7       VCC
// GND         8       GND
// IO22(SCL)   9       SCL
// IO21(SDA)   10      SDA


#ifndef _TTGOBEAM_H
#define _TTGOBEAM_H

#include <stdint.h>

#define FASTLED_DATA_PIN        GPIO_NUM_2

#define POTI_PIN                GPIO_NUM_32
#define PMU_INT_PIN             GPIO_NUM_35     // AXP192 interrupt
#define BUTTON_PIN              GPIO_NUM_38     // middle on board button

#define POWER_RAIL_PIN          GPIO_NUM_15     // Power Rail
#define SERVO1_PIN              GPIO_NUM_13
#define SERVO2_PIN              GPIO_NUM_4

#define GPS_TX                  GPIO_NUM_34
#define GPS_RX                  GPIO_NUM_12



#define HAS_LORA                1               // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio        1               // HPD13A LoRa SoC
#define BOARD_HAS_PSRAM         1               // use extra 4MB external RAM

#define HAS_PMU                 1               // AXP192 power management chip
#define PMU_CHG_CURRENT         AXP1XX_CHARGE_CUR_550MA // battery charge current possible values (mA): 100/190/280/360/450/550/630/700/780/880/960/1000/1080/1160/1240/1320
#define PMU_CHG_CUTOFF          AXP202_TARGET_VOL_4_2V  // battery charge cutoff possible values (V): 4_1/4_15/4_2/4_36

#define HAS_LED NOT_A_PIN

// GPS settings
#define HAS_GPS         1                       // use on board GPS


// enable only if device has these sensors, otherwise comment these lines
// BME680 sensor on I2C bus
//#define HAS_BME 1 // Enable BME sensors in general
//#define HAS_BME680 SDA, SCL
//#define BME680_ADDR BME680_I2C_ADDR_PRIMARY // !! connect SDIO of BME680 to GND !!

// display (if connected)
//#define HAS_DISPLAY U8X8_SSD1306_128X64_NONAME_HW_I2C
//#define MY_OLED_SDA SDA
//#define MY_OLED_SCL SCL
//#define MY_OLED_RST U8X8_PIN_NONE
//#define DISPLAY_FLIP  1 // use if display is rotated

// user defined sensors (if connected)
//#define HAS_SENSORS 1 // comment out if device has user defined sensors

//#define DISABLE_BROWNOUT 1 // comment out if you want to keep brownout feature

#endif
