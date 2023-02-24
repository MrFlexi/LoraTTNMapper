#include "globals.h"
#include "payload.h"

static const char TAG[] = "";

PayloadConvert::PayloadConvert(uint8_t size)
{
  buffer = (uint8_t *)malloc(size);
  cursor = 0;
}

PayloadConvert::~PayloadConvert(void) { free(buffer); }

void PayloadConvert::reset(void) { cursor = 0; }
uint8_t PayloadConvert::getSize(void) { return cursor; }
uint8_t *PayloadConvert::getBuffer(void) { return buffer; }

/* ---------------- plain format without special encoding ---------- */

#if (PAYLOAD_ENCODER == 3)
#define LPP_DIGITAL_INPUT 0 // 1 byte

void PayloadConvert::addCount(uint8_t channel, uint16_t value)
{
  uint16_t volt = value * 100;
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
}

void PayloadConvert::addFloat(uint8_t channel, float value)
{
  uint16_t volt = value * 100;
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
}

void PayloadConvert::addFloatN(uint8_t channel, uint8_t sensor, float value)
{
  uint16_t volt = value * 100;
  buffer[cursor++] = channel;
  buffer[cursor++] = sensor;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
}

void PayloadConvert::addVoltage(uint8_t channel, float value)
{
  uint16_t volt = value * 100;
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
}

void PayloadConvert::addCurrent(uint8_t channel, float value)
{
  uint16_t volt = value * 100;
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
}

void PayloadConvert::addTemperature(uint8_t channel, float value)
{
  uint16_t val = value * 10;
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_TEMPERATURE;
  buffer[cursor++] = highByte(val);
  buffer[cursor++] = lowByte(val);
}

void PayloadConvert::addBMETemp(uint8_t channel)
{
#if (USE_BME280)
  int16_t temperature = (int16_t)(dataBuffer.data.temperature * 100); // float -> int
  uint16_t humidity = (uint16_t)(dataBuffer.data.humidity * 100 );     // float -> int

  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_BME;
  buffer[cursor++] = highByte(temperature);
  buffer[cursor++] = lowByte(temperature);
  buffer[cursor++] = highByte(humidity);
  buffer[cursor++] = lowByte(humidity);

#endif
}

void PayloadConvert::addDeviceData(uint8_t channel){
 
  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_DEVICE;
  buffer[cursor++] = highByte(dataBuffer.data.bootCounter);
  buffer[cursor++] = lowByte(dataBuffer.data.bootCounter);
  buffer[cursor++] = highByte(dataBuffer.settings.sleep_time);
  buffer[cursor++] = lowByte(dataBuffer.settings.sleep_time); 
#endif
}


void PayloadConvert::addPMU(uint8_t channel)
{

  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_PMU;

  uint16_t out = dataBuffer.data.bus_voltage * 100;
  buffer[cursor++] = highByte(out);
  buffer[cursor++] = lowByte(out);

  out = dataBuffer.data.bus_current * 100;
  buffer[cursor++] = highByte(out);
  buffer[cursor++] = lowByte(out);

  out = dataBuffer.data.bat_voltage * 100;
  buffer[cursor++] = highByte(out);
  buffer[cursor++] = lowByte(out);

  out = dataBuffer.data.bat_charge_current * 100;
  buffer[cursor++] = highByte(out);
  buffer[cursor++] = lowByte(out);

  out = dataBuffer.data.bat_DeltamAh;
  buffer[cursor++] = highByte(out);
  buffer[cursor++] = lowByte(out);
}

void PayloadConvert::addGPS_TTN(TinyGPSPlus tGps)
{

#if (USE_GPS)
  uint32_t LatitudeBinary, LongitudeBinary;
  uint16_t altitudeGps;
  uint8_t hdopGps;

  LatitudeBinary = ((tGps.location.lat() + 90) / 180.0) * 16777215;
  LongitudeBinary = ((tGps.location.lng() + 180) / 360.0) * 16777215;

  buffer[cursor++] = (LatitudeBinary >> 16) & 0xFF;
  buffer[cursor++] = (LatitudeBinary >> 8) & 0xFF;
  buffer[cursor++] = LatitudeBinary & 0xFF;

  buffer[cursor++] = (LongitudeBinary >> 16) & 0xFF;
  buffer[cursor++] = (LongitudeBinary >> 8) & 0xFF;
  buffer[cursor++] = LongitudeBinary & 0xFF;

  altitudeGps = tGps.altitude.meters();
  buffer[cursor++] = (altitudeGps >> 8) & 0xFF;
  buffer[cursor++] = altitudeGps & 0xFF;

  hdopGps = tGps.hdop.value() / 10;
  buffer[cursor++] = hdopGps & 0xFF;
#endif
}

void PayloadConvert::addGPS_LPP(uint8_t channel, TinyGPSPlus tGps)
{

#if (USE_GPS)
  uint32_t lat, lon;
  int32_t alt;
  uint8_t hdopGps;

  lat = (uint32_t)(tGps.location.lat() * 1e6 / 100);
  lon = (uint32_t)(tGps.location.lng() * 1e6 / 100);
  alt = tGps.altitude.meters() * 100;

  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_GPS;

  buffer[cursor++] = (byte)((lat & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((lat & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)((lat & 0x0000FF));

  buffer[cursor++] = (byte)((lon & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((lon & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)(lon & 0x0000FF);

  buffer[cursor++] = (byte)((alt & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((alt & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)(alt & 0x0000FF);
#endif
}

void PayloadConvert::enqueue_port(uint8_t port)
{
  #if (HAS_LORA)
  int ret;
  MessageBuffer_t SendBuffer;

  if (dataBuffer.data.aliveCounter > 0)
  {

    SendBuffer.MessageSize = payload.getSize();
    SendBuffer.MessagePrio = 1;
    SendBuffer.MessagePort = port;

    int n = uxQueueMessagesWaiting(LoraSendQueue);
    ESP_LOGI(TAG, "Lora queue size: %d", n);

    // Clear first entry in queue if no space available
    if (uxQueueSpacesAvailable(LoraSendQueue) == 0)
    {
      if (xQueueReceive(LoraSendQueue, &SendBuffer, portMAX_DELAY) == pdTRUE) // delete one element
      {
        ESP_LOGI(TAG, "Queue aging: First element deleted:");
        dump_single_message(SendBuffer);
      }
      //int p = uxQueueMessagesWaiting(LoraSendQueue);
      //ESP_LOGI(TAG, "Queue aging waiting bevore: %d, after: %d", n, p);
    }

    // Insert new element at end of queue
    ESP_LOGI(TAG, "Enqueue new message, size: %d port: %d", SendBuffer.MessageSize, SendBuffer.MessagePort);
    memcpy(SendBuffer.Message, payload.getBuffer(), SendBuffer.MessageSize);
    ret = xQueueSendToBack(LoraSendQueue, &SendBuffer, 0);
    if (ret != 1)
    {
      ESP_LOGI(TAG, "LORA sendqueue is full");
    }
  }
#endif
}


void lora_queue_init(void)
{
#if (HAS_LORA)
  assert(SEND_QUEUE_SIZE);
  LoraSendQueue = xQueueCreate(SEND_QUEUE_SIZE, sizeof(MessageBuffer_t));
  if (LoraSendQueue == 0)
  {
    ESP_LOGE(TAG, "Could not create LORA send queue. Aborting.");
  }
  ESP_LOGI(TAG, "LORA send queue created, size %d Bytes",
           SEND_QUEUE_SIZE * sizeof(MessageBuffer_t));
#endif
}
