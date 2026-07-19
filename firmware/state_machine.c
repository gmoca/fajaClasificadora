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
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

static void adc_init(void) {
    ADCON1 = 0x0E;          // AN0 (RA0) analog, others digital
    TRISAbits.TRISA0 = 1;   // RA0 input
    ADCON0 = 0x01;          // Select AN0, ADC ON
    ADCON2 = 0xA5;          // Right justified, 8 TAD, FOSC/16
}

static uint16_t adc_read_an0(void) {
    ADCON0bits.GO = 1;      // Start conversion
    while (ADCON0bits.GO);  // Wait for it to finish
    return ((uint16_t)ADRESH << 8) | ADRESL;
}

static uint8_t last_adc_val = 0;

static state_t state = ST_IDLE;
static uint8_t motor_speed = 0;
static uint8_t auto_mode = 1;
static uint16_t distance_s1_mm = 200;  // mm from sensor to servo 1
static uint16_t distance_s2_mm = 250;  // mm from sensor to servo 2
static uint16_t min_spacing_pulses = 100;
static uint16_t dwell_time_ms = 500;
static uint32_t last_detect_tick = 0;
static uint32_t test_motor_watchdog = 0;
static uint8_t sorting_servo_id = 1;
static uint8_t sorting_phase = 0;
static uint32_t last_detect_pulses = 0;
static uint8_t first_detection = 1;

void state_machine_set_mode(uint8_t mode) { auto_mode = mode; }
void state_machine_set_spacing(uint16_t space) { min_spacing_pulses = space; }
void state_machine_set_dwell(uint16_t dwell) { dwell_time_ms = dwell; }
void state_machine_set_speed(uint8_t speed) {
    motor_speed = speed;
    if (state == ST_RUNNING || state == ST_SORTING || state == ST_TEST) {
        pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
    }
    last_adc_val = (uint8_t)(adc_read_an0() >> 2);
}
void state_machine_set_dist(uint8_t sid, uint16_t dist) {
    if (sid == 1) distance_s1_mm = dist;
    else if (sid == 2) distance_s2_mm = dist;
}

void state_machine_init(void) {
    state = ST_IDLE;
    anti_jam_init();
    adc_init();
    last_adc_val = (uint8_t)(adc_read_an0() >> 2);
    distance_s1_mm = calibration_read_word(EEPROM_ADDR_SERVO1_DIST);
    if (distance_s1_mm == 0 || distance_s1_mm > 2000) distance_s1_mm = 200;
    distance_s2_mm = calibration_read_word(EEPROM_ADDR_SERVO2_DIST);
    if (distance_s2_mm == 0 || distance_s2_mm > 2000) distance_s2_mm = 250;
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
    first_detection = 1;
    pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
    gpio_hbridge_dir(HB_FWD);
    uart_send_str("STATE:run\n");
    lcd_clear();
    lcd_print("RUNNING");
}

static uint16_t menu_s1_home = 90;
static uint16_t menu_s1_defl = 0;
static uint16_t menu_s1_dwell = 500;
static uint16_t menu_s1_dist = 200;
static uint16_t menu_s2_home = 90;
static uint16_t menu_s2_defl = 0;
static uint16_t menu_s2_dwell = 500;
static uint16_t menu_s2_dist = 250;
static uint16_t menu_ppr = 20;

static uint8_t menu_index = 0;
static uint8_t menu_active = 0;

static uint32_t idle_press_start = 0;
static uint8_t idle_was_pressed = 0;
static uint8_t idle_long_pressed = 0;
static uint8_t ignore_mode_until_release = 0;
static uint8_t first_run = 1;

static void load_menu_values(void) {
    menu_s1_home = calibration_read_word(EEPROM_ADDR_SERVO1_HOME);
    if (menu_s1_home > 180) menu_s1_home = 90;
    menu_s1_defl = calibration_read_word(EEPROM_ADDR_SERVO1_DEFL);
    if (menu_s1_defl > 180) menu_s1_defl = 0;
    menu_s1_dwell = calibration_read_word(EEPROM_ADDR_SERVO1_DWELL);
    if (menu_s1_dwell > 5000) menu_s1_dwell = 500;
    menu_s1_dist = calibration_read_word(EEPROM_ADDR_SERVO1_DIST);
    if (menu_s1_dist == 0 || menu_s1_dist > 2000) menu_s1_dist = 200;
    
    menu_s2_home = calibration_read_word(EEPROM_ADDR_SERVO2_HOME);
    if (menu_s2_home > 180) menu_s2_home = 90;
    menu_s2_defl = calibration_read_word(EEPROM_ADDR_SERVO2_DEFL);
    if (menu_s2_defl > 180) menu_s2_defl = 0;
    menu_s2_dwell = calibration_read_word(EEPROM_ADDR_SERVO2_DWELL);
    if (menu_s2_dwell > 5000) menu_s2_dwell = 500;
    menu_s2_dist = calibration_read_word(EEPROM_ADDR_SERVO2_DIST);
    if (menu_s2_dist == 0 || menu_s2_dist > 2000) menu_s2_dist = 250;
    
    menu_ppr = calibration_read_word(EEPROM_ADDR_ENC_PPR);
    if (menu_ppr == 0 || menu_ppr > 1000) menu_ppr = 20;
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
            if (dir > 0 && menu_s1_dist < 2000) menu_s1_dist += 5;
            if (dir < 0 && menu_s1_dist > 5) menu_s1_dist -= 5;
            break;
        case 4:
            if (dir > 0 && menu_s2_home < 180) menu_s2_home++;
            if (dir < 0 && menu_s2_home > 0) menu_s2_home--;
            servo_set_angle(2, menu_s2_home);
            break;
        case 5:
            if (dir > 0 && menu_s2_defl < 180) menu_s2_defl++;
            if (dir < 0 && menu_s2_defl > 0) menu_s2_defl--;
            servo_set_angle(2, menu_s2_defl);
            break;
        case 6:
            if (dir > 0 && menu_s2_dwell < 5000) menu_s2_dwell += 50;
            if (dir < 0 && menu_s2_dwell > 50) menu_s2_dwell -= 50;
            break;
        case 7:
            if (dir > 0 && menu_s2_dist < 2000) menu_s2_dist += 5;
            if (dir < 0 && menu_s2_dist > 5) menu_s2_dist -= 5;
            break;
        case 8:
            if (dir > 0 && menu_ppr < 1000) menu_ppr++;
            if (dir < 0 && menu_ppr > 1) menu_ppr--;
            break;
        case 9:
            // Exit option, no value to adjust
            break;
    }
}

