#include "globals.h"
#include "payload.h"



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

  void PayloadConvert::addVoltage(uint8_t channel, float value)
  {
    uint16_t volt = value*100;
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ANALOG_INPUT;
    buffer[cursor++] = highByte(volt);
    buffer[cursor++] = lowByte(volt);
  }


  void PayloadConvert::addCurrent(uint8_t channel, float value)
  {
    uint16_t volt = value*100;
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ANALOG_INPUT;
    buffer[cursor++] = highByte(volt);
    buffer[cursor++] = lowByte(volt);
  }

void PayloadConvert::addTemperature(uint8_t channel, float value)
  {
    uint16_t val = value * 100;
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_TEMPERATURE;
    buffer[cursor++] = highByte(val);
    buffer[cursor++] = lowByte(val);
  }



void PayloadConvert::addBMETemp(uint8_t channel,  DataBuffer dataBuffer) {
#if (USE_BME280)
  int16_t temperature = (int16_t)(dataBuffer.data.temperature * 100 ); // float -> int
  uint16_t humidity = (uint16_t)(dataBuffer.data.humidity * 100);     // float -> int

  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_TEMPERATURE;
  buffer[cursor++] = highByte(temperature);
  buffer[cursor++] = lowByte(temperature);

  //buffer[cursor++] = highByte(humidity);
  //buffer[cursor++] = lowByte(humidity);

#endif
}

void PayloadConvert::addBatVoltage(uint8_t channel,  DataBuffer dataBuffer) {

  int16_t voltage = (int16_t)(dataBuffer.data.bat_voltage ); // float -> int

  buffer[cursor++] = channel;
  buffer[cursor++] = LPP_TEMPERATURE;
  buffer[cursor++] = highByte(voltage);
  buffer[cursor++] = lowByte(voltage);

}


void PayloadConvert::addGPS_Lora(uint8_t channel, TinyGPSPlus tGps)
{

#if (USE_GPS)
  uint32_t LatitudeBinary, LongitudeBinary;
  uint16_t altitudeGps;
  uint8_t hdopGps;

  LatitudeBinary = ((tGps.location.lat() + 90) / 180.0) * 16777215;
  LongitudeBinary = ((tGps.location.lng() + 180) / 360.0) * 16777215;

  buffer[cursor++] = LPP_GPS_CHANNEL;
  buffer[cursor++] = LPP_GPS;

  buffer[cursor++] = (LatitudeBinary >> 16) & 0xFF;
  buffer[cursor++] = (LatitudeBinary >> 8) & 0xFF;
  buffer[cursor++] = LatitudeBinary & 0xFF;

  buffer[cursor++] = (LongitudeBinary >> 16) & 0xFF;
  buffer[cursor++] = (LongitudeBinary >> 8) & 0xFF;
  buffer[cursor++]= LongitudeBinary & 0xFF;

  altitudeGps = tGps.altitude.meters();
  buffer[cursor++]= (altitudeGps >> 8) & 0xFF;
  buffer[cursor++] = altitudeGps & 0xFF;

  hdopGps = tGps.hdop.value() / 10;
  buffer[cursor++]= hdopGps & 0xFF;
 #endif 
}


void PayloadConvert::addGPS_LPP(TinyGPSPlus tGps)
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
  buffer[cursor++]= LongitudeBinary & 0xFF;

  altitudeGps = tGps.altitude.meters();
  buffer[cursor++]= (altitudeGps >> 8) & 0xFF;
  buffer[cursor++] = altitudeGps & 0xFF;

  hdopGps = tGps.hdop.value() / 10;
  buffer[cursor++]= hdopGps & 0xFF;
 #endif 
}

  void PayloadConvert::enqueue_port(uint8_t port)
  {
    int ret;
    MessageBuffer_t
        SendBuffer; // contains MessageSize, MessagePort, MessagePrio, Message[]

    SendBuffer.MessageSize = payload.getSize();
    ESP_LOGI(TAG, "Payload size %d", SendBuffer.MessageSize);
    SendBuffer.MessagePrio = 1;
    SendBuffer.MessagePort = port;

    memcpy(SendBuffer.Message, payload.getBuffer(), SendBuffer.MessageSize);
    ESP_LOGI(TAG, "SendBuffer[0..8]: %d %d %d %d %d %d %d %d ", SendBuffer.Message[0], SendBuffer.Message[1], SendBuffer.Message[2], SendBuffer.Message[3], SendBuffer.Message[4], SendBuffer.Message[5], SendBuffer.Message[6], SendBuffer.Message[7]);
    ret = xQueueSendToBack(LoraSendQueue, &SendBuffer, 0);

    if (ret != 1)
    {
      ESP_LOGI(TAG, "LORA sendqueue is full");
    }
  }

#endif


  void lora_queue_init(void)
  {
    assert(SEND_QUEUE_SIZE);
    LoraSendQueue = xQueueCreate(SEND_QUEUE_SIZE, sizeof(MessageBuffer_t));
    if (LoraSendQueue == 0)
    {
      ESP_LOGE(TAG, "Could not create LORA send queue. Aborting.");
    }
    ESP_LOGI(TAG, "LORA send queue created, size %d Bytes",
             SEND_QUEUE_SIZE * sizeof(MessageBuffer_t));
  }


