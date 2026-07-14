#include "servo.h"
#include "config.h"

static volatile uint16_t servo1_pulse = SERVO_PULSE_NEUT;
static volatile uint8_t  servo1_ready = 0;

static volatile uint16_t servo2_pulse = SERVO_PULSE_NEUT;

/* TMR3 reload for 25 µs @ prescaler 1:1, Fcyc = 5 MHz (200 ns/tick)
 * 25 µs / 200 ns = 125 ticks → reload = 65536 - 125 = 65411 */
#define SERVO2_TMR3_RELOAD  65411

void servo_init(void) {
    T3CONbits.T3CCP2 = 0;

    LATCbits.LATC1 = 0;
    TRISCbits.TRISC1 = 0;

    CCP2CONbits.CCP2M = 0b0101;
    CCPR2 = 0;
    PIE2bits.CCP2IE = 1;
    IPR2bits.CCP2IP = 0;
    servo1_ready = 1;

    T3CONbits.TMR3ON = 0;
    T3CONbits.TMR3CS = 0;
    T3CONbits.T3CKPS0 = 0;
    T3CONbits.T3CKPS1 = 0;
    T3CONbits.T3CCP2 = 0;
    TMR3H = (uint8_t)(SERVO2_TMR3_RELOAD >> 8);
    TMR3L = (uint8_t)(SERVO2_TMR3_RELOAD & 0xFF);
    PIR2bits.TMR3IF = 0;
    PIE2bits.TMR3IE = 1;
    IPR2bits.TMR3IP = 0;
    TRISCbits.TRISC0 = 0;
    LATCbits.LATC0 = 0;
    T3CONbits.TMR3ON = 1;
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

uint16_t servo_get_angle(uint8_t servo_id) {
    uint16_t pulse = (servo_id == 1) ? servo1_pulse : servo2_pulse;
    if (pulse <= SERVO_PULSE_MIN) return 0;
    if (pulse >= SERVO_PULSE_MAX) return 180;
    return (uint16_t)(((uint32_t)(pulse - SERVO_PULSE_MIN) * 180) / (SERVO_PULSE_MAX - SERVO_PULSE_MIN));
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

/* Servo 2 — software PWM via TMR3 ISR (25 µs/tick)
 * servo2_pulse (400 ns ticks) is converted to 25 µs steps:
 *   step = tick * 400 / 25000 = tick / 62.5 → (tick * 2) / 125
 *   pulse 1.0 ms (2500) → 40 steps, 2.0 ms (5000) → 80 steps
 *   800 steps × 25 µs = 20 ms frame */
void servo_timer3_isr(void) {
    static uint16_t tick = 0;
    tick++;
    if (tick >= 800) {
        tick = 0;
        LATCbits.LATC0 = 1;
    }
    if (tick == (uint16_t)((servo2_pulse * 2UL) / 125))
        LATCbits.LATC0 = 0;
    TMR3H = (uint8_t)(SERVO2_TMR3_RELOAD >> 8);
    TMR3L = (uint8_t)(SERVO2_TMR3_RELOAD & 0xFF);
}
