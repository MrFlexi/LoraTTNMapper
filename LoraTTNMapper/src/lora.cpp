#include "globals.h"
#include "lora.h"

// Local logging tag
static const char TAG[] = __FILE__;


void t_enqueue_LORA_messages()
{
  String stringOne;

  if (LoraSendQueue == 0)
  {
    ESP_LOGE(TAG, "LORA send queue not initalized. Aborting.");
  }
  else
  {

#if (USE_GPS)
    if (gps.checkGpsFix())
    {
      payload.reset();
      payload.addGPS(gps.tGps); // TTN-Mapper format will be generated in TTN Payload converter
      payload.enqueue_port(1);
    }
    else
    {
      ESP_LOGV(TAG, "GPS no fix");
    }
#endif

#if (USE_BME280)
    payload.reset();
    payload.addBMETemp(2, dataBuffer); // Cayenne format will be generated in TTN Payload converter
    payload.enqueue_port(2);
#endif

    payload.reset();
    payload.addTemperature(1, 5.11);
    payload.enqueue_port(2);

#if (HAS_PMU)
    payload.reset();
    payload.addVoltage(20, dataBuffer.data.bus_voltage);
    payload.enqueue_port(2);

    payload.reset();
    payload.addVoltage(30, dataBuffer.data.bat_voltage);
    payload.enqueue_port(2);

    payload.reset();
    payload.addVoltage(31, dataBuffer.data.bat_charge_current);
    payload.enqueue_port(2);

    payload.reset();
    payload.addVoltage(32, dataBuffer.data.bat_discharge_current);
    payload.enqueue_port(2);
#endif   
    ESP_LOGI(TAG, "Lora Message Queue: %d", uxQueueMessagesWaiting(LoraSendQueue));
  }
}


void do_send(osjob_t *j)
{

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {
    if (gps.checkGpsFix())
    {
      // Prepare upstream data transmission at the next possible time.
      gps.buildPacket(txBuffer);
      LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), 0);
      Serial.println(F("Packet queued"));
      digitalWrite(BUILTIN_LED, HIGH);
    }
    else
      ESP_LOGV(TAG, "GPS no fix");
  }
}

void t_LORA_send_from_queue(osjob_t *j)
{
  MessageBuffer_t SendBuffer;
  ESP_LOGI(TAG, "Send Lora MSG from Queue");

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {

    if (LoraSendQueue == 0)
    {
      ESP_LOGE(TAG, "LORA send queue not initalized. Aborting.");
    }
    else
    {
      if (xQueueReceive(LoraSendQueue, &SendBuffer, portMAX_DELAY) != pdTRUE)
      {
        ESP_LOGE(TAG, "Queue is empty...");
      }
      else
      {
        ESP_LOGI(TAG, "LORA package queued: Port %d, Size %d", SendBuffer.MessagePort, SendBuffer.MessageSize);
        ESP_LOGI(TAG, "SendBuffer[0..8]: %d %d %d %d %d %d %d %d ", SendBuffer.Message[0], SendBuffer.Message[1], SendBuffer.Message[2], SendBuffer.Message[3], SendBuffer.Message[4], SendBuffer.Message[5], SendBuffer.Message[6], SendBuffer.Message[7]);
        LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
        ESP_LOGI(TAG, "done...");
      }
    }
    ESP_LOGE(TAG, "New callback scheduled...");
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), t_LORA_send_from_queue);
  }
}