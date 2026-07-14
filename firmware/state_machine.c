#include "state_machine.h"
#include "system.h"
#include "gpio.h"
#include "pwm.h"
#include "encoder.h"
#include "tcs34725.h"
#include "uart.h"
#include "lcd.h"
#include "servo.h"
#include "bt_protocol.h"
#include "calibration.h"
#include "anti_jam.h"
#include <string.h>

static void u16_to_str(char *buf, uint16_t val) {
    char tmp[6];
    uint8_t i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val > 0 && i < 5) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (uint8_t j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    buf[i] = '\0';
}

static void u32_to_str(char *buf, uint32_t val) {
    char tmp[11];
    uint8_t i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val > 0 && i < 10) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (uint8_t j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    buf[i] = '\0';
}

static state_t state = ST_IDLE;
static uint8_t motor_speed = 0;
static uint8_t auto_mode = 1;
static uint16_t distance_to_servo_mm = 200;  // mm from sensor to servo
static uint16_t min_spacing_pulses = 100;
static uint16_t dwell_time_ms = 500;
static uint32_t last_detect_tick = 0;
static uint32_t test_motor_watchdog = 0;

void state_machine_set_mode(uint8_t mode) { auto_mode = mode; }
void state_machine_set_spacing(uint16_t space) { min_spacing_pulses = space; }
void state_machine_set_dwell(uint16_t dwell) { dwell_time_ms = dwell; }

void state_machine_init(void) {
    state = ST_IDLE;
    anti_jam_init();
    lcd_clear();
    lcd_print("Sistema OK");
    lcd_set_cursor(0, 1);
    lcd_print("Presione START");
}

static volatile uint8_t estop_pending = 0;

void state_machine_estop(void) {
    state = ST_ERROR;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    estop_pending = 1;
}

void state_machine_start(void) {
    if (state != ST_IDLE) return;
    state = ST_RUNNING;
    pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
    gpio_hbridge_dir(HB_FWD);
    uart_send_str("STATE:run\n");
    lcd_clear();
    lcd_print("RUNNING");
}

void state_machine_test_enter(void) {
    if (state != ST_IDLE) return;
    state = ST_TEST;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    uart_send_str("STATE:test\n");
    lcd_clear();
    lcd_print("MODO TEST");
}

void state_machine_test_exit(void) {
    if (state != ST_TEST) return;
    state = ST_IDLE;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    uart_send_str("STATE:idle\n");
    lcd_clear();
    lcd_print("IDLE");
}

void state_machine_test_motor(void) {
    if (state != ST_TEST) return;
    test_motor_watchdog = system_ticks;
    pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
    gpio_hbridge_dir(HB_FWD);
}

state_t state_machine_get(void) {
    return state;
}

int8_t color_match_index(color_rgbc_t *c) {
    color_config_t colors[3];
    uint8_t num_colors = calibration_load_all(colors, 3);
    for (uint8_t i = 0; i < num_colors; i++) {
        if (c->r >= colors[i].r_min && c->r <= colors[i].r_max &&
            c->g >= colors[i].g_min && c->g <= colors[i].g_max &&
            c->b >= colors[i].b_min && c->b <= colors[i].b_max) {
            return (int8_t)i;
        }
    }
    return -1;
}

void state_machine_run(void) {
    static uint32_t last_telemetry = 0;
    static uint16_t lcd_counter = 0;

    INTCONbits.GIEL = 0;
    uint32_t now = system_ticks;
    INTCONbits.GIEL = 1;

    if (estop_pending) {
        estop_pending = 0;
        uart_send_str("STATE:err\n");
        lcd_clear();
        lcd_print("!EMERGENCY STOP!");
    }

    if (gpio_button_pressed(BTN_MODE)) {
        if (state == ST_IDLE && auto_mode) {
            state = ST_RUNNING;
            pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
            gpio_hbridge_dir(HB_FWD);
            uart_send_str("STATE:run\n");
            lcd_clear();
            lcd_print("RUNNING");
        } else if (state == ST_RUNNING) {
            state = ST_IDLE;
            pwm_hbridge_set_duty(0);
            gpio_hbridge_dir(HB_STOP);
            uart_send_str("STATE:idle\n");
            lcd_clear();
            lcd_print("IDLE");
        }
    }
    
    anti_jam_check();

    switch (state) {
        case ST_IDLE:
            if (lcd_counter++ % 500 == 0) {
                lcd_set_cursor(0, 1);
                lcd_print("BT OK        ");
            }
            break;

        case ST_RUNNING:
            if (now % 100 == 0) {
                color_rgbc_t color;
                tcs34725_get_raw(&color);
                int8_t idx = color_match_index(&color);
                if (idx >= 0) {
                    state = ST_SORTING;
                    char buf[32];
                    strcpy(buf, "DETECT:");
                    char idx_str[4];
                    u16_to_str(idx_str, (uint16_t)idx);
                    strcat(buf, idx_str);
                    strcat(buf, "\n");
                    uart_send_str(buf);
                    
                    lcd_set_cursor(0, 1);
                    lcd_print("Detectado: ");
                    lcd_print(idx_str);
                }
            }
            if (lcd_counter++ % 50 == 0) {
                lcd_set_cursor(0, 1);
                lcd_print("Vel: ");
                char v_str[8];
                u16_to_str(v_str, encoder_get_speed_mm_s());
                lcd_print(v_str);
                lcd_print(" mm/s    ");
            }
            break;

        case ST_SORTING:
            {
                uint16_t speed = encoder_get_speed_mm_s();
                if (speed > 0) {
                    uint32_t transit_ms = (uint32_t)distance_to_servo_mm * 1000 / speed;
                    uint32_t timeout_wait = system_ticks + transit_ms;
                    // wait transit_ms
                    while (system_ticks < timeout_wait) {
                        anti_jam_check();
                        if (anti_jam_is_jammed()) break;
                    }
                    
                    if (!anti_jam_is_jammed()) {
                        servo_set_angle(1, 90);
                        uint32_t timeout = system_ticks + dwell_time_ms;
                        while (system_ticks < timeout) {}
                        servo_set_angle(1, 0);
                    }
                }
                state = ST_RUNNING;
                uart_send_str("DONE\n");
            }
            break;

        case ST_ERROR:
            if (gpio_button_pressed(BTN_MODE)) {
                state = ST_IDLE;
                uart_send_str("STATE:idle\n");
                lcd_clear();
                lcd_print("IDLE - OK");
            }
            break;
            
        case ST_TEST:
            if (system_ticks - test_motor_watchdog > 2000) {
                pwm_hbridge_set_duty(0);
                gpio_hbridge_dir(HB_STOP);
            }
            if (lcd_counter++ % 500 == 0) {
                lcd_set_cursor(0, 1);
                lcd_print("Test Activo  ");
            }
            break;
    }

    if (now - last_telemetry >= 500) {
        last_telemetry = now;
        
        uart_send_str("STATE:");
        uart_send_str(state == ST_IDLE ? "idle" :
                      state == ST_RUNNING ? "run" :
                      state == ST_SORTING ? "sort" : 
                      state == ST_TEST ? "test" : "err");
        
        uart_send_str(" SPEED:");
        char num_buf[16];
        u16_to_str(num_buf, encoder_get_speed_mm_s());
        uart_send_str(num_buf);
        
        uart_send_str(" PULSES:");
        u32_to_str(num_buf, encoder_get_pulses());
        uart_send_str(num_buf);
        uart_send_str("\n");
    }

    bt_protocol_process();  
}
