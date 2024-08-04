/*

LICENSE

Copyright  2020      Deutsche Bahn Station&Service AG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


This file is based of the ESP32-Paxcounter:

 * Copyright  2018-2020 Klaus Wilting <verkehrsrot@arcor.de>
 * https://github.com/cyberman54/ESP32-Paxcounter
 * https://github.com/cyberman54/ESP32-Paxcounter/blob/30731f5c0ce5396fdbcc0d5147481a5c69e15bff/src/wifiscan.cpp

Which in turn is based of Łukasz Marcin Podkalicki's ESP32/016 WiFi Sniffer

 * Copyright 2017 Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * https://github.com/lpodkalicki/blog/tree/master/esp32/016_wifi_sniffer

*/
#include "globals.h"
//#include "libpax.h"
#include "wifiscan.h"


#define WIFI_CHANNEL_ALL    0b1111111111111
#define WIFI_CHANNEL_1      0b0000000000001
#define WIFI_CHANNEL_2      0b0000000000010
#define WIFI_CHANNEL_3      0b0000000000100
#define WIFI_CHANNEL_4      0b0000000001000
#define WIFI_CHANNEL_5      0b0000000010000
#define WIFI_CHANNEL_6      0b0000000100000
#define WIFI_CHANNEL_7      0b0000001000000
#define WIFI_CHANNEL_8      0b0000010000000
#define WIFI_CHANNEL_9      0b0000100000000
#define WIFI_CHANNEL_10     0b0001000000000
#define WIFI_CHANNEL_11     0b0010000000000
#define WIFI_CHANNEL_12     0b0100000000000
#define WIFI_CHANNEL_13     0b1000000000000

#define LIBPAX_ERROR_WIFI_NOT_AVAILABLE 0b00000001
#define LIBPAX_ERROR_BLE_NOT_AVAILABLE  0b00000010

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

#define LIBPAX_MAX_SIZE 0xFFFF  // full enumeration of uint16_t
#define LIBPAX_MAP_SIZE (LIBPAX_MAX_SIZE / BITS_PER_WORD)

typedef enum { MAC_SNIFF_WIFI, MAC_SNIFF_BLE, MAC_SNIFF_BLE_ENS } snifftype_t;
typedef uint32_t bitmap_t;
enum { BITS_PER_WORD = sizeof(bitmap_t) * CHAR_BIT };

DRAM_ATTR bitmap_t seen_ids_map[LIBPAX_MAP_SIZE];
int seen_ids_count = 0;

uint16_t macs_wifi = 0;
uint16_t macs_ble = 0;
uint8_t channel = 0;  // channel rotation counter

IRAM_ATTR void set_id(bitmap_t *bitmap, uint16_t id) {
  bitmap[WORD_OFFSET(id)] |= ((bitmap_t)1 << BIT_OFFSET(id));
}

IRAM_ATTR int get_id(bitmap_t *bitmap, uint16_t id) {
  bitmap_t bit = bitmap[WORD_OFFSET(id)] & ((bitmap_t)1 << BIT_OFFSET(id));
  return bit != 0;
}

/** remember given id
 * returns 1 if id is new, 0 if already seen this is since last reset
 */
IRAM_ATTR int add_to_bucket(uint16_t id) {
  if (get_id(seen_ids_map, id)) {
    return 0;  // already seen
  } else {
    set_id(seen_ids_map, id);
    seen_ids_count++;
    return 1;  // new
  }
}


IRAM_ATTR int mac_add(uint8_t *paddr, snifftype_t sniff_type) {
  uint16_t *id;
  // mac addresses are 6 bytes long, we only use the last two bytes
  id = (uint16_t *)(paddr + 4);
    
  //ESP_LOGD(TAG, "MAC=%02x:%02x:%02x:%02x:%02x:%02x -> ID=%04x", paddr[0],
  //         paddr[1], paddr[2], paddr[3], paddr[4], paddr[5], *id);
    
  // if it is NOT a locally administered ("random") mac, we don't count it
  if (!(paddr[0] & 0b10)) return false;
  
  int added = add_to_bucket(*id);

  // Count only if MAC was not yet seen
  if (added) {
    if(sniff_type == MAC_SNIFF_BLE) {
      macs_ble++;
    } else if(sniff_type == MAC_SNIFF_WIFI)  {
      macs_wifi++;
    }
  };  // added

  return added;  // function returns bool if a new and unique Wifi or BLE mac
                 // was counted (true) or not (false)
}


