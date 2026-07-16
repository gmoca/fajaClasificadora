#include "pwm.h"
#include "config.h"

void pwm_hbridge_init(void) {
    CCP1CONbits.CCP1M = 0b1100;
    CCPR1L = 0;
    TRISCbits.TRISC2 = 0;
}

void pwm_hbridge_set_duty(uint8_t duty) {
    // Escalar de 0-255 a 0-1000 (resolución máxima de PR2 = 249)
    uint16_t val = ((uint16_t)duty * 1000) / 255;
    CCPR1L = val >> 2;
    CCP1CONbits.DC1B = val & 0x03;
}
