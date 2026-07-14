#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void encoder_init(void);
uint32_t encoder_get_pulses(void);
uint16_t encoder_get_speed_mm_s(void);
void encoder_reset(void);
void encoder_isr_handler(void);
void encoder_tick_handler(void);

#endif
