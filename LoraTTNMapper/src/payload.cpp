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

void PayloadConvert::addByte(uint8_t value)
{
  /* 
  not implemented
  */ }

  void PayloadConvert::addVoltage(uint16_t value)
  {
    uint16_t volt = value / 10;
    buffer[cursor++] = LPP_BATT_CHANNEL;
    buffer[cursor++] = LPP_ANALOG_INPUT;
    buffer[cursor++] = highByte(volt);
    buffer[cursor++] = lowByte(volt);
  }

#endif

  void PayloadConvert::enqueue(uint8_t port)
  {
    int ret;
    MessageBuffer_t
        SendBuffer; // contains MessageSize, MessagePort, MessagePrio, Message[]

    SendBuffer.MessageSize = payload.getSize();
    ESP_LOGI(TAG, "Payload size %d", SendBuffer.MessageSize);
    SendBuffer.MessagePrio = 1;
    SendBuffer.MessagePort = port;

    memcpy(SendBuffer.Message, payload.getBuffer(), SendBuffer.MessageSize);
    ESP_LOGI(TAG, "SendBuffer[0,1,2,3,4]: %d %d %d %d ", SendBuffer.Message[0], SendBuffer.Message[1], SendBuffer.Message[2], SendBuffer.Message[3]);
    ret = xQueueSendToBack(LoraSendQueue, &SendBuffer, 0);

    if (ret != 1)
    {
      ESP_LOGI(TAG, "LORA sendqueue is full");
    }
  }

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
