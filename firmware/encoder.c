#include "encoder.h"
#include "config.h"

#define PULSES_PER_REV  20
#define ROLLER_MM       100.0f
#define SPEED_WINDOW_MS 500

static volatile uint32_t total_pulses = 0;
static volatile uint16_t pulse_window = 0;
static volatile uint16_t speed_mm_s = 0;

void encoder_init(void) {
    TRISBbits.TRISB2 = 1;
    INTCON3bits.INT2IE = 1;
    INTCON3bits.INT2IP = 0;
    INTCON2bits.INTEDG2 = 1;
    INTCON3bits.INT2IF = 0;
}

void encoder_reset(void) {
    INTCONbits.PEIE = 0;
    total_pulses = 0;
    pulse_window = 0;
    speed_mm_s = 0;
    INTCONbits.PEIE = 1;
}

void encoder_isr_handler(void) {
    total_pulses++;
    pulse_window++;
}

void encoder_tick_handler(void) {
    static uint16_t counter = 0;
    counter++;
    if (counter >= SPEED_WINDOW_MS) {
        counter = 0;
        uint32_t pps = (uint32_t)pulse_window * (1000 / SPEED_WINDOW_MS);
        speed_mm_s = (uint16_t)(pps * ROLLER_MM / PULSES_PER_REV);
        pulse_window = 0;
    }
}

uint32_t encoder_get_pulses(void) {
    uint32_t p;
    INTCONbits.PEIE = 0;
    p = total_pulses;
    INTCONbits.PEIE = 1;
    return p;
}

uint16_t encoder_get_speed_mm_s(void) {
    return speed_mm_s;
}
