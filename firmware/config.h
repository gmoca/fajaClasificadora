#ifndef CONFIG_H
#define CONFIG_H

#pragma config FOSC = HS
#pragma config XINST = OFF
#pragma config WDT = OFF
#pragma config CPUDIV = OSC1_PLL2
#pragma config PLLDIV = 1
#pragma config USBDIV = 1
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config BOR = ON
#pragma config PWRT = ON
#pragma config MCLRE = OFF
#pragma config PBADEN = OFF
#pragma config CCP2MX = ON
#pragma config STVREN = ON
#pragma config LVP = OFF
#pragma config DEBUG = OFF
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF
#pragma config CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF
#pragma config EBTRB = OFF

#include <xc.h>

#define _XTAL_FREQ  20000000UL
#define FOSC_HZ     20000000UL
#define FCYC_HZ     (FOSC_HZ / 4)

#define SPBRG_VAL   42  // 115200 baud at 20MHz (exact integer calculation)

#define TMR0_RELOAD 64911

#endif
