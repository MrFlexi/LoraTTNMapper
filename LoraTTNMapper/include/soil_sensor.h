
#pragma once

#define DEVICE_NAME "soil_sensor-01"
#define USE_OTA 0
#define USE_BME280 0
#define HAS_LORA 1
#define HAS_INA3221  0
#define HAS_INA219  0
#define USE_GPS 1
#define USE_DISPLAY 1

#define HAS_PMU 1
#define USE_INTERRUPTS 1
#define USE_PMU_INTERRUPT 1
#define USE_BUTTON 0

#define USE_DISTANCE_SENSOR_HCSR04 1       // Ultrasonic distance sensor
#define HCSR04_trigger_pin  GPIO_NUM_32
#define HCSR04_echo_pin     GPIO_NUM_33

#define USE_SPIFF_LOGGING 1
#define USE_PWM_SERVO 1
#define USE_SUN_POSITION 1

#define USE_BLE_SERVER 1

#define USE_WIFI 1
#define USE_WIFI_MANAGER 0
#define USE_WEBSERVER   1 
#define USE_CAYENNE 0
#define USE_MQTT 1
#define USE_MQTT_SENSORS 1
#define USE_MQTT_TRAIN 0

#define USE_FASTLED 0
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_POTI 1
#define USE_SOIL_MOISTURE 1

#define sunTrackerRefreshIntervall 60
#define displayRefreshIntervall 10     // get sensor values and update display     ---> t_cyclic
#define displayMoveIntervall 7          // shift to next display page               ---> t_moveDisplay

#define LORAenqueueMessagesIntervall 60 // Queue Lora messages
#define LORA_TX_INTERVAL  60            // Transmitt Lora messages
#define LORA_DATARATE DR_SF12

#define sendMqttIntervall      120 // Cayenne mqtt send intervall                   ---> t_send_cycle
#define sendWebsocketIntervall 15 // Update Webpage

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 51
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1         // Main switch


#define TIME_TO_SLEEP 10       // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  4 // // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 4 // after n Lora TX events

#define AUTO_POWER_SAVE 1     // If battery voltage < 3.7  --> sleep for 54 Minutes
#define TIME_TO_SLEEP_BAT_HIGH  5
#define TIME_TO_SLEEP_BAT_MID 18
#define TIME_TO_SLEEP_BAT_LOW  54
#define BAT_LOW   37         // 3.7 Volt
#define BAT_HIGH  40 


