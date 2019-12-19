
#ifndef _TTGOBEAM_H
#define _TTGOBEAM_H

#include <stdint.h>

// Hardware related definitions for TTGO T-Beam board
// (only) for newer T-Beam version T22_V10
// pinouts taken from https://github.com/lewisxhe/TTGO-T-Beam

#define HAS_LORA                1               // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio        1               // HPD13A LoRa SoC
#define BOARD_HAS_PSRAM         1               // use extra 4MB external RAM
#define HAS_BUTTON              GPIO_NUM_39     // middle on board button
#define HAS_PMU                 0               // AXP192 power management chip
#define PMU_INT                 GPIO_NUM_35     // AXP192 interrupt

#define HAS_LED GPIO_NUM_14 // on board green LED, only new version TTGO-BEAM V07
#define BUILTIN_LED GPIO_NUM_14


// GPS settings
#define HAS_GPS 1 // use on board GPS
#define GPS_TX          12
#define GPS_RX          15

#endif
