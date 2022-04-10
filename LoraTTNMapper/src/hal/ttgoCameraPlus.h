// TTGO Camera  has on board power management by IPS 5306 PMU chip:

#pragma once
#include <stdint.h>

#define FASTLED_DATA_PIN        GPIO_NUM_2

#define POTI_PIN                GPIO_NUM_32
#define BUTTON_PIN              GPIO_NUM_38     // middle on board button
#define GPS_TX                  GPIO_NUM_34
#define GPS_RX                  GPIO_NUM_12

#define HAS_LORA                0               // comment out if device shall not send data via LoRa
#define BOARD_HAS_PSRAM         1               // use extra 4MB external RAM
#define HAS_IP5306              1
#define ENABLE_IP5306           1
#define ENABLE_TFT              1
#define HAS_TFT_DISPLAY         1

#define PWDN_GPIO_NUM       -1
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       4
#define SIOD_GPIO_NUM       18
#define SIOC_GPIO_NUM       23

#define Y9_GPIO_NUM         36
#define Y8_GPIO_NUM         37
#define Y7_GPIO_NUM         38
#define Y6_GPIO_NUM         39
#define Y5_GPIO_NUM         35
#define Y4_GPIO_NUM         26
#define Y3_GPIO_NUM         13
#define Y2_GPIO_NUM         34
#define VSYNC_GPIO_NUM      5
#define HREF_GPIO_NUM       27
#define PCLK_GPIO_NUM       25

#define I2C_SDA             18
#define I2C_SCL             23

#define HAS_I2C_MICROPHONE  1
#define IIS_SCK             14
#define IIS_WS              32
#define IIS_DOUT            33

#define TFT_CS_PIN          12
#define TFT_DC_PIN          15
#define TFT_MOSI_PIN        19
#define TFT_MISO_PIN        22
#define TFT_SCLK_PIN        21
#define TFT_BL_PIN          2

#define SDCARD_CS_PIN       0

