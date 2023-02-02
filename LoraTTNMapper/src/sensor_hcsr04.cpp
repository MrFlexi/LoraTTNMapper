#include <Arduino.h>
#include "globals.h"

static const char TAG[] = __FILE__;

#if (USE_DISTANCE_SENSOR_HCSR04)
HCSR04 ultrasonicSensor(HCSR04_trigger_pin,HCSR04_echo_pin, 20, 300);

void t_getHcsr04Values(void *parameter)
{
  DataBuffer *locdataBuffer;
  uint16_t distance;
  uint16_t distance_old;
  locdataBuffer = (DataBuffer *)parameter;

  //Continuously sample 
  while (1)
  {
    distance = (uint16_t)ultrasonicSensor.getMedianFilterDistance();
    if (distance != distance_old)
    {
      distance_old = distance;
      locdataBuffer->data.hcsr04_distance = distance;
      locdataBuffer->data.distance_changed = true;
      ESP_LOGI(TAG, "HC-SR04 Distance: %d", locdataBuffer->data.hcsr04_distance );
    }
    vTaskDelay(2000);
  }
}

void setup_hcsr04_rtos()
{
  ultrasonicSensor.begin(); //set trigger as output & echo pin as input
  ultrasonicSensor.setTemperature(18.5);
  
  xTaskCreatePinnedToCore(
      t_getHcsr04Values,      /* Task function. */
      "globalClassTask",   /* String with name of task. */
      10000,               /* Stack size in words. */
      (void *)&dataBuffer, /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,
      0); /* Task handle. */
}
#endif