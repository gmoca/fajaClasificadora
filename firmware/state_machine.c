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

static void safe_delay_ms(uint16_t ms) {
    while (ms--) {
        __delay_ms(1);
    }
}

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
static const char *estop_reason = "UNKNOWN";
volatile uint8_t estop_triggered_by_button = 0;

void state_machine_estop_reason(const char *reason) {
    state = ST_ERROR;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    estop_reason = reason;
    estop_pending = 1;
}

void state_machine_estop(void) {
    state_machine_estop_reason("BUTTON_OR_BT");
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

static uint16_t menu_s1_home = 90;
static uint16_t menu_s1_defl = 0;
static uint16_t menu_s1_dwell = 500;
static uint16_t menu_s2_home = 90;
static uint16_t menu_s2_defl = 0;
static uint16_t menu_s2_dwell = 500;
static uint16_t menu_ppr = 20;

static uint8_t menu_index = 0;
static uint8_t menu_active = 0;

static void load_menu_values(void) {
    menu_s1_home = calibration_read_word(EEPROM_ADDR_SERVO1_HOME);
    menu_s1_defl = calibration_read_word(EEPROM_ADDR_SERVO1_DEFL);
    menu_s1_dwell = calibration_read_word(EEPROM_ADDR_SERVO1_DWELL);
    menu_s2_home = calibration_read_word(EEPROM_ADDR_SERVO2_HOME);
    menu_s2_defl = calibration_read_word(EEPROM_ADDR_SERVO2_DEFL);
    menu_s2_dwell = calibration_read_word(EEPROM_ADDR_SERVO2_DWELL);
    menu_ppr = calibration_read_word(EEPROM_ADDR_ENC_PPR);
}

static void adjust_menu_value(int8_t dir) {
    switch (menu_index) {
        case 0:
            if (dir > 0 && menu_s1_home < 180) menu_s1_home++;
            if (dir < 0 && menu_s1_home > 0) menu_s1_home--;
            servo_set_angle(1, menu_s1_home);
            break;
        case 1:
            if (dir > 0 && menu_s1_defl < 180) menu_s1_defl++;
            if (dir < 0 && menu_s1_defl > 0) menu_s1_defl--;
            servo_set_angle(1, menu_s1_defl);
            break;
        case 2:
            if (dir > 0 && menu_s1_dwell < 5000) menu_s1_dwell += 50;
            if (dir < 0 && menu_s1_dwell > 50) menu_s1_dwell -= 50;
            break;
        case 3:
            if (dir > 0 && menu_s2_home < 180) menu_s2_home++;
            if (dir < 0 && menu_s2_home > 0) menu_s2_home--;
            servo_set_angle(2, menu_s2_home);
            break;
        case 4:
            if (dir > 0 && menu_s2_defl < 180) menu_s2_defl++;
            if (dir < 0 && menu_s2_defl > 0) menu_s2_defl--;
            servo_set_angle(2, menu_s2_defl);
            break;
        case 5:
            if (dir > 0 && menu_s2_dwell < 5000) menu_s2_dwell += 50;
            if (dir < 0 && menu_s2_dwell > 50) menu_s2_dwell -= 50;
            break;
        case 6:
            if (dir > 0 && menu_ppr < 1000) menu_ppr++;
            if (dir < 0 && menu_ppr > 1) menu_ppr--;
            break;
        case 7:
            // Exit option, no value to adjust
            break;
    }
}

static void save_current_menu_value(void) {
    switch (menu_index) {
        case 0: calibration_save_servo_home(1, menu_s1_home); break;
        case 1: calibration_save_servo_deflect(1, menu_s1_defl); break;
        case 2: calibration_save_servo_dwell(1, menu_s1_dwell); break;
        case 3: calibration_save_servo_home(2, menu_s2_home); break;
        case 4: calibration_save_servo_deflect(2, menu_s2_defl); break;
        case 5: calibration_save_servo_dwell(2, menu_s2_dwell); break;
        case 6: calibration_save_ppr(menu_ppr); break;
        case 7: // Exit option, no value to save
            break;
    }
}

static void display_menu(void) {
    lcd_set_cursor(0, 0);
    lcd_print("CALIBRAR LOCAL  ");
    lcd_set_cursor(0, 1);
    
    char val_str[16];
    switch (menu_index) {
        case 0:
            lcd_print("S1 Home: ");
            u16_to_str(val_str, menu_s1_home);
            break;
        case 1:
            lcd_print("S1 Defl: ");
            u16_to_str(val_str, menu_s1_defl);
            break;
        case 2:
            lcd_print("S1 Dwell: ");
            u16_to_str(val_str, menu_s1_dwell);
            break;
        case 3:
            lcd_print("S2 Home: ");
            u16_to_str(val_str, menu_s2_home);
            break;
        case 4:
            lcd_print("S2 Defl: ");
            u16_to_str(val_str, menu_s2_defl);
            break;
        case 5:
            lcd_print("S2 Dwell: ");
            u16_to_str(val_str, menu_s2_dwell);
            break;
        case 6:
            lcd_print("Enc PPR: ");
            u16_to_str(val_str, menu_ppr);
            break;
        case 7:
            lcd_print("Mant. p/ ");
            strcpy(val_str, "SALIR");
            break;
    }
    lcd_print(val_str);
    lcd_print("      ");
}

void state_machine_test_enter(void) {
    if (state != ST_IDLE) return;
    state = ST_TEST;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    last_bt_activity = system_ticks;
    menu_active = 0;
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
    static uint32_t last_lcd_update = 0;

    INTCONbits.GIEL = 0;
    uint32_t now = system_ticks;
    INTCONbits.GIEL = 1;

    encoder_update_speed();

    if (estop_triggered_by_button) {
        estop_triggered_by_button = 0;
        state_machine_estop_reason("E_STOP_BUTTON");
    }

    if (estop_pending) {
        estop_pending = 0;
        uart_send_str("STATE:err\n");
        uart_send_str("JAM:");
        uart_send_str(estop_reason);
        uart_send_str("\n");
        lcd_clear();
        lcd_print("!EMERGENCY STOP!");
    }

    static uint32_t idle_press_start = 0;
    static uint8_t idle_was_pressed = 0;
    static uint8_t idle_long_pressed = 0;
    static uint8_t ignore_mode_until_release = 0;
    static uint8_t first_run = 1;

    if (first_run) {
        first_run = 0;
        if (gpio_button_state(BTN_MODE)) {
            ignore_mode_until_release = 1;
        }
    }

    if (state == ST_IDLE) {
        if (ignore_mode_until_release) {
            if (!gpio_button_state(BTN_MODE)) {
                ignore_mode_until_release = 0;
            }
        } else if (gpio_button_state(BTN_MODE)) {
            if (!idle_was_pressed) {
                idle_press_start = now;
                idle_was_pressed = 1;
                idle_long_pressed = 0;
            } else if (!idle_long_pressed && (now - idle_press_start > 2000)) {
                // Long press: enter ST_TEST (calibration menu)
                state = ST_TEST;
                menu_active = 1;
                menu_index = 0;
                load_menu_values();
                lcd_clear();
                lcd_print("MENU CALIB");
                safe_delay_ms(800);
                lcd_clear();
                idle_long_pressed = 1;
                gpio_button_pressed(BTN_MODE); // Consume the edge
            }
        } else {
            if (idle_was_pressed) {
                if (!idle_long_pressed && (now - idle_press_start > 50)) {
                    // Short press: start running
                    if (auto_mode) {
                        state = ST_RUNNING;
                        pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
                        gpio_hbridge_dir(HB_FWD);
                        uart_send_str("STATE:run\n");
                        lcd_clear();
                        lcd_print("RUNNING");
                        gpio_button_pressed(BTN_MODE); // Consume/clear edge to prevent bounce-back
                    }
                }
                idle_was_pressed = 0;
            }
        }
    } else if (gpio_button_pressed(BTN_MODE)) {
        if (state == ST_RUNNING) {
            state = ST_IDLE;
            pwm_hbridge_set_duty(0);
            gpio_hbridge_dir(HB_STOP);
            uart_send_str("STATE:idle\n");
            lcd_clear();
            lcd_print("IDLE");
            if (gpio_button_state(BTN_MODE)) {
                ignore_mode_until_release = 1;
            }
        } else if (state == ST_ERROR) {
            state = ST_IDLE;
            uart_send_str("STATE:idle\n");
            lcd_clear();
            lcd_print("IDLE - OK");
            if (gpio_button_state(BTN_MODE)) {
                ignore_mode_until_release = 1;
            }
        }
    }
    
    anti_jam_check();

    switch (state) {
        case ST_IDLE:
            if (now - last_lcd_update >= 500) {
                last_lcd_update = now;
                lcd_set_cursor(0, 1);
                lcd_print("BT OK           ");
            }
            break;
 
        case ST_RUNNING:
            if (now % 100 == 0 && tcs34725_is_present()) {
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
            if (now - last_lcd_update >= 500) {
                last_lcd_update = now;
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
                static uint8_t sorting_phase = 0;
                static uint32_t sorting_timeout = 0;
                
                if (sorting_phase == 0) {
                    uint16_t speed = encoder_get_speed_mm_s();
                    if (speed > 0) {
                        uint32_t transit_ms = (uint32_t)distance_to_servo_mm * 1000 / speed;
                        sorting_timeout = system_ticks + transit_ms;
                        sorting_phase = 1;
                    } else {
                        // Speed is 0, abort sorting
                        state = ST_RUNNING;
                        sorting_phase = 0;
                    }
                }
                
                if (sorting_phase == 1) {
                    if (system_ticks >= sorting_timeout) {
                        if (!anti_jam_is_jammed() && state != ST_ERROR) {
                            uint16_t defl = calibration_read_word(EEPROM_ADDR_SERVO1_DEFL);
                            servo_set_angle(1, defl);
                            sorting_timeout = system_ticks + dwell_time_ms;
                            sorting_phase = 2;
                        } else {
                            state = ST_RUNNING;
                            sorting_phase = 0;
                        }
                    }
                }
                
                if (sorting_phase == 2) {
                    if (system_ticks >= sorting_timeout) {
                        uint16_t home = calibration_read_word(EEPROM_ADDR_SERVO1_HOME);
                        servo_set_angle(1, home);
                        state = ST_RUNNING;
                        sorting_phase = 0;
                        uart_send_str("DONE\n");
                    }
                }
            }
            break;

        case ST_ERROR:
            break;
            
        case ST_TEST:
            if (system_ticks - test_motor_watchdog > 2000) {
                pwm_hbridge_set_duty(0);
                gpio_hbridge_dir(HB_STOP);
            }
            
            if (system_ticks - last_bt_activity > 5000 || last_bt_activity == 0) {
                if (!menu_active) {
                    menu_active = 1;
                    menu_index = 0;
                    load_menu_values();
                    lcd_clear();
                }
                
                // Debounced long-press and short-press logic
                static uint32_t mode_press_start = 0;
                static uint8_t mode_was_pressed = 0;
                static uint8_t mode_long_pressed = 0;
                
                if (gpio_button_state(BTN_MODE)) {
                    if (!mode_was_pressed) {
                        mode_press_start = now;
                        mode_was_pressed = 1;
                        mode_long_pressed = 0;
                    } else if (!mode_long_pressed && (now - mode_press_start > 1500)) {
                        if (menu_index == 7) {
                            state = ST_IDLE;
                            menu_active = 0;
                            lcd_clear();
                            lcd_print("MENU SALIDA");
                            safe_delay_ms(800);
                            lcd_clear();
                            lcd_print("IDLE");
                            if (gpio_button_state(BTN_MODE)) {
                                ignore_mode_until_release = 1;
                            }
                        } else {
                            save_current_menu_value();
                            lcd_clear();
                            lcd_print("GUARDADO!");
                            safe_delay_ms(800);
                            lcd_clear();
                        }
                        mode_long_pressed = 1;
                    }
                } else {
                    if (mode_was_pressed) {
                        if (!mode_long_pressed && (now - mode_press_start > 50)) {
                            menu_index = (menu_index + 1) % 8;
                            lcd_clear();
                        }
                        mode_was_pressed = 0;
                    }
                }
                
                if (gpio_button_pressed(BTN_UP)) {
                    adjust_menu_value(1);
                }
                if (gpio_button_pressed(BTN_DOWN)) {
                    adjust_menu_value(-1);
                }
                
                if (now - last_lcd_update >= 200) {
                    last_lcd_update = now;
                    display_menu();
                }
            } else {
                menu_active = 0;
                if (now - last_lcd_update >= 500) {
                    last_lcd_update = now;
                    lcd_set_cursor(0, 1);
                    lcd_print("Test Activo  ");
                }
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
