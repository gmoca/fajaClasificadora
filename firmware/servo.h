#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#define SERVO_PULSE_MIN    2500
#define SERVO_PULSE_MAX    5000
#define SERVO_PULSE_NEUT   3750
#define SERVO_FRAME_TICKS  50000

void servo_init(void);
void servo_set_angle(uint8_t servo_id, uint16_t angle_deg);
void servo_ccp2_isr(void);
void servo_step(void);

#endif
