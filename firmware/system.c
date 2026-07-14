#include "system.h"
#include "gpio.h"
#include "encoder.h"
#include "servo.h"
#include "uart.h"
#include "state_machine.h"

volatile uint32_t system_ticks = 0;

void system_init(void) {
    RCONbits.IPEN = 1;

    INTCON3bits.INT2IE = 1;
    INTCON3bits.INT2IP = 0;
    INTCON2bits.INTEDG2 = 1;
    INTCON3bits.INT2IF = 0;

    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS = 0b001;
    TMR0H = (TMR0_RELOAD >> 8) & 0xFF;
    TMR0L = TMR0_RELOAD & 0xFF;
    T0CONbits.TMR0ON = 1;
    INTCONbits.TMR0IE = 1;
    INTCON2bits.TMR0IP = 0;  // Set TMR0 priority to low to match isr_low handler
    INTCONbits.TMR0IF = 0;

    T1CONbits.TMR1CS = 0;
    T1CONbits.T1CKPS = 0b01;
    T1CONbits.TMR1ON = 1;
    PIR1bits.TMR1IF = 0;

    T3CONbits.T3CCP2 = 0;

    T2CONbits.T2CKPS = 0b00;
    PR2 = 249;
    T2CONbits.TMR2ON = 1;

    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

void __interrupt(high_priority) isr_high(void) {
    // INT0 E-stop moved to polled RB3 input to prevent I2C SDA conflicts on RB0
}

void __interrupt(low_priority) isr_low(void) {
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        TMR0H = (TMR0_RELOAD >> 8) & 0xFF;
        TMR0L = TMR0_RELOAD & 0xFF;
        system_ticks++;
        encoder_tick_handler();
        gpio_scan_buttons();
        
        // Polled E-stop on RB3 (active low)
        if (PORTBbits.RB3 == 0) {
            state_machine_estop();
        }
    }
    if (INTCON3bits.INT2IF) {
        INTCON3bits.INT2IF = 0;
        encoder_isr_handler();
    }
    if (PIR1bits.RCIF || (PIR1bits.TXIF && PIE1bits.TXIE)) {
        uart_isr_handler();
    }
    if (PIR2bits.CCP2IF) {
        PIR2bits.CCP2IF = 0;
        servo_ccp2_isr();
    }
    if (PIR2bits.TMR3IF) {
        PIR2bits.TMR3IF = 0;
        servo_timer3_isr();
    }
}
