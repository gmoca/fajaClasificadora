#include "gpio.h"
#include "config.h"

#define DEBOUNCE_MS 5

static uint8_t btn_state[3];
static uint8_t btn_edge[3];
static uint8_t btn_counter[3];

static const uint8_t btn_pins[3] = { 2, 5, 6 };

void gpio_init(void) {
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    gpio_hbridge_dir(HB_STOP);

    TRISDbits.TRISD2 = 1;
    TRISDbits.TRISD5 = 1;
    TRISDbits.TRISD6 = 1;

    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 1;

    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB2 = 1;

    gpio_breakbeam_emitter(1);

    for (int i = 0; i < 3; i++) {
        btn_state[i] = 1;
        btn_edge[i] = 0;
        btn_counter[i] = 0;
    }
}

void gpio_scan_buttons(void) {
    for (int i = 0; i < 3; i++) {
        uint8_t raw = (PORTD >> btn_pins[i]) & 1;
        if (raw != btn_state[i]) {
            btn_counter[i]++;
            if (btn_counter[i] >= DEBOUNCE_MS) {
                btn_state[i] = raw;
                btn_counter[i] = 0;
                if (raw == 0)
                    btn_edge[i] = 1;
            }
        } else {
            btn_counter[i] = 0;
        }
    }
}

uint8_t gpio_button_pressed(button_t btn) {
    uint8_t ret = btn_edge[btn];
    btn_edge[btn] = 0;
    return ret;
}

uint8_t gpio_button_state(button_t btn) {
    return (btn_state[btn] == 0);
}

void gpio_hbridge_dir(hbridge_dir_t dir) {
    switch (dir) {
        case HB_STOP: LATDbits.LATD0 = 0; LATDbits.LATD1 = 0; break;
        case HB_FWD:  LATDbits.LATD0 = 1; LATDbits.LATD1 = 0; break;
        case HB_REV:  LATDbits.LATD0 = 0; LATDbits.LATD1 = 1; break;
    }
}

void gpio_breakbeam_emitter(uint8_t on) {
    LATDbits.LATD3 = on ? 1 : 0;
}

uint8_t gpio_breakbeam_read(uint8_t station) {
    if (station == 1)
        return (PORTD >> 4) & 1;
    return (PORTD >> 7) & 1;
}
