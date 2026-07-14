#ifndef PWM_H
#define PWM_H

#include <stdint.h>

void pwm_hbridge_init(void);
void pwm_hbridge_set_duty(uint8_t duty);

#endif
