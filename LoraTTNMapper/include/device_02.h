
#pragma once

#define DEVICE_NAME "mrflexi-02"
#define USE_BME280 1
#define HAS_LORA 1
#define USE_GPS 1
#define USE_DISPLAY 1


#define HAS_INA219  1
#define USE_INTERRUPTS 1

#define HAS_PMU 1
#define USE_PMU_INTERRUPT 1
#define USE_PMU_MPP_TRACKING 1          // Maximum power point tracking for Solar Pannel

#define USE_BUTTON 1

#define USE_BLE 0
#define USE_BLE_SERVER 0
#define USE_BLE_SCANNER 0               // Corona Warn App
#define USE_SERIAL_BT 0
#define USE_SUN_POSITION 1
#define USE_PWM_SERVO 1                 // Uses external I2C Servo expander

#define USE_SOIL_MOISTURE 0 

#define USE_DISTANCE_SENSOR_HCSR04 1       // Ultrasonic distance sensor
#define HCSR04_trigger_pin  GPIO_NUM_2
#define HCSR04_echo_pin     GPIO_NUM_13

#define USE_WIFI 1
#define USE_WEBSERVER   1
#define USE_MQTT 1
#define USE_MQTT_SENSORS 1
#define USE_MQTT_TRAIN 0

#define USE_FASTLED 0
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_SUN_POSITION 1
#define USE_POTI 0

#define displayRefreshIntervall 5      // get sensor values and update display     ---> t_cyclic
#define displayMoveIntervall 10         // shift to next display page               ---> t_moveDisplay

#define LORAenqueueMessagesIntervall 60 // Queue Lora messages
#define LORA_TX_INTERVAL 30           // Transmitt Lora messages
#define LORA_DATARATE DR_SF10

#define sendMqttIntervall      10 // Cayenne mqtt send intervall                   ---> t_send_cycle
#define sendWebsocketIntervall  2 // Update Webpage
#define sunTrackerRefreshIntervall 60

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
#define ESP_SLEEP 0         // Main switch


#define TIME_TO_SLEEP 54                        // sleep for n minute if no uto power mode
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  6    // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 10               // after n Lora TX events

#define AUTO_POWER_SAVE 1     // If battery voltage < 3.7  --> sleep for 54 Minutes
#define TIME_TO_SLEEP_BAT_HIGH  6
#define TIME_TO_SLEEP_BAT_MID 14
#define TIME_TO_SLEEP_BAT_LOW  114
#define BAT_LOW   37         // 3.7 Volt
#define BAT_HIGH  40 


