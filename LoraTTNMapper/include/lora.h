#ifndef _LORA_H
#define _LORA_H

#include "globals.h"

extern uint8_t txBuffer[9];
extern QueueHandle_t LoraSendQueue;

#define TTN_COMMAND_RESET_COULOMB 0x01
#define TTN_COMMAND_SLEEP 0x02

//--------------------------------------------------------------------------
// LORA Settings
//--------------------------------------------------------------------------

// LORA Bordspecific settings

#if DEVICE_ID == 1 // mrflexi-01 Device EU ID = DE0000000000100  TTN V3
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = {0x99, 0x14, 0xEE, 0xA1, 0x1A, 0xEE, 0x3F, 0x53, 0x62, 0x38, 0xC8, 0x52, 0x57, 0x9A, 0x6D, 0x90};

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = {0x9a, 0xa7, 0xE6, 0x73, 0x30, 0x18, 0x51, 0x7F, 0xF7, 0x55, 0xF0, 0xE4, 0x38, 0xBA, 0x84, 0x6c};

static u4_t DEVADDR = 0x260B078D; //
#endif

#if DEVICE_ID == 2 // mrflexi-02 Device EU ID = DE00000000000101 TTN V3
// LoRaWAN NwkSKey, network session key // NwkSKey // msb
static PROGMEM u1_t NWKSKEY[16] = {0x4F, 0xAE, 0x23, 0x49, 0x0A, 0xF6, 0x3E, 0x7A, 0x52, 0x1F, 0x94, 0x44, 0x5D, 0x2C, 0x3B, 0x3A};

// LoRaWAN AppSKey, application session key // AppSKey// msb
static u1_t PROGMEM APPSKEY[16] = {0x11 , 0xDE , 0xC9 , 0x4E , 0x2D , 0x77 , 0xC0 , 0x6F , 0xD4 , 0x82 , 0x1F , 0xD0 , 0xD8 , 0x08 , 0x57 , 0x13};
static u4_t DEVADDR = 0x260BEFEC; // msb
#endif

#if DEVICE_ID == 3 // nix
// LoRaWAN NwkSKey, network session key // msb
static PROGMEM u1_t NWKSKEY[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// LoRaWAN AppSKey, application session key // msb
static u1_t PROGMEM APPSKEY[16] = {0x59, 0xB9, 0x24, 0x25, 0xF3, 0xB5, 0xB3, 0xF6, 0x13, 0xCC, 0x0D, 0x6C, 0x98, 0xBD, 0x25, 0xF0};
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