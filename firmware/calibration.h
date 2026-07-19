#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "tcs34725.h"
#include <stdint.h>

#define EEPROM_MAGIC         0xA6
#define EEPROM_ADDR_MAGIC    0x00
#define EEPROM_ADDR_WHITE    0x01
#define EEPROM_ADDR_SERVO1_HOME 0x07
#define EEPROM_ADDR_SERVO1_DEFL 0x09
#define EEPROM_ADDR_SERVO1_DWELL 0x0B
#define EEPROM_ADDR_SERVO2_HOME 0x0D
#define EEPROM_ADDR_SERVO2_DEFL 0x0F
#define EEPROM_ADDR_SERVO2_DWELL 0x11
#define EEPROM_ADDR_ENC_PPR  0x13
#define EEPROM_ADDR_NUM_CLR  0x15
#define EEPROM_ADDR_COLORS   0x16
#define EEPROM_ADDR_SERVO1_DIST 0x60
#define EEPROM_ADDR_SERVO2_DIST 0x62

typedef struct {
    uint16_t r_min, r_max, g_min, g_max, b_min, b_max;
    uint8_t servo_id;
    char name[11];
} color_config_t;

void calibration_init(void);
void calibration_start(void);
uint8_t calibration_is_done(void);
void calibration_apply_white(color_rgbc_t *c);
void calibration_save_color(uint8_t idx, color_config_t *cfg);
uint8_t calibration_load_all(color_config_t *buf, uint8_t max);
void calibration_save_servo_home(uint8_t sid, uint16_t val);
void calibration_save_servo_deflect(uint8_t sid, uint16_t val);
void calibration_save_servo_dwell(uint8_t sid, uint16_t val);
void calibration_save_servo_dist(uint8_t sid, uint16_t val);
void calibration_save_ppr(uint16_t val);
void calibration_send_servo_config(uint8_t sid);
void calibration_write_word(uint8_t addr, uint16_t val);
uint16_t calibration_read_word(uint8_t addr);

#endif
