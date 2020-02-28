#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "globals.h"

#define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board
#define DEFAULT_VREF 1100 
#define NO_OF_SAMPLES 32  // we do some multisampling to get better values

esp_adc_cal_characteristics_t *adc_characs =
    (esp_adc_cal_characteristics_t *)calloc(
        1, sizeof(esp_adc_cal_characteristics_t));


static const adc1_channel_t adc_channel = ADC1_GPIO35_CHANNEL;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;



void Poti_calibrate_voltage(void)
{

// configure ADC
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channel, atten));

  // calibrate ADC
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_characs);
  // show ADC characterization base
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
      Serial.println( "ADC characterization based on Two Point values stored in eFuse");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
   Serial.println( "ADC characterization based on reference voltage stored in eFuse");
  }
  else
  {
    Serial.println(  "ADC characterization based on default reference voltage");
  }

}


float Poti_read_voltage(adc1_channel_t channel)
{
  uint16_t voltage = 0;
  uint32_t adc_reading = 0;

  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    adc_reading += adc1_get_raw(channel);
  }

  adc_reading /= NO_OF_SAMPLES;
  // Convert ADC reading to voltage in mV
  voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_characs);
  //Serial.print(voltage);Serial.print("  ");Serial.println(adc_reading);

  voltage = map(voltage, 0, 3300, 0, 4096 );
  return voltage / (float) 1000;
}

uint16_t Poti_read_ticks(adc1_channel_t channel)
{
  uint16_t voltage = 0;
  uint16_t ticks = 0;
  uint32_t adc_reading = 0;

  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    adc_reading += adc1_get_raw(channel);
  }

  adc_reading /= NO_OF_SAMPLES;
  // Convert ADC reading to voltage in mV
  voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_characs);
  

  ticks = map(voltage, 0, 3300, 4096, 0 );
  //Serial.print(voltage);Serial.print("  ");Serial.print(adc_reading);Serial.print("  ");Serial.println(ticks);
  return ticks;
}



void t_getADCValues( void * parameter ){
    DataBuffer *locdataBuffer;
    
    locdataBuffer = (DataBuffer *) parameter; 
 
    //Continuously sample ADC1
    while (1) {
    //Serial.print("globalClassTask: ");
    //Serial.println( locdataBuffer->data.potentiometer_a );
    locdataBuffer->data.potentiometer_a = Poti_read_ticks(adc_channel);
    //Serial.print(xPortGetCoreID());
    vTaskDelay(200);
    }
 
} 
 
void poti_setup_RTOS() {  

  Poti_calibrate_voltage();  

  xTaskCreatePinnedToCore(
                    t_getADCValues,             /* Task function. */
                    "globalClassTask",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&dataBuffer,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL,
                    0);                     /* Task handle. */
 
 
}

 
