#include "config.h"
#include "system.h"
#include "gpio.h"
#include "uart.h"
#include "i2c.h"
#include "lcd.h"
#include "tcs34725.h"
#include "pwm.h"
#include "servo.h"
#include "encoder.h"
#include "state_machine.h"
#include "bt_protocol.h"
#include "calibration.h"

void main(void) {
    system_init();
    gpio_init();
    uart_init();
    uart_send_str("BOOTING...\n");
    
    calibration_init();
    i2c_init();

    uart_send_str("Scanning I2C...\n");
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        if (i2c_probe(addr)) {
            uart_send_str("Found device at 0x");
            char hex[5];
            hex[0] = "0123456789ABCDEF"[addr >> 4];
            hex[1] = "0123456789ABCDEF"[addr & 0x0F];
            hex[2] = '\n';
            hex[3] = '\0';
            uart_send_str(hex);
        }
    }

    lcd_init();

    if (tcs34725_init()) {
        lcd_print("TSC OK");
    } else {
        lcd_print("NO SENSOR");
    }
    lcd_set_cursor(0, 1);
    lcd_print("PryMicroFaja v1");

    pwm_hbridge_init();
    servo_init();
    encoder_init();
    bt_protocol_init();
    state_machine_init();

    uart_send_str("SYSTEM_READY\n");

    while (1) {
        state_machine_run();
    }
}
