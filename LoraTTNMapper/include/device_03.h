
#pragma once

#define DEVICE_NAME "esp-01"
#define USE_OTA 0
#define USE_BME280 0

#define HAS_INA  1
#define USE_DASH 0
#define USE_GPS 0

#define HAS_PMU 0

#define USE_DISPLAY 1
#define CYCLIC_SHOW_LOG 1

#define USE_INTERRUPTS 0
#define USE_PMU_INTERRUPT 0
#define USE_BUTTON 0

#define USE_WIFI 1
#define USE_MQTT 1
#define USE_WEBSERVER   1 
#define USE_CAYENNE 0
#define HAS_LORA 0


#define USE_FASTLED 1
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_POTI 0

#define displayRefreshIntervall 2       // every x second
#define displayMoveIntervall 7          // every x second

#define LORAenqueueMessagesIntervall 60 // every x seconds
#define LORA_TX_INTERVAL 30

#define sendCycleIntervall      60 // every x seconds
#define sendWebsocketIntervall  10 // every x seconds

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 51
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define USE_GPS_MOTION 0
#define GPS_MOTION_DISTANCE 20  // Meter

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 0              // Main switch
#define TIME_TO_SLEEP  24       // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  5 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 10 // after n Lora TX events
