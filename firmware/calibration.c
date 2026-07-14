#include "calibration.h"
#include "uart.h"
#include <xc.h>

static uint8_t eeprom_read_byte(uint8_t addr) {
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

static void eeprom_write_byte(uint8_t addr, uint8_t val) {
    while (EECON1bits.WR);
    EEADR = addr;
    EEDATA = val;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
}

static uint8_t is_calibrating = 0;

void calibration_start(void) {
    is_calibrating = 1;
}

uint8_t calibration_is_done(void) {
    return !is_calibrating;
}

void calibration_apply_white(color_rgbc_t *c) {
    if (is_calibrating) {
        // Simple mock of applying white balance
        is_calibrating = 0;
    }
}

void calibration_save_color(uint8_t idx, color_config_t *cfg) {
    uint8_t addr = EEPROM_ADDR_COLORS + (idx * sizeof(color_config_t));
    uint8_t *ptr = (uint8_t *)cfg;
    for (uint8_t i = 0; i < sizeof(color_config_t); i++) {
        eeprom_write_byte(addr + i, ptr[i]);
    }
}

uint8_t calibration_load_all(color_config_t *buf, uint8_t max) {
    uint8_t num = eeprom_read_byte(EEPROM_ADDR_NUM_CLR);
    if (num > max) num = max;
    if (num == 0xFF) return 0;

    for (uint8_t i = 0; i < num; i++) {
        uint8_t addr = EEPROM_ADDR_COLORS + (i * sizeof(color_config_t));
        uint8_t *ptr = (uint8_t *)&buf[i];
        for (uint8_t j = 0; j < sizeof(color_config_t); j++) {
            ptr[j] = eeprom_read_byte(addr + j);
        }
    }
    return num;
}

void calibration_save_servo_home(uint8_t sid) {
    // Stub for servo save home
}

void calibration_send_servo_config(uint8_t sid) {
    // Stub for send servo config
    uart_send_str("SERVO_CFG:1,0,180\n");
}
