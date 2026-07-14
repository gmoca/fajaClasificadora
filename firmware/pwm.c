#include "pwm.h"
#include "config.h"

void pwm_hbridge_init(void) {
    CCP1CONbits.CCP1M = 0b1100;
    CCPR1L = 0;
    TRISCbits.TRISC2 = 0;
}

void pwm_hbridge_set_duty(uint8_t duty) {
    CCPR1L = duty >> 2;
    CCP1CONbits.DC1B = duty & 0x03;
}
