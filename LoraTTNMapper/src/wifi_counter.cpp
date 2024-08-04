#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <ArduinoJson.h>
#define WIFI_CHANNEL_SWITCH_INTERVAL (500)
#define WIFI_CHANNEL_MAX (13)

uint8_t level = 0, channel_rotation = 1;
uint32_t mac_array[100];
uint8_t new_mac = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_init(void);
static void wifi_sniffer_set_channel(uint8_t channel_rotation);
static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

static wifi_country_t wifi_country = {.cc = "DE", .schan = 1, .nchan = 13}; // Most recent esp32 library struct

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

// Create a DynamicJsonDocument
DynamicJsonDocument doc(2048);

// Create the root object
JsonObject root = doc.to<JsonObject>();

// Create an array named "mac_list" in the root object
JsonArray macList = root.createNestedArray("mac_list");

// Function to add objects to the mac_list array
// auto addMacObjectToArray = [](JsonArray &array, const char *mac_adr, const char *date, const char *first_seen, const char *last_seen)
//{
// JsonObject obj = array.createNestedObject();
//  obj["mac_adr"] = mac_adr;
//  obj["date"] = date;
//  obj["first_seen"] = first_seen;
//  obj["last_seen"] = last_seen;
//};

auto addUniqueObject = [](JsonArray &array, String mac_adr, const char *date, const char *first_seen, const char *last_seen)
{
  bool dublette = false;
  uint8_t dub_cnt = 0;

  dublette == false;

  // Check if the mac_adr already exists in the array
  for (JsonObject obj : array)
  {
    Serial.println("---- Check ---");

    Serial.println(obj["mac_adr"].as<String>());
    Serial.println(mac_adr);

    if (obj["mac_adr"] == mac_adr) 
    {
      // The mac_adr already exists, do not add a new object
      Serial.println("dublette");
      dublette = true;
      dub_cnt++;
      break;
    }
  }

  if (dublette == false)
  {
    JsonObject new_obj = array.createNestedObject();
    new_obj["mac_adr"] = mac_adr;
    new_obj["date"] = date;
    new_obj["first_seen"] = first_seen;
    new_obj["last_seen"] = last_seen;
    Serial.print("-----------------------------------------------------> MAC added");Serial.println(mac_adr);
  }
};

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
};

void wifi_sniffer_init(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country)); /* set country for channel range [1, 13] */
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
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
  
  

  Serial.println("Before add:");
  serializeJsonPretty(doc, output);
  Serial.println(output);

  //const char macAdr = macString;
  addUniqueObject(macList, macString, "04.08.2024", "12:00:00", "13:20:00");

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

  // Serialize the JSON document to a string and print it
  Serial.println("After add:");
  serializeJsonPretty(doc, output);
  Serial.println(output);


  // Count the number of objects in the array
  size_t objectCount = macList.size();

  // Print the number of objects
  Serial.print("Number of objects in 'mac_list': ");
  Serial.println(objectCount);
}

// the setup function runs once when you press reset or power the board
void setup_wifi_counter()
{
  wifi_sniffer_init();
  //addUniqueObject(macList, "aa:bb:cc:dd:ee:ff", "04.08.2024", "12:00:00", "12:00:00");
}

// the loop function runs over and over again forever
void wifi_counter_loop()
{
  vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
  wifi_sniffer_set_channel(channel_rotation);
  channel_rotation = (channel_rotation % WIFI_CHANNEL_MAX) + 1;
}
