#ifndef _LORA_H
#define _LORA_H

// #include "globals.h"

const unsigned TX_INTERVAL = 30;
extern uint8_t txBuffer[9];
extern QueueHandle_t LoraSendQueue;



//--------------------------------------------------------------------------
// LORA Settings
//--------------------------------------------------------------------------
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = {0x88, 0x06, 0xDA, 0xCF, 0x30, 0xFB, 0x44, 0xDC, 0x69, 0x0E, 0x15, 0xF8, 0xAD, 0xCB, 0x40, 0x6C};

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = {0xB3, 0xB1, 0x59, 0x5D, 0x24, 0xBD, 0xD2, 0xF5, 0x6A, 0x17, 0x0A, 0x94, 0xF2, 0xED, 0xDB, 0xC2};
static u4_t DEVADDR = 0x260118B7; // <-- Change this address for every node!

void os_getArtEui(u1_t *buf);
void os_getDevEui(u1_t *buf);
void os_getDevKey(u1_t *buf);

static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty cycle limitations).

void t_enqueue_LORA_messages();
void do_send(osjob_t *j);
void t_LORA_send_from_queue(osjob_t *j);
void dump_queue();
void queue_aging();
void dump_single_message(MessageBuffer_t SendBuffer);
void setup_lora();
#endif