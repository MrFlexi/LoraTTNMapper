#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "globals.h"

#define WIFI_CHANNEL_SWITCH_INTERVAL 500
#define WIFI_CHANNEL_MAX 13
#define WIFI_CHANNEL_ALL 0b1111111111111

#define MUTEXREFRES_MS 40
static SemaphoreHandle_t mutex;

uint8_t level = 0, channel_rotation = 1;
uint8_t new_mac = 0;
TimerHandle_t WifiChanTimer;

static const char TAG[] = __FILE__;
static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_init(void);
static void wifi_sniffer_set_channel(uint8_t channel_rotation);
static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

uint16_t channels_map = WIFI_CHANNEL_ALL;
static wifi_country_t country;

static wifi_country_t wifi_country = {.cc = "DE", .schan = 1, .nchan = 13}; // Most recent esp32 library struct

// Struktur für die MAC-Daten
struct MacData
{
  char mac_adr[18];
  char date[11];
  char first_seen[9];
  char last_seen[9];
};

// Maximale Anzahl von MAC-Datensätzen
const int maxMacListSize = 512;
MacData macListArray[maxMacListSize];
u_int8_t macListSize = 0;

typedef struct
{
  unsigned frame_ctrl : 16;
  unsigned duration_id : 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl : 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

void printMacList()
{
  // Ausgabe der MAC-Daten
  for (int i = 0; i < macListSize; i++)
  {
    Serial.print("MAC: " + String(macListArray[i].mac_adr));
    Serial.print(" Date: " + String(macListArray[i].date));
    Serial.print(" First: " + String(macListArray[i].first_seen));
    Serial.println(" Last: " + String(macListArray[i].last_seen));
  }
}

u_int32_t convertStringToSeconds(String timeString)
{
  int hours, minutes, seconds;  

    // Extract hours, minutes, and seconds from the string
    hours = timeString.substring(0, 2).toInt();
    minutes = timeString.substring(2, 4).toInt();
    seconds = timeString.substring(4, 6).toInt();

  uint32_t timeInSeconds = (hours * 3600) + (minutes * 60) + seconds;

  // Convert to total seconds
  return timeInSeconds;
}

int getMacListCountlastMinutes(int minutes)
{
  // Ausgabe der MAC-Daten
  int count = 0;

  u_int32_t actualTimeInSeconds = dataBuffer.data.timeinfo.tm_hour * 3600 + dataBuffer.data.timeinfo.tm_min * 60 + dataBuffer.data.timeinfo.tm_sec;
  u_int32_t  startTimeInSeconds = actualTimeInSeconds - (minutes * 60);

  ESP_LOGI(TAG, "MacList last %d minutes:", minutes);
  ESP_LOGI(TAG, "Actual Time in Seconds: %d ", actualTimeInSeconds);
  ESP_LOGI(TAG, "Last Seen > StartTime %d ", startTimeInSeconds);
  
  for (int i = 0; i < macListSize; i++)
  {

    int lastSeenSeconds = convertStringToSeconds(macListArray[i].last_seen);
    int firstSeenSeconds = convertStringToSeconds(macListArray[i].first_seen);
    u_int32_t deltaSeconds = lastSeenSeconds - firstSeenSeconds;   
      
      Serial.print("MAC:" + String(macListArray[i].mac_adr));
      Serial.print(" Date:" + String(macListArray[i].date));
      Serial.print(" F:" + String(macListArray[i].first_seen));
      Serial.print(" L:" + String(macListArray[i].last_seen));
      //Serial.println(" Last Seconds: " + String(lastSeenSeconds));
      Serial.print(" delta:" + String(deltaSeconds));
      if (lastSeenSeconds > startTimeInSeconds)
        { 
          Serial.println(" * ");
          count++;
        }
        else
        {
          Serial.println();
        }
  }
  return count;
}

// Funktion zum Hinzufügen oder Aktualisieren eines Datensatzes
void UpdateMacListArray(const char *mac)
{

  char date[11];
  char time[9];
  bool dublette = false;

  if (dataBuffer.data.time.year > 2020 ) 
  {

  ESP_LOGI(TAG, "Update MacList");  
  sprintf(date, "%04d%02d%02d", dataBuffer.data.time.year, dataBuffer.data.time.month, dataBuffer.data.time.day);
  sprintf(time, "%02d%02d%02d", dataBuffer.data.timeinfo.tm_hour, dataBuffer.data.timeinfo.tm_min, dataBuffer.data.timeinfo.tm_sec);

  for (int i = 0; i < macListSize; i++)
  {
    // Prüfen, ob der MAC-Adresse-Eintrag bereits existiert
    if (strcmp(macListArray[i].mac_adr, mac) == 0)
    {
      // MAC-Adresse gefunden, `last_seen` aktualisieren
      strncpy(macListArray[i].last_seen, time, sizeof(macListArray[i].last_seen) - 1);
      macListArray[i].last_seen[sizeof(macListArray[i].last_seen) - 1] = '\0';
      Serial.print("Update:"); Serial.println(mac);
      dublette = true;
    }
  }

  if (dublette)
  {
  }
  else
  {
    // Neuen Datensatz hinzufügen, wenn die MAC-Adresse nicht gefunden wurde
    if (macListSize < maxMacListSize)
    {
      strncpy(macListArray[macListSize].mac_adr, mac, sizeof(macListArray[macListSize].mac_adr) - 1);
      macListArray[macListSize].mac_adr[sizeof(macListArray[macListSize].mac_adr) - 1] = '\0';

      strncpy(macListArray[macListSize].date, date, sizeof(macListArray[macListSize].date) - 1);
      macListArray[macListSize].date[sizeof(macListArray[macListSize].date) - 1] = '\0';

      strncpy(macListArray[macListSize].first_seen, time, sizeof(macListArray[macListSize].first_seen) - 1);
      macListArray[macListSize].first_seen[sizeof(macListArray[macListSize].first_seen) - 1] = '\0';

      strncpy(macListArray[macListSize].last_seen, time, sizeof(macListArray[macListSize].last_seen) - 1);
      macListArray[macListSize].last_seen[sizeof(macListArray[macListSize].last_seen) - 1] = '\0';

      Serial.print("Append:");Serial.println(mac);

      macListSize++;
    }
  }
}  
else
{
  ESP_LOGI(TAG, "Waiting for time synconization WLAN/GPS");
}  
}

//----------------------------------------------------------------------------------------
//       File Handling
//----------------------------------------------------------------------------------------
String get_wificounter_filename()
{
  char filename[30];
  //if (dataBuffer.data.time.year > 2023)
  //{
  //  sprintf(filename, "/maclist_%04d%02d%02d.json", dataBuffer.data.time.year, dataBuffer.data.time.month, dataBuffer.data.time.day);
  //  return String(filename);
  //}
  //else
  return String("/maclist.json");
}

void load_file()
{

  DynamicJsonDocument doc(4096);
  JsonObject root = doc.to<JsonObject>();

  ESP_LOGI(TAG, "Core %d:", xPortGetCoreID());
  String filename = get_wificounter_filename();

  u_int32_t free = get_free_spiffsKB();
  

  // Open file for reading
  Serial.print("Opening:");
  Serial.println(filename);
  File file = SPIFFS.open(filename);
  if (!file)
  {
    Serial.print("Failed to open file:");
    Serial.println(filename);
    return;
  }

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else
  {
    ESP_LOGI(TAG, "Json loaded ");
    serializeJsonPretty(doc, Serial);

    // Extrahiere die "mac_list" Komponente
    JsonArray macListJson = doc["mac_list"];

    // Übertrage die JSON-Daten in das Array

    for (JsonObject macObj : macListJson)
    {
      if (macListSize >= maxMacListSize)
        break;
      strlcpy(macListArray[macListSize].mac_adr, macObj["mac_adr"], sizeof(macListArray[macListSize].mac_adr));
      strlcpy(macListArray[macListSize].date, macObj["date"], sizeof(macListArray[macListSize].date));
      strlcpy(macListArray[macListSize].first_seen, macObj["first_seen"], sizeof(macListArray[macListSize].first_seen));
      strlcpy(macListArray[macListSize].last_seen, macObj["last_seen"], sizeof(macListArray[macListSize].last_seen));
      macListSize++;
    }

    // Ausgabe der MAC-Daten
    for (int i = 0; i < macListSize; i++)
    {
      Serial.println("MAC: " + String(macListArray[i].mac_adr));
      Serial.println("Date: " + String(macListArray[i].date));
      Serial.println("First: " + String(macListArray[i].first_seen));
      Serial.println("Last: " + String(macListArray[i].last_seen));
      Serial.println();
    }
  }
  file.close();
}



void save_file()
{

  DynamicJsonDocument doc(4096);
  JsonObject root = doc.to<JsonObject>();

  String filename = get_wificounter_filename();
  
  // Füge das String-Feld hinzu
  doc["string"] = "Hello World";

  // Erstelle das mac_list-Array
  JsonArray macList = doc.createNestedArray("mac_list");

  // Kopiere die Elemente des Arrays in das JSON-Dokument
  for (int i = 0; i < macListSize; i++)
  {
    JsonObject macData = macList.createNestedObject();
    macData["mac_adr"] = macListArray[i].mac_adr;
    macData["date"] = macListArray[i].date;
    macData["first_seen"] = macListArray[i].first_seen;
    macData["last_seen"] = macListArray[i].last_seen;
  }

  // Open file for reading
  Serial.print("Saving:");
  Serial.println(filename);
  File file = SPIFFS.open(filename, "w");
  // Serialize the JSON document to the file
  size_t bytesWritten = serializeJson(doc, file);
  if (bytesWritten == 0)
  {
    Serial.print("Failed to write to file");
    Serial.println(filename);
  }
  else
  {
    Serial.print("Successfully wrote ");
    Serial.print(bytesWritten);
    Serial.println(" bytes to file");
  }

  file.close();

  dataBuffer.wificounter.fileSize = u_int16_t(bytesWritten / 1024);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
};

// Software-timer driven Wifi channel rotation callback function
void switchWifiChannel(TimerHandle_t xTimer)
{
  configASSERT(xTimer);
  do
  {
    channel_rotation = (channel_rotation % country.nchan) + 1; // rotate channels in bitmap
  } while (!(channels_map >> (channel_rotation - 1) & 1));
  esp_wifi_set_channel(channel_rotation, WIFI_SECOND_CHAN_NONE); // we use HT20 bandwith
}

void wifi_sniffer_init(void)
{
  ESP_LOGI(TAG, "Core %d:", xPortGetCoreID());
  // initialize wifi driver with settings tuned for sniffing
  wifi_init_config_t wificfg = WIFI_INIT_CONFIG_DEFAULT();
  wificfg.nvs_enable = 0;         // we don't need any wifi settings from NVRAM
  wificfg.wifi_task_core_id = 0;  // we want wifi task running on core 0
  wificfg.static_rx_buf_num = 16; // increase RX buffer to minimize packet loss
  wificfg.dynamic_rx_buf_num = 64;
  wificfg.rx_ba_win = 32;  // should be twice of static_rx_buf_num
  wificfg.tx_buf_type = 1; // we don't TX, thus keep tx memory footprint small
  wificfg.static_tx_buf_num = 0;
  wificfg.dynamic_tx_buf_num = 4;
  wificfg.cache_tx_buf_num = 4; // can't be zero!

  // filter management and data frames to the sniffer
  wifi_promiscuous_filter_t filter = {.filter_mask =
                                          WIFI_PROMIS_FILTER_MASK_MGMT |
                                          WIFI_PROMIS_FILTER_MASK_DATA};

  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_init(&wificfg));
  ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country)); /* set country for channel range [1, 13] */
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter)); // enable frame filtering
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true)); // start sniffer mode

  // setup wifi channel rotation timer
  // if (WIFI_CHANNEL_SWITCH_INTERVAL > 0) {
  //   WifiChanTimer = xTimerCreate(
  //       "WifiChannelTimer", pdMS_TO_TICKS(WIFI_CHANNEL_SWITCH_INTERVAL * 10),
  //       pdTRUE, (void*)0, switchWifiChannel);
  //   assert(WifiChanTimer);
  //   xTimerStart(WifiChanTimer, 0);
  //}
}

