
#pragma once

#define DEVICE_NAME "camera-01"
#define USE_OTA 0
#define USE_BME280 0
#define HAS_LORA 0
#define HAS_INA3221  0
#define HAS_INA219  0
#define USE_CAMERA 1

#define USE_GPS 0

#define USE_DISPLAY 0
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#define HAS_PMU 0
#define USE_INTERRUPTS 1
#define USE_PMU_INTERRUPT 1
#define USE_BUTTON 0

#define USE_SPIFF_LOGGING 1

#define USE_WIFI 1
#define USE_WIFI_MANAGER 0
#define USE_WEBSERVER   0 
#define USE_CAYENNE 0
#define USE_MQTT 1
#define USE_MQTT_SENSORS 1
#define USE_MQTT_TRAIN 0

#define USE_FASTLED 0
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_POTI 0
#define USE_SOIL_MOISTURE 0


#if (HAS_I2C_MICROPHONE)
#define USE_I2C_MICROPHONE 0
#endif

#define LORAenqueueMessagesIntervall 120 // Queue Lora messages
#define LORA_TX_INTERVAL  120            // Transmitt Lora messages
#define LORA_DATARATE DR_SF12
#define displayRefreshIntervall 5         // get sensor values and update display     ---> t_cyclic
#define displayMoveIntervall 120          // shift to next display page               ---> t_moveDisplay

#define sendMqttIntervall      15 // Cayenne mqtt send intervall                   ---> t_send_cycle
#define sendWebsocketIntervall  120 // Update Webpage

#define PAYLOAD_BUFFER_SIZE 0

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1            // Main switch


#define TIME_TO_SLEEP 10       // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  4 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 4 // after n Lora TX events

#define AUTO_POWER_SAVE 0    // If battery voltage < 3.7  --> sleep for 54 Minutes
#define TIME_TO_SLEEP_BAT_HIGH  6
#define TIME_TO_SLEEP_BAT_MID 18
#define TIME_TO_SLEEP_BAT_LOW  54
#define BAT_LOW   37         // 3.7 Volt
#define BAT_HIGH  40 