static void save_current_menu_value(void) {
    switch (menu_index) {
        case 0: calibration_save_servo_home(1, menu_s1_home); break;
        case 1: calibration_save_servo_deflect(1, menu_s1_defl); break;
        case 2: calibration_save_servo_dwell(1, menu_s1_dwell); break;
        case 3: calibration_save_servo_dist(1, menu_s1_dist); distance_s1_mm = menu_s1_dist; break;
        case 4: calibration_save_servo_home(2, menu_s2_home); break;
        case 5: calibration_save_servo_deflect(2, menu_s2_defl); break;
        case 6: calibration_save_servo_dwell(2, menu_s2_dwell); break;
        case 7: calibration_save_servo_dist(2, menu_s2_dist); distance_s2_mm = menu_s2_dist; break;
        case 8: calibration_save_ppr(menu_ppr); break;
        case 9: // Exit option, no value to save
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
            lcd_print("S1 Dist: ");
            u16_to_str(val_str, menu_s1_dist);
            break;
        case 4:
            lcd_print("S2 Home: ");
            u16_to_str(val_str, menu_s2_home);
            break;
        case 5:
            lcd_print("S2 Defl: ");
            u16_to_str(val_str, menu_s2_defl);
            break;
        case 6:
            lcd_print("S2 Dwell: ");
            u16_to_str(val_str, menu_s2_dwell);
            break;
        case 7:
            lcd_print("S2 Dist: ");
            u16_to_str(val_str, menu_s2_dist);
            break;
        case 8:
            lcd_print("Enc PPR: ");
            u16_to_str(val_str, menu_ppr);
            break;
        case 9:
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

    static uint32_t last_pot_check = 0;
    if (now - last_pot_check >= 100) {
        last_pot_check = now;
        uint16_t adc = adc_read_an0();
        uint8_t pot_speed = (uint8_t)(adc >> 2);
        if (ABS_DIFF(pot_speed, last_adc_val) > 4) {
            last_adc_val = pot_speed;
            state_machine_set_speed(pot_speed);
        }
    }

    if (estop_triggered_by_button) {
        estop_triggered_by_button = 0;
        if (state != ST_ERROR) {
            state_machine_estop_reason("E_STOP_BUTTON");
        }
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



    if (!gpio_button_state(BTN_MODE)) {
        ignore_mode_until_release = 0;
    }

    if (first_run) {
        first_run = 0;
        if (gpio_button_state(BTN_MODE)) {
            ignore_mode_until_release = 1;
        }
    }

    if (state == ST_IDLE) {
        if (ignore_mode_until_release) {
            // Do nothing, waiting for release
        } else if (gpio_button_state(BTN_MODE)) {
            if (!idle_was_pressed) {
                idle_press_start = now;
                idle_was_pressed = 1;
                idle_long_pressed = 0;
            } else if (!idle_long_pressed && (now - idle_press_start > 4000)) {
                // Long press: enter ST_TEST (calibration menu)
                state = ST_TEST;
                menu_active = 1;
                menu_index = 0;
                load_menu_values();
                lcd_clear();
                display_menu();
                idle_long_pressed = 1;
                gpio_button_pressed(BTN_MODE); // Consume the edge
            }
        } else {
            if (idle_was_pressed) {
                if (!idle_long_pressed && (now - idle_press_start > 100)) {
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
            idle_was_pressed = 0;
            idle_long_pressed = 0;
            if (gpio_button_state(BTN_MODE)) {
                ignore_mode_until_release = 1;
            }
        } else if (state == ST_ERROR) {
            state = ST_IDLE;
            uart_send_str("STATE:idle\n");
            lcd_clear();
            lcd_print("IDLE - OK");
            idle_was_pressed = 0;
            idle_long_pressed = 0;
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
            {
                static uint32_t last_color_check = 0;
                if (now - last_color_check >= 100) {
                    last_color_check = now;
                    uint32_t current_pulses = encoder_get_pulses();
                    uint16_t speed = encoder_get_speed_mm_s();
                    uint8_t allowed = 0;
                    
                    if (speed == 0) {
                        // fallback: time-based debounce of 2 seconds
                        if (first_detection || (now - last_detect_tick >= 2000)) {
                            allowed = 1;
                        }
                    } else {
                        if (first_detection || (current_pulses - last_detect_pulses >= min_spacing_pulses)) {
                            allowed = 1;
                        }
                    }
                    
                    if (allowed) {
                        if (tcs34725_is_present()) {
                            color_rgbc_t color;
                            tcs34725_get_raw(&color);
                            int8_t idx = color_match_index(&color);
                            if (idx >= 0) {
                                color_config_t colors[3];
                                uint8_t num_colors = calibration_load_all(colors, 3);
                                uint8_t sid = (idx < num_colors) ? colors[idx].servo_id : 0;
                                
                                first_detection = 0;
                                last_detect_pulses = current_pulses;
                                last_detect_tick = now;
                                
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
                                
                                if (sid == 1 || sid == 2) {
                                    state = ST_SORTING;
                                    sorting_phase = 0;
                                    sorting_servo_id = sid;
                                }
                            }
                        }
                    }
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
                static uint32_t sorting_timeout = 0;
                
                if (sorting_phase == 0) {
                    uint16_t speed = encoder_get_speed_mm_s();
                    if (speed == 0) {
                        // fallback speed if encoder is disconnected or not registering (100 mm/s)
                        speed = 100;
                    }
                    uint16_t dist = (sorting_servo_id == 1) ? distance_s1_mm : distance_s2_mm;
                    uint32_t transit_ms = (uint32_t)dist * 1000 / speed;
                    sorting_timeout = system_ticks + transit_ms;
                    sorting_phase = 1;
                }
                
                if (sorting_phase == 1) {
                    if (system_ticks >= sorting_timeout) {
                        if (!anti_jam_is_jammed() && state != ST_ERROR) {
                            uint8_t addr_defl = (sorting_servo_id == 1) ? EEPROM_ADDR_SERVO1_DEFL : EEPROM_ADDR_SERVO2_DEFL;
                            uint8_t addr_dwell = (sorting_servo_id == 1) ? EEPROM_ADDR_SERVO1_DWELL : EEPROM_ADDR_SERVO2_DWELL;
                            uint16_t defl = calibration_read_word(addr_defl);
                            uint16_t dwell = calibration_read_word(addr_dwell);
                            
                            servo_set_angle(sorting_servo_id, defl);
                            sorting_timeout = system_ticks + dwell;
                            sorting_phase = 2;
                        } else {
                            state = ST_RUNNING;
                            sorting_phase = 0;
                        }
                    }
                }
                
                if (sorting_phase == 2) {
                    if (system_ticks >= sorting_timeout) {
                        uint8_t addr_home = (sorting_servo_id == 1) ? EEPROM_ADDR_SERVO1_HOME : EEPROM_ADDR_SERVO2_HOME;
                        uint16_t home = calibration_read_word(addr_home);
                        servo_set_angle(sorting_servo_id, home);
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
                static uint8_t lcd_needs_update = 0;
                
                if (!menu_active) {
                    menu_active = 1;
                    menu_index = 0;
                    load_menu_values();
                    lcd_clear();
                    lcd_needs_update = 1;
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
                    } else if (!mode_long_pressed && (now - mode_press_start > 3000)) {
                        if (menu_index == 9) {
                            state = ST_IDLE;
                            menu_active = 0;
                            lcd_clear();
                            lcd_print("IDLE");
                            idle_was_pressed = 0;
                            idle_long_pressed = 0;
                            if (gpio_button_state(BTN_MODE)) {
                                ignore_mode_until_release = 1;
                            }
                        } else {
                            save_current_menu_value();
                            lcd_set_cursor(0, 0);
                            lcd_print("VALOR GUARDADO  ");
                        }
                        mode_long_pressed = 1;
                    }
                } else {
                    if (mode_was_pressed) {
                        if (!mode_long_pressed && (now - mode_press_start > 100)) {
                            menu_index = (menu_index + 1) % 10;
                            lcd_needs_update = 1;
                        }
                        mode_was_pressed = 0;
                    }
                }
                
                if (gpio_button_pressed(BTN_UP)) {
                    adjust_menu_value(1);
                    lcd_needs_update = 1;
                }
                if (gpio_button_pressed(BTN_DOWN)) {
                    adjust_menu_value(-1);
                    lcd_needs_update = 1;
                }
                
                if (lcd_needs_update) {
                    lcd_needs_update = 0;
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

        uart_send_str(" PWM:");
        u16_to_str(num_buf, motor_speed);
        uart_send_str(num_buf);
        uart_send_str("\n");
    }

    bt_protocol_process();  
}
