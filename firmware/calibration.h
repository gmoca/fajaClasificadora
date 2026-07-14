#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "tcs34725.h"
#include <stdint.h>

#define EEPROM_ADDR_NUM_CLR  0x15
#define EEPROM_ADDR_COLORS   0x16

typedef struct {
    uint16_t r_min, r_max, g_min, g_max, b_min, b_max;
    char name[12];
} color_config_t;

void calibration_start(void);
uint8_t calibration_is_done(void);
void calibration_apply_white(color_rgbc_t *c);
void calibration_save_color(uint8_t idx, color_config_t *cfg);
uint8_t calibration_load_all(color_config_t *buf, uint8_t max);
void calibration_save_servo_home(uint8_t sid);
void calibration_send_servo_config(uint8_t sid);

#endif
