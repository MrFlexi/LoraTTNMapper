#include "globals.h"
#include "lora.h"

// Local logging tag
static const char TAG[] = __FILE__;
char s[32];

// Pin mapping
extern const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN, // was "14,"
    .dio = {26, 33, 32},
};

QueueHandle_t LoraSendQueue;

void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

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
      payload.addGPS_TTN(gps.tGps); // TTN-Mapper format will be re-generated in TTN Payload converter
      payload.enqueue_port(1);

      payload.reset();
      payload.addGPS_LPP(LPP_GPS_CHANNEL, gps.tGps); // Format for Cayenne LPP Message
      payload.enqueue_port(2);
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

void setup_lora()
{
  log_display("Setup LORA");
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7, 14);

  t_LORA_send_from_queue(&sendjob);
}

void onEvent(ev_t ev)
{
  switch (ev)
  {
  case EV_SCAN_TIMEOUT:
    Serial.println(F("EV_SCAN_TIMEOUT"));
    break;
  case EV_BEACON_FOUND:
    Serial.println(F("EV_BEACON_FOUND"));
    break;
  case EV_BEACON_MISSED:
    Serial.println(F("EV_BEACON_MISSED"));
    break;
  case EV_BEACON_TRACKED:
    Serial.println(F("EV_BEACON_TRACKED"));
    break;
  case EV_JOINING:
    Serial.println(F("EV_JOINING"));
    break;
  case EV_JOINED:
    Serial.println(F("EV_JOINED"));
    // Disable link check validation (automatically enabled
    // during join, but not supported by TTN at this time).
    LMIC_setLinkCheckMode(0);
    break;
  case EV_RFU1:
    Serial.println(F("EV_RFU1"));
    break;
  case EV_JOIN_FAILED:
    Serial.println(F("EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    Serial.println(F("EV_REJOIN_FAILED"));
    break;
  case EV_TXCOMPLETE:
    log_display("EV_TXCOMPLETE");
    dataBuffer.data.txCounter++;
    Serial.println(F("EV_TXCOMPLETE (waiting for RX windows)"));
    digitalWrite(BUILTIN_LED, LOW);
    if (LMIC.txrxFlags & TXRX_ACK)
    {
      Serial.println(F("Received Ack"));
    }
    if (LMIC.dataLen)
    {
      sprintf(s, "Received %i bytes payload", LMIC.dataLen);
      Serial.println(s);
      dataBuffer.data.lmic = LMIC;
      sprintf(s, "RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
      Serial.println(s);
      Serial.println("");
      Serial.println("Payload");
      for (int i = 0; i < LMIC.dataLen; i++)
      {
        if (LMIC.frame[LMIC.dataBeg + i] < 0x10)
        {
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
        }
      }
    }
    // Schedule next transmission
    //esp_sleep_enable_timer_wakeup(TX_INTERVAL*1000000);
    //esp_deep_sleep_start();
    log_display("Next TX started");
    // Next TX is scheduled after TX_COMPLETE event.
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), t_LORA_send_from_queue);
    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    Serial.println(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    Serial.println(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    Serial.println(F("EV_LINK_sleep"));
    break;
  default:
    Serial.println(F("Unknown event"));
    break;
  }
}