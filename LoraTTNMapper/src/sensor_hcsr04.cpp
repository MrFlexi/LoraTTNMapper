#include <Arduino.h>
#include "globals.h"

HCSR04 ultrasonicSensor(HCSR04_trigger_pin,HCSR04_echo_pin, 20, 300);

void t_getHcsr04Values(void *parameter)
{
  DataBuffer *locdataBuffer;
  float distance;
  float distance_old;
  locdataBuffer = (DataBuffer *)parameter;

  //Continuously sample ADC1
  while (1)
  {
    distance = ultrasonicSensor.getMedianFilterDistance();
    if (distance != distance_old)
    {
      distance_old = distance;
      locdataBuffer->data.distance = distance;
      locdataBuffer->data.distance_changed = true;
      Serial.print("HC-SR04 Distance:");Serial.println( locdataBuffer->data.distance );
    }
    vTaskDelay(500);
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
