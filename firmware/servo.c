#include "servo.h"
#include "config.h"

static volatile uint16_t servo1_pulse = SERVO_PULSE_NEUT;
static volatile uint8_t  servo1_ready = 0;

static volatile uint16_t servo2_pulse = SERVO_PULSE_NEUT;

void servo_init(void) {
    T3CONbits.T3CCP2 = 0;

    LATCbits.LATC1 = 0;
    TRISCbits.TRISC1 = 0;

    CCP2CONbits.CCP2M = 0b0101;
    CCPR2 = 0;
    PIE2bits.CCP2IE = 1;
    IPR2bits.CCP2IP = 0;
    servo1_ready = 1;

    TRISCbits.TRISC0 = 0;
    LATCbits.LATC0 = 0;
}

void servo_set_angle(uint8_t servo_id, uint16_t angle_deg) {
    if (angle_deg > 180) angle_deg = 180;
    uint16_t pulse = SERVO_PULSE_MIN
        + ((uint32_t)(SERVO_PULSE_MAX - SERVO_PULSE_MIN) * angle_deg / 180);
    if (servo_id == 1)
        servo1_pulse = pulse;
    else
        servo2_pulse = pulse;
}

void servo_ccp2_isr(void) {
    static uint8_t phase = 0;

    if (phase == 0) {
        LATCbits.LATC1 = 1;
        CCP2CONbits.CCP2M = 0b0101;
        CCPR2 = servo1_pulse;
        phase = 1;
    } else if (phase == 1) {
        CCP2CONbits.CCP2M = 0b0111;
        CCPR2 = SERVO_FRAME_TICKS;
        phase = 2;
    } else {
        phase = 0;
        LATCbits.LATC1 = 1;
        CCP2CONbits.CCP2M = 0b0101;
        CCPR2 = servo1_pulse;
    }
}

void servo_step(void) {
    static uint16_t tick = 0;
    tick++;
    if (tick >= 20) {
        tick = 0;
        LATCbits.LATC0 = 1;
    }
    if (tick == (servo2_pulse / 2500))
        LATCbits.LATC0 = 0;
}