void wifi_sniffer_set_channel(uint8_t channel_rotation)
{
  esp_wifi_set_channel(channel_rotation, WIFI_SECOND_CHAN_NONE);
}

const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
  switch (type)
  {
  case WIFI_PKT_MGMT:
    return "MGMT";
  case WIFI_PKT_DATA:
    return "DATA";
  default:
  case WIFI_PKT_MISC:
    return "MISC";
  }
}

void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type)
{
  char macString[18]; // Buffer for the formatted string
  String output;

  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  // Format the MAC address into the buffer
  snprintf(macString, sizeof(macString), "%02X:%02X:%02X:%02X:%02X:%02X",
           hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
           hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
 

  UpdateMacListArray(macString);
  

  printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
         " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x,"
         " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
         " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
         wifi_sniffer_packet_type2str(type),
         ppkt->rx_ctrl.channel,
         ppkt->rx_ctrl.rssi,
         /* ADDR1 */
         hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
         hdr->addr1[3], hdr->addr1[4], hdr->addr1[5],
         /* ADDR2 */
         hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
         hdr->addr2[3], hdr->addr2[4], hdr->addr2[5],
         /* ADDR3 */
         hdr->addr3[0], hdr->addr3[1], hdr->addr3[2],
         hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);
}

u_int16_t wifi_count_get()
{

  if (macListSize > 0)
  {
    esp_wifi_set_promiscuous(false); // now switch off monitor mode
    delay(100);
    save_file();
  }

  //printMacList();
  return macListSize;
}

