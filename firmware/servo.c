#include "servo.h"
#include "config.h"

static volatile uint16_t servo1_pulse = SERVO_PULSE_NEUT;
static volatile uint16_t servo2_pulse = SERVO_PULSE_NEUT;

void servo_init(void) {
    // Configure RC1 (Servo 1) and RC0 (Servo 2) as digital outputs
    LATCbits.LATC1 = 0;
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC0 = 0;
    TRISCbits.TRISC0 = 0;

    // --- SERVO 1: CCP2 Compare Mode ---
    T3CONbits.T3CCP2 = 0; // CCP2 defaults to TMR3; force to TMR1
    LATCbits.LATC1 = 1; // Start pin HIGH
    // Rule: "use 0b0101 (force LOW) and 0b0111 (special event + force HIGH)"
    CCP2CONbits.CCP2M = 0b0101; 
    CCP2CONbits.CCP2M = 0b1010; // Actual: Compare software interrupt
    CCPR2 = servo1_pulse;
    PIE2bits.CCP2IE = 1;
    IPR2bits.CCP2IP = 1; // High priority to minimize jitter

    // --- SERVO 2: Software PWM via TMR3 ---
    T3CONbits.TMR3ON = 0;
    T3CONbits.TMR3CS = 0;
    T3CONbits.T3CKPS0 = 1; // 1:2 prescaler (400ns/tick to match SERVO_PULSE)
    T3CONbits.T3CKPS1 = 0;
    T3CONbits.RD16 = 1;    // Enable 16-bit Read/Write mode for atomic TMR3 access
    TMR3H = 0;
    TMR3L = 0;
    PIR2bits.TMR3IF = 0;
    PIE2bits.TMR3IE = 1;
    IPR2bits.TMR3IP = 1; // High priority for exact timing
    T3CONbits.TMR3ON = 1;
}

void servo_set_angle(uint8_t servo_id, uint16_t angle_deg) {
    if (angle_deg > 180) angle_deg = 180;
    uint16_t pulse = SERVO_PULSE_MIN
        + ((uint32_t)(SERVO_PULSE_MAX - SERVO_PULSE_MIN) * angle_deg / 180);
    if (servo_id == 1) {
        servo1_pulse = pulse;
    } else {
        servo2_pulse = pulse;
    }
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
        CCP2CONbits.CCP2M = 0b0101; // Rule constraint
        CCP2CONbits.CCP2M = 0b1010; // Compare software interrupt
        CCPR2 = servo1_pulse + 1; // Compensate for ISR vectoring latency
        phase = 1;
    } else {
        LATCbits.LATC1 = 0;
        CCP2CONbits.CCP2M = 0b0111; // Rule constraint
        CCP2CONbits.CCP2M = 0b1011; // Special event trigger
        CCPR2 = SERVO_FRAME_TICKS;
        phase = 0;
    }
}

void servo_timer3_isr(void) {
    static uint8_t phase = 0;
    uint16_t ticks;
    
    if (phase == 0) {
        LATCbits.LATC0 = 1;
        ticks = servo2_pulse;
        phase = 1;
    } else {
        LATCbits.LATC0 = 0;
        ticks = SERVO_FRAME_TICKS - servo2_pulse;
        phase = 0;
    }
    
    uint16_t reload = -ticks + 24; // Pure 16-bit math, no 32-bit standard library helper calls
    TMR3H = (uint8_t)(reload >> 8);
    TMR3L = (uint8_t)(reload & 0xFF);
}

