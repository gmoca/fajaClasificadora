#ifndef BT_PROTOCOL_H
#define BT_PROTOCOL_H

#include <stdint.h>

extern uint32_t last_bt_activity;

void bt_protocol_init(void);
void bt_protocol_process(void);

#endif
