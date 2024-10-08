#include "globals.h"
#include "lora.h"

// Local logging tag
static const char TAG[] = "";
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
#if (HAS_LORA)
  String stringOne;

  if (LoraSendQueue == 0)
  {
    ESP_LOGE(TAG, "LORA send queue not initalized. Aborting.");
    return;
  }

  // -----------------------------------------------------------------------------
  //   Port 1: TTN Mapper
  // -----------------------------------------------------------------------------
#if (USE_GPS)
  if (gps.checkGpsFix())
  {
    if (gps.tGps.location.lat() > 0)
    {
      payload.reset();
      payload.addGPS_TTN(gps.tGps); // TTN-Mapper format will be re-generated in TTN Payload converter
      payload.enqueue_port(1);
    }
  }
  else
   ESP_LOGE(TAG, "LORA GPS NO FIX.");
#endif

  // -----------------------------------------------------------------------------
  //   Port 2: Cayenne My Devices
  // -----------------------------------------------------------------------------

  // payload.reset();
  // payload.addCount(LPP_BOOTCOUNT_CHANNEL, dataBuffer.data.bootCounter);
  // payload.addFloat(LPP_FIRMWARE_CHANNEL, dataBuffer.data.firmware_version);

  // #if (USE_GPS)
  //     if (gps.checkGpsFix())
  //     {
  //       //payload.addGPS_LPP(5, gps.tGps); // Format for Cayenne LPP Message
  //     }
  // #endif

#if (HAS_INA3221 || HAS_INA219)
  payload.addVoltage(10, dataBuffer.data.panel_voltage);
  payload.addVoltage(12, dataBuffer.data.panel_current);
#endif

  // payload.enqueue_port(2); // send data

  // -----------------------------------------------------------------------------
  //   Port 3: Device --> TTN, no payload concerter --> NodeRed --> Influxdb
  //   Payload will be converted to Json-InfluxDB format in NodeRed
  // -----------------------------------------------------------------------------
  payload.reset();

#if (HAS_PMU)
  payload.addPMU(01); //(channel, 10 bytes)
#endif

#if (USE_WIFICOUNTER)
  payload.addWifiCount(01); //(channel, 4 bytes)
#endif


#if (USE_SOIL_MOISTURE)
  payload.addFloatN(0x01, LPP_SOIL_MOISTURE, dataBuffer.data.soil_moisture);
#endif


#if (USE_DISTANCE_SENSOR_HCSR04)
  // HC-SR04 Sonic distance sensor
  payload.addFloatN(0x01, LPP_HCSR04_DISTANCE, dataBuffer.data.hcsr04_distance);
  // measurement["HCSR04_Distance"] = data.hcsr04_distance;
#endif

#if (USE_BME280)
  payload.addBMETemp(01); // (channel, 4 bytes)
#endif

  payload.addDeviceData(0); // (channel, 4 bytes)
  payload.enqueue_port(2);  // send data

#endif
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
      ESP_LOGI(TAG, "Send Lora ");
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

    if (LoraSendQueue != 0)
    {

      int n = uxQueueMessagesWaiting(LoraSendQueue);
      ESP_LOGI(TAG, "Messages waiting: %d", n);

      if (xQueueReceive(LoraSendQueue, &SendBuffer, portMAX_DELAY) == pdTRUE)
      {
        ESP_LOGI(TAG, "Lora trying to send from queue:");
        dump_single_message(SendBuffer);
        LMIC_setTxData2(SendBuffer.MessagePort, SendBuffer.Message, SendBuffer.MessageSize, 0);
      }
      else
      {
        ESP_LOGV(TAG, "Queue is empty...");
      }
    }
    else
    {
      ESP_LOGV(TAG, "LORA send queue not initalized. Aborting.");
    }
    ESP_LOGI(TAG, "New callback scheduled...");
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(LORA_TX_INTERVAL), t_LORA_send_from_queue);
  }
}

void dump_queue()
{
  MessageBuffer_t SendBuffer;
  ESP_LOGI(TAG, "--- Queue Dump ---");

  if (LoraSendQueue != 0)
  {
    int n = uxQueueMessagesWaiting(LoraSendQueue);
    ESP_LOGI(TAG, "Messages waiting: %d", n);

    for (int i = 0; i < n; i++)
    {
      if (xQueueReceive(LoraSendQueue, &SendBuffer, portMAX_DELAY) == pdTRUE)
      {
        dump_single_message(SendBuffer);
      }
    }
    Serial.println();
  }
}

void dump_single_message(MessageBuffer_t SendBuffer)
{
  char hex_string[5];
  ESP_LOGI(TAG, "Lora TX Message Port: %d  Size: %d", SendBuffer.MessagePort, SendBuffer.MessageSize);

  Serial.println("Payload:");
  for (int p = 0; p < SendBuffer.MessageSize; p++)
  {
    sprintf(hex_string, "0x%02X", SendBuffer.Message[p]);
    Serial.print(hex_string);
    Serial.print(" ");
  }
  Serial.println();
}

void setup_lora()
{
#if (HAS_LORA)
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
  LMIC_setDrTxpow(LORA_DATARATE, 14);

  t_LORA_send_from_queue(&sendjob);

  ESP_LOGI(TAG, "IBM LMIC version %d.%d.%d", LMIC_VERSION_MAJOR,
           LMIC_VERSION_MINOR, LMIC_VERSION_BUILD);
  ESP_LOGI(TAG, "Arduino LMIC version %d.%d.%d.%d",
           ARDUINO_LMIC_VERSION_GET_MAJOR(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_MINOR(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_PATCH(ARDUINO_LMIC_VERSION),
           ARDUINO_LMIC_VERSION_GET_LOCAL(ARDUINO_LMIC_VERSION));
#endif
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
    if (LMIC.txrxFlags & TXRX_ACK)
    {
      log_display("Received Ack");
      dataBuffer.data.rxCounter++;
    }
    if (LMIC.dataLen > 0)
    {
      sprintf(s, "Received %i bytes payload", LMIC.dataLen);
      log_display(s);
      dataBuffer.data.lmic = LMIC;
      sprintf(s, "RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
      log_display(s);
      Serial.println("");
      Serial.println("Payload");
      for (int i = 0; i < LMIC.dataLen; i++)
      {
        if (LMIC.frame[LMIC.dataBeg + i] < 0x10)
        {
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
        }
      }

      switch (LMIC.frame[LMIC.dataBeg + 1])
      {
      case TTN_COMMAND_RESET_COULOMB:
        Serial.println(F("TTN Command: Reset Coulomb Counter"));
#if (HAS_PMU)
        pmu.ClearCoulombcounter();
#endif
        break;

      case TTN_COMMAND_SLEEP:
        Serial.println(F("TTN Command: Sleep"));
        ESP32_sleep();
        break;

      default:
        Serial.println(F("TTN Command unknown"));
        break;
      }
    }
    // Schedule next transmission
    ESP_LOGI(TAG, "Next TX started");
    // Next TX is scheduled after TX_COMPLETE event.
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(LORA_TX_INTERVAL), t_LORA_send_from_queue);
    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    log_display("EV_RXCOMPLETE");
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