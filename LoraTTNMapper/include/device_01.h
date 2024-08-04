
//--------------------------------------------------------------------------
// Device ID
//--------------------------------------------------------------------------
// 01    --> T-Beam   W-Lan and BLE defect
//----------------------------------------------------------------
// GPIO 02   Output Distance Sensor Trigger
// GPIO 13   Input  Distance Sensor Echo
// GPIO 35   APX    PMU Interrupt Pin
// GPIO 36   ADC1   Soil Moisture       PIN VP
//----------------------------------------------------------------

#pragma once

#define DEVICE_NAME "mrflexi-01"
#define USE_WIFI_AP 1
#define USE_OTA 0
#define USE_BME280 0
#define HAS_LORA 1
#define HAS_INA3221  0
#define HAS_INA219  0
#define USE_GPS 1
#define USE_DISPLAY 1

#define USE_WIFISCANNER 0
#define USE_WIFICOUNTER 1

#define HAS_PMU 1
#define PMU_SLEEP_ALL_OFF 0    // Error I2C Interface on reboot  On TBEAM must be 0
#define USE_INTERRUPTS 1
#define USE_PMU_INTERRUPT 1
#define USE_BUTTON 1

#define USE_SPIFF_LOGGING 0

#define USE_WIFI 1
#define USE_WIFI_AP 0
#define USE_WIFI_MANAGER 0
#define USE_WEBSERVER   1
#define USE_MQTT 1
#define USE_MQTT_SENSORS 1
#define USE_MQTT_TRAIN 0

#define USE_FASTLED 0
#define USE_FASTLED_RTOS 0
#define FASTLED_SHOW_DEGREE 0
#define FASTLED_SHOW_POTI 0

#define USE_BLE_SERVER 0
#define USE_POTI 0
#define USE_SOIL_MOISTURE 0

#define USE_DISTANCE_SENSOR_HCSR04 0      // Ultrasonic distance sensor
#define HCSR04_trigger_pin  GPIO_NUM_2
#define HCSR04_echo_pin     GPIO_NUM_13

#define USE_PWM_SERVO 0                         // Uses external I2C Servo expander
#define USE_SUN_POSITION 0

#define displayRefreshIntervall 4               // get sensor values and update display     ---> t_cyclic
#define displayMoveIntervall 8                  // shift to next display page               ---> t_moveDisplay

#define LORAenqueueMessagesIntervall 30         // Queue Lora messages
#define LORA_TX_INTERVAL  15                    // Transmitt Lora messages
#define LORA_DATARATE DR_SF10

#define sendMqttIntervall      60               // Mqtt send intervall in s                ---> t_send_cycle
#define sendWebsocketIntervall 15000              // WebSocket send intervall in ms 
#define sunTrackerRefreshIntervall 60

#define PAYLOAD_ENCODER 3
#define PAYLOAD_BUFFER_SIZE 60
#define SEND_QUEUE_SIZE 10
#define PAD_TRESHOLD 40

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define HAS_DISPLAY U8G2_SSD1306_128X64_NONAME_F_HW_I2C

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------
#define ESP_SLEEP 1                             // Main switch

#define TIME_TO_SLEEP 10                        // sleep for n minute
#define TIME_TO_NEXT_SLEEP_WITHOUT_MOTION  10    // sleep after n minutes without movement or
#define SLEEP_AFTER_N_TX_COUNT 10               // after n Lora TX events

#define USE_GPS_MOTION 1
#define GPS_MOTION_DISTANCE 50                  // Reset sleep if moved for x Meter

#define AUTO_POWER_SAVE 1                       // If battery voltage < 3.7  --> sleep for 54 Minutes
#define TIME_TO_SLEEP_BAT_HIGH  6
#define TIME_TO_SLEEP_BAT_MID 18
#define TIME_TO_SLEEP_BAT_LOW  54
#define BAT_LOW   37                            // 3.7 Volt
#define BAT_HIGH  40 


