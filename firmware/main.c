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
    calibration_init();
    uart_init();
    i2c_init();
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
