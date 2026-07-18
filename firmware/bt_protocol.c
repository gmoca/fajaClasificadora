#include "bt_protocol.h"
#include "uart.h"
#include "state_machine.h"
#include "pwm.h"
#include "servo.h"
#include "calibration.h"
#include "encoder.h"
#include "gpio.h"
#include "system.h"
#include "tcs34725.h"
#include <string.h>
#include <stdlib.h>

static char cmd_buf[64];
static uint8_t cmd_pos = 0;

uint32_t last_bt_activity = 0;

void bt_protocol_init(void) {
    last_bt_activity = 0;
}

static void u16_to_str(char *buf, uint16_t val) {
    char tmp[6];
    uint8_t i = 0;
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (val > 0 && i < 5) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    uint8_t j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

void bt_protocol_process(void) {
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    while (uart_available()) {
        uint8_t c = uart_read_byte();
        
        INTCONbits.GIEL = 0;
        last_bt_activity = system_ticks;
        INTCONbits.GIEL = 1;
        if (c == '\n' || c == '\r') {
            if (cmd_pos == 0) continue;
            cmd_buf[cmd_pos] = '\0';
            cmd_pos = 0;

            if (strcmp(cmd_buf, "START") == 0) {
                uart_send_str("CMD_OK:START\n");
                state_machine_start();
            }
            else if (strcmp(cmd_buf, "STOP") == 0) {
                uart_send_str("CMD_OK:STOP\n");
                state_machine_estop();
            }
            else if (strncmp(cmd_buf, "SET_SPEED ", 10) == 0) {
                int val = atoi(cmd_buf + 10);
                if (val >= 0 && val <= 255) {
                    state_machine_set_speed((uint8_t)val);
                    uart_send_str("CMD_OK:SET_SPEED\n");
                } else {
                    uart_send_str("CMD_ERR:SPEED_RANGE\n");
                }
            }
            else if (strcmp(cmd_buf, "STATUS") == 0) {
                uart_send_str("STATUS_RESP:0,0,1,0,0,0\n");
            }
            else if (strcmp(cmd_buf, "CALIBRATE") == 0) {
                uart_send_str("CMD_OK:CALIBRATE\n");
                calibration_start();
            }
            else if (strcmp(cmd_buf, "TEST_ENTER") == 0) {
                uart_send_str("CMD_OK:TEST_ENTER\n");
                state_machine_test_enter();
            }
            else if (strcmp(cmd_buf, "TEST_EXIT") == 0) {
                uart_send_str("CMD_OK:TEST_EXIT\n");
                state_machine_test_exit();
            }
            else if (strcmp(cmd_buf, "TEST_MOTOR") == 0) {
                uart_send_str("CMD_OK:TEST_MOTOR\n");
                state_machine_test_motor();
            }
            else if (strncmp(cmd_buf, "SERVO ", 6) == 0) {
                char *sp1 = strchr(cmd_buf + 6, ' ');
                if (sp1) {
                    *sp1 = '\0';
                    uint8_t sid = atoi(cmd_buf + 6);
                    uint16_t ang = atoi(sp1 + 1);
                    if ((sid == 1 || sid == 2) && ang <= 180) {
                        servo_set_angle(sid, ang);
                        char resp[32];
                        strcpy(resp, "CMD_OK:SERVO ");
                        char sid_str[4];
                        u16_to_str(sid_str, sid);
                        strcat(resp, sid_str);
                        strcat(resp, "\n");
                        uart_send_str(resp);
                    } else {
                        uart_send_str("CMD_ERR:SERVO_VAL\n");
                    }
                } else {
                    uart_send_str("CMD_ERR:SERVO_FMT\n");
                }
            }
            else if (strncmp(cmd_buf, "SET_MODE ", 9) == 0) {
                if (strcmp(cmd_buf + 9, "AUTO") == 0) {
                    state_machine_set_mode(1);
                    uart_send_str("CMD_OK:SET_MODE AUTO\n");
                }
                else if (strcmp(cmd_buf + 9, "MANUAL") == 0) {
                    state_machine_set_mode(0);
                    uart_send_str("CMD_OK:SET_MODE MANUAL\n");
                } else {
                    uart_send_str("CMD_ERR:MODE_VAL\n");
                }
            }
            else if (strncmp(cmd_buf, "SET_SPACING ", 12) == 0) {
                // Minimum encoder pulses spacing
                uint16_t space = atoi(cmd_buf + 12);
                state_machine_set_spacing(space);
            }
            else if (strncmp(cmd_buf, "SET_THRESHOLD ", 14) == 0) {
                // Format: SET_THRESHOLD <idx> <r_min> <r_max> <g_min> <g_max> <b_min> <b_max>
                int args[7];
                char *ptr = cmd_buf + 14;
                for (int i=0; i<7; i++) {
                    args[i] = atoi(ptr);
                    ptr = strchr(ptr, ' ');
                    if (!ptr) break;
                    ptr++;
                }
                color_config_t cfg;
                cfg.r_min = args[1]; cfg.r_max = args[2];
                cfg.g_min = args[3]; cfg.g_max = args[4];
                cfg.b_min = args[5]; cfg.b_max = args[6];
                calibration_save_color(args[0], &cfg);
            }
            else if (strncmp(cmd_buf, "SERVO_SET ", 10) == 0) {
                // Alias for SERVO
                char *sp1 = strchr(cmd_buf + 10, ' ');
                if (sp1) {
                    *sp1 = '\0';
                    uint8_t sid = atoi(cmd_buf + 10);
                    uint16_t ang = atoi(sp1 + 1);
                    if ((sid == 1 || sid == 2) && ang <= 180) {
                        servo_set_angle(sid, ang);
                    }
                }
            }
            else if (strncmp(cmd_buf, "SERVO_SAVE_HOME ", 16) == 0) {
                uint8_t sid = atoi(cmd_buf + 16);
                uint16_t angle = servo_get_angle(sid);
                calibration_save_servo_home(sid, angle);
            }
            else if (strncmp(cmd_buf, "SERVO_SAVE_DEFLECT ", 19) == 0) {
                uint8_t sid = atoi(cmd_buf + 19);
                uint16_t angle = servo_get_angle(sid);
                calibration_save_servo_deflect(sid, angle);
            }
            else if (strncmp(cmd_buf, "SERVO_GET_CONFIG ", 17) == 0) {
                uint8_t sid = atoi(cmd_buf + 17);
                calibration_send_servo_config(sid);
            }
            else if (strncmp(cmd_buf, "SET_DWELL ", 10) == 0) {
                char *sp1 = strchr(cmd_buf + 10, ' ');
                if (sp1) {
                    *sp1 = '\0';
                    uint8_t sid = atoi(cmd_buf + 10);
                    uint16_t dwell = atoi(sp1 + 1);
                    calibration_save_servo_dwell(sid, dwell);
                } else {
                    uint16_t dwell = atoi(cmd_buf + 10);
                    state_machine_set_dwell(dwell);
                }
            }
            else if (strcmp(cmd_buf, "TEST_ENCODER_RESET") == 0) {
                encoder_reset();
            }
            else if (strcmp(cmd_buf, "TEST_ENCODER_READ") == 0) {
                char buf[32];
                strcpy(buf, "ENC:");
                char num_buf[12];
                uint32_t val = encoder_get_pulses();
                uint8_t pos = 0;
                if (val == 0) { num_buf[pos++] = '0'; }
                else {
                    char tmp[11];
                    uint8_t i = 0;
                    while (val > 0 && i < 10) {
                        tmp[i++] = (char)('0' + (val % 10));
                        val /= 10;
                    }
                    while (i > 0) num_buf[pos++] = tmp[--i];
                }
                num_buf[pos] = '\0';
                strcat(buf, num_buf);
                strcat(buf, "\n");
                uart_send_str(buf);
            }
            else if (strncmp(cmd_buf, "TEST_BEAM", 9) == 0) {
                char buf[32];
                uint8_t s1 = gpio_breakbeam_read(1);
                uint8_t s2 = gpio_breakbeam_read(2);
                if (strlen(cmd_buf) > 9) {
                    uint8_t stn = atoi(cmd_buf + 10);
                    buf[0] = 'B'; buf[1] = 'E'; buf[2] = 'A'; buf[3] = 'M';
                    buf[4] = ':';
                    buf[5] = (char)('0' + stn);
                    buf[6] = ':';
                    buf[7] = (stn == 1 ? s1 : s2) ? 'B' : 'C';
                    buf[8] = '\n'; buf[9] = '\0';
                } else {
                    strcpy(buf, "BEAM:1:");
                    buf[6] = s1 ? 'B' : 'C';
                    buf[7] = ' ';
                    buf[8] = '2';
                    buf[9] = ':';
                    buf[10] = s2 ? 'B' : 'C';
                    buf[11] = '\n'; buf[12] = '\0';
                }
                uart_send_str(buf);
            }
            else if (strcmp(cmd_buf, "TEST_BUTTON_ECHO") == 0) {
                char buf[32];
                strcpy(buf, "BTN:");
                buf[4] = gpio_button_pressed(BTN_UP) ? '1' : '0';
                buf[5] = gpio_button_pressed(BTN_DOWN) ? '1' : '0';
                buf[6] = gpio_button_pressed(BTN_MODE) ? '1' : '0';
                buf[7] = '\n';
                buf[8] = '\0';
                uart_send_str(buf);
            }
            else if (strcmp(cmd_buf, "TEST_COLOR") == 0) {
                if (tcs34725_is_present()) {
                    color_rgbc_t color;
                    tcs34725_get_raw(&color);
                    char buf[64];
                    strcpy(buf, "COLOR:");
                    char num[8];
                    
                    u16_to_str(num, color.r);
                    strcat(buf, num);
                    strcat(buf, ",");
                    
                    u16_to_str(num, color.g);
                    strcat(buf, num);
                    strcat(buf, ",");
                    
                    u16_to_str(num, color.b);
                    strcat(buf, num);
                    strcat(buf, ",");
                    
                    u16_to_str(num, color.c);
                    strcat(buf, num);
                    strcat(buf, "\n");
                    
                    uart_send_str(buf);
                } else {
                    uart_send_str("COLOR_ERR:NOT_PRESENT\n");
                }
            }
            else if (strcmp(cmd_buf, "TEST_POLL") == 0) {
                char buf[64];
                strcpy(buf, "POLL_RESP:");
                
                // Break beams
                uint8_t s1 = gpio_break_beam_blocked(1);
                uint8_t s2 = gpio_break_beam_blocked(2);
                char beams[3];
                beams[0] = s1 ? 'B' : 'C';
                beams[1] = s2 ? 'B' : 'C';
                beams[2] = '\0';
                strcat(buf, beams);
                strcat(buf, ",");
                
                // Buttons
                char btns[4];
                btns[0] = gpio_button_pressed(BTN_UP) ? '1' : '0';
                btns[1] = gpio_button_pressed(BTN_DOWN) ? '1' : '0';
                btns[2] = gpio_button_pressed(BTN_MODE) ? '1' : '0';
                btns[3] = '\0';
                strcat(buf, btns);
                strcat(buf, ",");
                
                // Color RGBC
                color_rgbc_t color = {0, 0, 0, 0};
                if (tcs34725_is_present()) {
                    tcs34725_get_raw(&color);
                }
                
                char num[8];
                u16_to_str(num, color.r);
                strcat(buf, num);
                strcat(buf, ",");
                
                u16_to_str(num, color.g);
                strcat(buf, num);
                strcat(buf, ",");
                
                u16_to_str(num, color.b);
                strcat(buf, num);
                strcat(buf, ",");
                
                u16_to_str(num, color.c);
                strcat(buf, num);
                strcat(buf, "\n");
                
                uart_send_str(buf);
            }
            else {
                uart_send_str("CMD_ERR:UNKNOWN\n");
            }
        } else {
            if (cmd_pos < sizeof(cmd_buf) - 1) {
                cmd_buf[cmd_pos++] = c;
            }
        }
    }
}
