#include "servo.h"
#include "config.h"

static volatile uint16_t servo1_pulse = SERVO_PULSE_NEUT;
static volatile uint16_t servo1_steps = 60;  // 1.5ms neutral default (60 * 25us = 1.5ms)

static volatile uint16_t servo2_pulse = SERVO_PULSE_NEUT;
static volatile uint16_t servo2_steps = 60;  // 1.5ms neutral default (60 * 25us = 1.5ms)

/* TMR3 reload for 25 µs @ prescaler 1:1, Fcyc = 5 MHz (200 ns/tick)
 * 25 µs / 200 ns = 125 ticks → reload = 65536 - 125 = 65411 */
#define SERVO_TMR3_RELOAD  65411

void servo_init(void) {
    // Configure RC1 (Servo 1) and RC0 (Servo 2) as digital outputs
    LATCbits.LATC1 = 0;
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC0 = 0;
    TRISCbits.TRISC0 = 0;

    // Initialize Timer 3 for software PWM
    T3CONbits.TMR3ON = 0;
    T3CONbits.TMR3CS = 0;
    T3CONbits.T3CKPS0 = 0;
    T3CONbits.T3CKPS1 = 0;
    T3CONbits.T3CCP2 = 0;
    TMR3H = (uint8_t)(SERVO_TMR3_RELOAD >> 8);
    TMR3L = (uint8_t)(SERVO_TMR3_RELOAD & 0xFF);
    PIR2bits.TMR3IF = 0;
    PIE2bits.TMR3IE = 1;
    IPR2bits.TMR3IP = 0;
    T3CONbits.TMR3ON = 1;
}

void servo_set_angle(uint8_t servo_id, uint16_t angle_deg) {
    if (angle_deg > 180) angle_deg = 180;
    uint16_t pulse = SERVO_PULSE_MIN
        + ((uint32_t)(SERVO_PULSE_MAX - SERVO_PULSE_MIN) * angle_deg / 180);
    if (servo_id == 1) {
        servo1_pulse = pulse;
        servo1_steps = (uint16_t)(((uint32_t)pulse * 2UL) / 125);
    } else {
        servo2_pulse = pulse;
        servo2_steps = (uint16_t)(((uint32_t)pulse * 2UL) / 125);
    }
}

uint16_t servo_get_angle(uint8_t servo_id) {
    uint16_t pulse = (servo_id == 1) ? servo1_pulse : servo2_pulse;
    if (pulse <= SERVO_PULSE_MIN) return 0;
    if (pulse >= SERVO_PULSE_MAX) return 180;
    return (uint16_t)(((uint32_t)(pulse - SERVO_PULSE_MIN) * 180) / (SERVO_PULSE_MAX - SERVO_PULSE_MIN));
}

/* Dual software PWM generation via TMR3 ISR (runs every 25 µs)
 * Precalculated steps used to avoid division inside ISR */
void servo_timer3_isr(void) {
    static uint16_t tick = 0;
    tick++;
    if (tick >= 800) {
        tick = 0;
        LATCbits.LATC0 = 1; // Start pulse for Servo 2 (RC0)
        LATCbits.LATC1 = 1; // Start pulse for Servo 1 (RC1)
    }
    if (tick == servo2_steps)
        LATCbits.LATC0 = 0; // End pulse for Servo 2
    if (tick == servo1_steps)
        LATCbits.LATC1 = 0; // End pulse for Servo 1

    TMR3H = (uint8_t)(SERVO_TMR3_RELOAD >> 8);
    TMR3L = (uint8_t)(SERVO_TMR3_RELOAD & 0xFF);
}
