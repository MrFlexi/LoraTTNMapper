// clang-format off
// upload_speed 921600
// board ttgo-t-beam

#ifndef _TTGOBEAM_H
#define _TTGOBEAM_H

#include <stdint.h>

// Hardware related definitions for TTGO T-Beam board
// (only) for newer T-Beam version T22_V10
// pinouts taken from https://github.com/lewisxhe/TTGO-T-Beam

#define HAS_LORA                1               // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio        1               // HPD13A LoRa SoC
#define BOARD_HAS_PSRAM         1               // use extra 4MB external RAM
#define HAS_BUTTON              GPIO_NUM_38     // middle on board button
#define HAS_PMU                 1               // AXP192 power management chip
#define PMU_INT                 GPIO_NUM_35     // AXP192 interrupt
#define GYRO_INT_PIN            GPIO_NUM_4      // Gyroskop Movement Detection
#define FASTLED_DATA_PIN        GPIO_NUM_2
#define POTI_PIN                ADC1_GPIO32_CHANNEL

#define GYRO_INT_PIN            GPIO_NUM_4      // ADXL Movement

#define HAS_LED NOT_A_PIN

// GPS settings
#define HAS_GPS         1                       // use on board GPS
#define GPS_TX          34
#define GPS_RX          12

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
