#ifndef _LORA_H
#define _LORA_H

#include "globals.h"

extern uint8_t txBuffer[9];

static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 60;

void t_enqueue_LORA_messages();
void do_send(osjob_t *j);
void t_LORA_send_from_queue(osjob_t *j);
#endif