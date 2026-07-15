#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#define SERVO_PULSE_MIN    2500  // 1.0 ms @ 400 ns/tick
#define SERVO_PULSE_MAX    5000  // 2.0 ms @ 400 ns/tick
#define SERVO_PULSE_NEUT   3750  // 1.5 ms @ 400 ns/tick
#define SERVO_FRAME_TICKS  50000U // 20 ms @ 400 ns/tick

void servo_init(void);
void servo_set_angle(uint8_t servo_id, uint16_t angle_deg);
uint16_t servo_get_angle(uint8_t servo_id);
void servo_timer3_isr(void);
void servo_ccp2_isr(void);

#endif
