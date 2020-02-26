#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board
#define DEFAULT_VREF 1100 
#define NO_OF_SAMPLES 64  // we do some multisampling to get better values

esp_adc_cal_characteristics_t *adc_characs =
    (esp_adc_cal_characteristics_t *)calloc(
        1, sizeof(esp_adc_cal_characteristics_t));


static const adc1_channel_t adc_channel = ADC1_GPIO35_CHANNEL;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;


typedef struct
{
  uint16_t potentiometer_a;   //
} deviceStatus_t;

class DataBuffer
{
  public:
    DataBuffer();
    void set( deviceStatus_t input );
    void get();  
    deviceStatus_t data ;
};

DataBuffer::DataBuffer()
{
}

void DataBuffer::set(deviceStatus_t input)
{
  data = input;
}

void DataBuffer::get()
{
}

int globalIntVar = 5;
DataBuffer dataBuffer;



void calibrate_voltage(void)
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


uint16_t read_voltage(adc1_channel_t channel)
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

  return voltage;
}


void globalIntTask( void * parameter ){
 
    Serial.print("globalIntTask: ");
    Serial.println(*((int*)parameter));            
 
    vTaskDelete( NULL );
 
}

void globalClassTask( void * parameter ){
    DataBuffer *locdataBuffer;
    
    locdataBuffer = (DataBuffer *) parameter; 
 
    //Continuously sample ADC1
    while (1) {
    //Serial.print("globalClassTask: ");
    //Serial.println( locdataBuffer->data.potentiometer_a );
    locdataBuffer->data.potentiometer_a = read_voltage(adc_channel);
    //Serial.print(xPortGetCoreID());
 
    vTaskDelay(200);
    }
 
}
 
 
void setup() {

  Serial.begin(112500);
  delay(1000);

  calibrate_voltage();

  dataBuffer.data.potentiometer_a = 99;
 
 xTaskCreatePinnedToCore(
                    globalIntTask,             /* Task function. */
                    "globalIntTask",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&globalIntVar,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL,
                    0);                     /* Task handle. */

  xTaskCreatePinnedToCore(
                    globalClassTask,             /* Task function. */
                    "globalClassTask",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&dataBuffer,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL,
                    0);                     /* Task handle. */
 
 
}
 
void loop() {
  delay(1000);
  Serial.println(dataBuffer.data.potentiometer_a );

}
 
