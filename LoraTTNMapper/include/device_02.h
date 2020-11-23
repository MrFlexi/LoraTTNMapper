
#pragma once

#define DEVICE_NAME "tbeam-02"
#define USE_OTA 0
#define USE_BME280 1
#define HAS_LORA 1

#define HAS_INA 0
#define USE_DASH 0
#define USE_GPS 1
#define USE_DISPLAY 1


#define USE_SPIFF_LOGGING 1

#define HAS_PMU 1
#define USE_INTERRUPTS 1
#define USE_PMU_INTERRUPT 1
#define USE_BUTTON 1

#define USE_BLE 0
#define USE_BLE_SCANNER 0               // Corona Warn App
#define USE_SERIAL_BT 0

#define USE_WIFI 1
#define USE_WEBSERVER   1 
#define sendWebsocketIntervall          10 // every x seconds
#define USE_CAYENNE     0
#define USE_MQTT 1

#define USE_FASTLED 0
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_POTI 0

#define displayRefreshIntervall 2       // get sensor values and update display     ---> t_cyclic
#define displayMoveIntervall 7          // shift to next display page               ---> t_moveDisplay

#define LORAenqueueMessagesIntervall 60 // Queue Lora messages
#define LORA_TX_INTERVAL 30             // Transmitt Lora messages
#define LORA_DATARATE DR_SF7

#define sendCycleIntervall      30 // Cayenne mqtt send intervall                   ---> t_send_cycle
#define sendWebsocketIntervall  60 // Update Webpage



#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 60
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define USE_GPS_MOTION 1
#define GPS_MOTION_DISTANCE 20  // Meter

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1              // Main switch
#define TIME_TO_SLEEP 10        // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  5 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 10 // after n Lora TX events
