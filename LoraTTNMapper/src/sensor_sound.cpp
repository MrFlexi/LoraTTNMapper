#include <Arduino.h>
#include "globals.h"
#include <driver/i2s.h>

#if (HAS_I2C_MICROPHONE)

// TTF Display   i2s  Port 1 
// Sound Sensor  i2S  Port 2
const i2s_port_t I2S_PORT = I2S_NUM_1;
const int BLOCK_SIZE = 128;

void t_getSoundPressure(void *parameter)
{

  DataBuffer *locdataBuffer;
  float distance;
  float distance_old;
  locdataBuffer = (DataBuffer *)parameter;

  // Read multiple samples at once and calculate the sound pressure
  int32_t samples[BLOCK_SIZE];

  while (1)
  {
    int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                        (char *)samples,
                                        BLOCK_SIZE,     // the doc says bytes, but its elements.
                                        portMAX_DELAY); // no timeout

    int samples_read = num_bytes_read / 8;
    if (samples_read > 0)
    {

      float mean = 0;
      for (int i = 0; i < samples_read; ++i)
      {
        mean += (samples[i] >> 14);
      }
      mean /= samples_read;

      float maxsample = -1e8, minsample = 1e8;
      for (int i = 0; i < samples_read; ++i)
      {
        minsample = min(minsample, samples[i] - mean);
        maxsample = max(maxsample, samples[i] - mean);
      }

      float delta = maxsample - minsample;
      if ( delta > 2035712)
        Serial.println(maxsample - minsample);
    }
    vTaskDelay(100);
  }
}

void t_getSoundValues(void *parameter)
{
  DataBuffer *locdataBuffer;
  float distance;
  float distance_old;
  locdataBuffer = (DataBuffer *)parameter;
  int32_t samples[BLOCK_SIZE];
  
  
  //Continuously read MES microphone
  while (1)
  {
    int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                        (char *)samples,
                                        BLOCK_SIZE,     // the doc says bytes, but its elements.
                                        portMAX_DELAY); // no timeout

    int samples_read = num_bytes_read / 8;
    if (samples_read > 0)
    {
      float mean = 0;
      for (int i = 0; i < samples_read; ++i)
      {
        mean += samples[i];
      }
      Serial.print("Sound level: ");
      Serial.println(mean);
    }

    vTaskDelay(50);
  }
}

void setup_sound_i2s()
{

  esp_err_t err;

  // Start listening for audio: MONO @ 16KHz
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_IRAM ,
      .dma_buf_count = 3,
      .dma_buf_len = 300};

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = IIS_SCK,  // BCKL
      .ws_io_num = IIS_WS,    // LRCL
      .data_out_num = -1,     // not used (only for speakers)
      .data_in_num = IIS_DOUT // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true)
      ;
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK)
  {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true)
      ;
  }
  Serial.println("I2S driver installed.");
}



void setup_sound_rtos()
{

  esp_err_t err;

  // Start listening for audio: MONO @ 16KHz
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_IRAM ,
      .dma_buf_count = 3,
      .dma_buf_len = 300};

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = IIS_SCK,  // BCKL
      .ws_io_num = IIS_WS,    // LRCL
      .data_out_num = -1,     // not used (only for speakers)
      .data_in_num = IIS_DOUT // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true)
      ;
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK)
  {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true)
      ;
  }
  Serial.println("I2S driver installed.");

  xTaskCreatePinnedToCore(
      //t_getSoundValues,    /* Task function. */
      t_getSoundPressure,
      "globalClassTask",   /* String with name of task. */
      10000,               /* Stack size in words. */
      (void *)&dataBuffer, /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,
      0); /* Task handle. */
}

#endif
