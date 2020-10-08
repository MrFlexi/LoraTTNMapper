#ifndef _LORA_H
#define _LORA_H

#include "globals.h"

// const unsigned LORA_TX_INTERVAL = 60;
extern uint8_t txBuffer[9];
extern QueueHandle_t LoraSendQueue;


#define TTN_COMMAND_RESET_COULOMB           0x01
#define TTN_COMMAND_SLEEP                   0x02



//--------------------------------------------------------------------------
// LORA Settings
//--------------------------------------------------------------------------


// LORA Bordspecific settings

#if DEVICE_ID == 1                 // TBEAM-01 Device EU ID = DE00000000000010
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = {0x88, 0x06, 0xDA, 0xCF, 0x30, 0xFB, 0x44, 0xDC, 0x69, 0x0E, 0x15, 0xF8, 0xAD, 0xCB, 0x40, 0x6C};

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = {0xB3, 0xB1, 0x59, 0x5D, 0x24, 0xBD, 0xD2, 0xF5, 0x6A, 0x17, 0x0A, 0x94, 0xF2, 0xED, 0xDB, 0xC2};
static u4_t DEVADDR = 0x260118B7; // 
#endif

#if DEVICE_ID == 2                 // TBEAM-02 Device EU ID = DE00000000000011
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = { 0xEE, 0x1A, 0x8E, 0x21, 0x51, 0xCD, 0x64, 0xC5, 0x7D, 0xF7, 0x97, 0x5D, 0x69, 0xDD, 0x81, 0x26 };

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = { 0x59, 0xB9, 0x24, 0x25, 0xF3, 0xB5, 0xB3, 0xF6, 0x13, 0xCC, 0x0D, 0x6C, 0x98, 0xBD, 0x25, 0xF0};
static u4_t DEVADDR = 0x26011F7F; // msb 
#endif

#if DEVICE_ID == 3                 // nix
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = { 0x59, 0xB9, 0x24, 0x25, 0xF3, 0xB5, 0xB3, 0xF6, 0x13, 0xCC, 0x0D, 0x6C, 0x98, 0xBD, 0x25, 0xF0};
static u4_t DEVADDR = 0x26011F7F; // msb 
#endif


void os_getArtEui(u1_t *buf);
void os_getDevEui(u1_t *buf);
void os_getDevKey(u1_t *buf);

static osjob_t sendjob;

void t_enqueue_LORA_messages();
void do_send(osjob_t *j);
void t_LORA_send_from_queue(osjob_t *j);
void dump_queue();
void dump_single_message(MessageBuffer_t SendBuffer);
void setup_lora();
#endif