TimerHandle_t WifiChanTimer;
int initialized_wifi = 0;
int wifi_rssi_threshold = 0;
uint16_t channels_map = WIFI_CHANNEL_ALL;
static wifi_country_t country;

void wifi_noop_sniffer(void* buff, wifi_promiscuous_pkt_type_t type) {}

// using IRAM_ATTR here to speed up callback function
static IRAM_ATTR void wifi_sniffer_packet_handler(
    void* buff, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t* ppkt = (wifi_promiscuous_pkt_t*)buff;
  const wifi_ieee80211_packet_t* ipkt = (wifi_ieee80211_packet_t*)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t* hdr = &ipkt->hdr;

  if ((wifi_rssi_threshold) &&
      (ppkt->rx_ctrl.rssi < wifi_rssi_threshold))  // rssi is negative value
    return;
  else
    mac_add((uint8_t*)hdr->addr2, MAC_SNIFF_WIFI);
}

// Software-timer driven Wifi channel rotation callback function
void switchWifiChannel(TimerHandle_t xTimer) {
  configASSERT(xTimer);
  do {
    channel = (channel % country.nchan) + 1;  // rotate channels in bitmap
  } while (!(channels_map >> (channel - 1) & 1));
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);  // we use HT20 bandwith
}

void set_wifi_country(const char* country_code) {
  ESP_ERROR_CHECK(esp_wifi_set_country_code(country_code, true));
  ESP_ERROR_CHECK(esp_wifi_get_country(&country));
}

// Keep this a while for compatibility with 1.0.1
void set_wifi_country(uint8_t cc) {
  switch (cc) {
    case 1:
      set_wifi_country("DE");
      break;
  }
}

void set_wifi_channels(uint16_t set_channels_map) {
  channels_map = set_channels_map;
}

void set_wifi_rssi_filter(int set_rssi_threshold) {
  wifi_rssi_threshold = set_rssi_threshold;
}

void wifi_sniffer_init(uint16_t wifi_channel_switch_interval) {
#ifdef LIBPAX_WIFI

  // initialize NVS, is used to store wifi PHY calibration data
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // initialize wifi driver with settings tuned for sniffing
  wifi_init_config_t wificfg = WIFI_INIT_CONFIG_DEFAULT();
  wificfg.nvs_enable = 0;          // we don't need any wifi settings from NVRAM
  wificfg.wifi_task_core_id = 0;   // we want wifi task running on core 0
  wificfg.static_rx_buf_num = 16;  // increase RX buffer to minimize packet loss
  wificfg.dynamic_rx_buf_num = 64;
  wificfg.rx_ba_win = 32;   // should be twice of static_rx_buf_num
  wificfg.tx_buf_type = 1;  // we don't TX, thus keep tx memory footprint small
  wificfg.static_tx_buf_num = 0;
  wificfg.dynamic_tx_buf_num = 4;
  wificfg.cache_tx_buf_num = 4;  // can't be zero!

  // filter management and data frames to the sniffer
  wifi_promiscuous_filter_t filter = {.filter_mask =
                                          WIFI_PROMIS_FILTER_MASK_MGMT |
                                          WIFI_PROMIS_FILTER_MASK_DATA};

  ESP_ERROR_CHECK(esp_wifi_init(&wificfg));  // configure Wifi with cfg
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(
      esp_wifi_set_promiscuous_filter(&filter));  // enable frame filtering
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));  // start sniffer mode

  // setup wifi channel rotation timer
  if (wifi_channel_switch_interval > 0) {
    WifiChanTimer = xTimerCreate(
        "WifiChannelTimer", pdMS_TO_TICKS(wifi_channel_switch_interval * 10),
        pdTRUE, (void*)0, switchWifiChannel);
    assert(WifiChanTimer);
    xTimerStart(WifiChanTimer, 0);
  }

  initialized_wifi = 1;
#endif
}

void wifi_sniffer_stop() {
#ifdef LIBPAX_WIFI
  if (initialized_wifi) {
    if (WifiChanTimer) xTimerStop(WifiChanTimer, 0);
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_noop_sniffer));
    ESP_ERROR_CHECK(
        esp_wifi_set_promiscuous(false));  // now switch off monitor mode
    esp_wifi_deinit();
    initialized_wifi = 0;
  }
#endif
}


void setup_wifiscanner() {
  wifi_sniffer_init(50);
  set_wifi_country("DE");
  set_wifi_channels(WIFI_CHANNEL_ALL);
  set_wifi_rssi_filter(0);
}