void t_getWifiCount(void *parameter)
{
  DataBuffer *locdataBuffer;
  locdataBuffer = (DataBuffer *)parameter;

  // Continuously sample ADC1
  while (1)
  {
    esp_wifi_set_promiscuous(true); // now switch on  for 15 seconds
    vTaskDelay(15000);
    esp_wifi_set_promiscuous(false); // now switch OFF  for 30 seconds
    vTaskDelay(30000);
  }
}

void setup_wificounter_RTOS()
{
  wifi_sniffer_init();
  xTaskCreatePinnedToCore(
      t_getWifiCount,      /* Task function. */
      "globalClassTask",   /* String with name of task. */
      10000,               /* Stack size in words. */
      (void *)&dataBuffer, /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,
      0); /* Task handle. */
}

// the setup function runs once when you press reset or power the board
void setup_wifi_counter()
{

  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL)
  {
    ESP_LOGE(TAG, "Failed to create mutex");
    return;
  }

  load_file();
  //  wifi_sniffer_init();
  setup_wificounter_RTOS();
}

// the loop function runs over and over again forever
void wifi_counter_loop()
{
  vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
  wifi_sniffer_set_channel(channel_rotation);
  channel_rotation = (channel_rotation % WIFI_CHANNEL_MAX) + 1;
}
