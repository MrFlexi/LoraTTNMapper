
#pragma once

#define DEVICE_NAME "tbeam-01"
#define USE_OTA 0
#define USE_BME280 1

#define HAS_LORA 1
#define USE_MQTT 0
#define HAS_INA 1
#define USE_DASH 0
#define USE_GPS 1
#define USE_DISPLAY 1

#define HAS_PMU 1
#define USE_INTERRUPTS 1
#define USE_PMU_INTERRUPT 0
#define USE_BUTTON 0

#define USE_BLE 0
#define USE_SERIAL_BT 0

#define USE_WIFI 1
#define USE_WEBSERVER   0 
#define USE_CAYENNE 0
#define USE_MQTT 1

#define USE_GYRO  1
#define WAKEUP_BY_MOTION 0

#define USE_FASTLED 1
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_POTI 0

#define displayRefreshIntervall 2       // every x second
#define displayMoveIntervall 7          // every x second

#define LORAenqueueMessagesIntervall 90 // every x seconds
#define LORA_TX_INTERVAL 30

#define sendCycleIntervall     10 // every x seconds
#define sendWebsocketIntervall  5 // every x seconds

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 51
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 0             // Main switch
#define uS_TO_S_FACTOR 1000000   //* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 25        // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  5 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 3 // after n Lora TX events
