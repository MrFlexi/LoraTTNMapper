#include <Arduino.h>

#if (USE_POTI)
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "globals.h"

static const char TAG[] = __FILE__;

#define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board
#define DEFAULT_VREF 1100
#define NO_OF_SAMPLES 100 // we do some multisampling to get better values

esp_adc_cal_characteristics_t *adc_characs =
    (esp_adc_cal_characteristics_t *)calloc(
        1, sizeof(esp_adc_cal_characteristics_t));

static const adc1_channel_t adc_channel = ADC1_CHANNEL_0;   // Tbeam GPIO36  PIN VP
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
    Serial.println("ADC characterization based on Two Point values stored in eFuse");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    Serial.println("ADC characterization based on reference voltage stored in eFuse");
  }
  else
  {
    Serial.println("ADC characterization based on default reference voltage");
  }
}

uint16_t ADC_read_ticks(adc1_channel_t channel)
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

  uint16_t ticks = map(voltage, 142, 3145, 0, 1000);
  ESP_LOGI(TAG, "Ticks  %d", ticks );
  return ticks;
}

uint16_t ADC_read_voltage(adc1_channel_t channel)
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
  //ESP_LOGI(TAG, "ADC Voltage %d mV", voltage );
  return voltage;
}

void globalIntTask(void *parameter)
{
  Serial.print("globalIntTask: ");
  Serial.println(*((int *)parameter));
  vTaskDelete(NULL);
}

void t_getADCValues(void *parameter)
{
  DataBuffer *locdataBuffer;
  uint16_t voltage_old;
  uint16_t voltage;
  locdataBuffer = (DataBuffer *)parameter;

  //Continuously sample ADC1
  while (1)
  {
    voltage = ADC_read_voltage(adc_channel);
    if (voltage != voltage_old)
    {
      voltage_old = voltage;
      locdataBuffer->data.potentiometer_a = voltage;
      locdataBuffer->data.potentiometer_a_changed = true;
      //Serial.print("ADC Ticks:");Serial.println( locdataBuffer->data.potentiometer_a );
    }
    vTaskDelay(300);
  }
}

void poti_setup_RTOS()
{
  Poti_calibrate_voltage();
  xTaskCreatePinnedToCore(
      t_getADCValues,      /* Task function. */
      "globalClassTask",   /* String with name of task. */
      10000,               /* Stack size in words. */
      (void *)&dataBuffer, /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,
      0); /* Task handle. */
}
#endif
