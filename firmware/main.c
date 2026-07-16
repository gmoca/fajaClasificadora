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

void debug_print(const char *msg) {
    uart_send_str(msg); // <-- HABILITADO PARA PRUEBAS EN FÍSICO
}

void i2c_scan(void) {
    uint8_t found = 0;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        if (i2c_probe(addr)) {
            found = 1;
            debug_print("I2C Device: 0x");
            char hex[5];
            hex[0] = "0123456789ABCDEF"[addr >> 4];
            hex[1] = "0123456789ABCDEF"[addr & 0x0F];
            hex[2] = '\n';
            hex[3] = '\0';
            debug_print(hex);
        }
    }
    if (!found) {
        debug_print("I2C BUS EMPTY\n");
    }
}

void main(void) {
    // 1. Core initializations (must be first to enable UART)
    system_init();
    gpio_init();
    uart_init();

    // 2. Start printing diagnostics
    debug_print("BOOTING...\n");
    debug_print("[OK] system_init\n");
    debug_print("[OK] gpio_init\n");
    debug_print("[OK] uart_init\n");

    // 3. Calibration Init
    debug_print("calibration_init...\n");
    calibration_init();
    debug_print("[OK] calibration_init\n");

    // 4. I2C Init
    debug_print("i2c_init...\n");
    i2c_init();
    debug_print("[OK] i2c_init\n");

    // Print I2C Configuration details
    debug_print("I2C Configuration\n");
    debug_print("SDA = RB0\n");
    debug_print("SCL = RB1\n");
    debug_print("Fosc = 20 MHz\n");
    debug_print("I2C = 100 kHz\n");
    debug_print("SSPADD = 49\n");

    // Run I2C scanner
    i2c_scan(); // <-- HABILITADO PARA PRUEBAS EN FÍSICO

    // 5. LCD Init
    debug_print("lcd_init...\n");
    if (!i2c_probe(LCD_ADDR)) {
        debug_print("[ERROR] LCD I2C ACK failed\n");
    }
    lcd_init();
    debug_print("[OK] lcd_init\n");

    // 6. TCS34725 Init
    debug_print("tcs34725_init...\n");
    if (tcs34725_init()) {
        debug_print("[OK] tcs34725_init\n");
        lcd_print("TSC OK");
    } else {
        debug_print("[ERROR] TCS34725 not detected\n");
        lcd_print("NO SENSOR");
    }
    lcd_set_cursor(0, 1);
    lcd_print("PryMicroFaja v1");

    // 7. PWM Init
    debug_print("pwm_init...\n");
    pwm_hbridge_init();
    debug_print("[OK] pwm_init\n");

    // 8. Servo Init
    debug_print("servo_init...\n");
    servo_init();
    debug_print("[OK] servo_init\n");

    // 9. Encoder Init
    debug_print("encoder_init...\n");
    encoder_init();
    debug_print("[OK] encoder_init\n");

    // 10. BT Protocol Init
    debug_print("bt_protocol_init...\n");
    bt_protocol_init();
    debug_print("[OK] bt_protocol_init\n");

    // 11. State Machine Init
    debug_print("state_machine_init...\n");
    state_machine_init();
    debug_print("[OK] state_machine_init\n");

    debug_print("SYSTEM_READY\n");

    while (1) {
        state_machine_run();
    }
}
