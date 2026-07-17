#include "pwm.h"
#include "config.h"

void pwm_hbridge_init(void) {
    CCP1CON = 0x0C;  // Single output PWM, P1A (RC2) active-high
    CCPR1L = 0;
    TRISCbits.TRISC2 = 0;
}

void pwm_hbridge_set_duty(uint8_t duty) {
    // TEMPORAL: Apagamos el módulo PWM y forzamos RC2 como salida digital simple (HIGH/LOW)
    // para descartar si el problema es de cableado físico o del oscilador de PWM.
    CCP1CON = 0;  // Desactiva el módulo CCP1 (PWM) para que el pin actúe como GPIO normal
    TRISCbits.TRISC2 = 0; // Asegura que RC2 es salida
    
    if (duty > 0) {
        LATCbits.LATC2 = 1;  // Fuerza 5V en RC2
    } else {
        LATCbits.LATC2 = 0;  // Fuerza 0V en RC2
    }
}
