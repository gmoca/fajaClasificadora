#include "calibration.h"
#include "uart.h"
#include <string.h>
#include <xc.h>

static uint8_t eeprom_read_byte(uint8_t addr) {
    while (EECON1bits.WR);
    while (EECON1bits.RD);  // Wait for any active read to complete
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    while (EECON1bits.RD);  // Wait for this read to complete
    return EEDATA;
}

static void eeprom_write_byte(uint8_t addr, uint8_t val) {
    while (EECON1bits.WR);
    while (EECON1bits.RD);  // Wait for any active read to complete
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
    while (EECON1bits.WR);
    EECON1bits.WREN = 0;
}

void calibration_write_word(uint8_t addr, uint16_t val) {
    eeprom_write_byte(addr, (uint8_t)(val >> 8));
    eeprom_write_byte(addr + 1, (uint8_t)(val & 0xFF));
}

uint16_t calibration_read_word(uint8_t addr) {
    uint16_t high = eeprom_read_byte(addr);
    uint16_t low = eeprom_read_byte(addr + 1);
    return (high << 8) | low;
}

void calibration_init(void) {
    uint8_t magic = eeprom_read_byte(EEPROM_ADDR_MAGIC);
    if (magic != EEPROM_MAGIC) {
        // Write defaults
        eeprom_write_byte(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
        calibration_write_word(EEPROM_ADDR_SERVO1_HOME, 90);
        calibration_write_word(EEPROM_ADDR_SERVO1_DEFL, 0);
        calibration_write_word(EEPROM_ADDR_SERVO1_DWELL, 500);
        calibration_write_word(EEPROM_ADDR_SERVO2_HOME, 90);
        calibration_write_word(EEPROM_ADDR_SERVO2_DEFL, 0);
        calibration_write_word(EEPROM_ADDR_SERVO2_DWELL, 500);
        calibration_write_word(EEPROM_ADDR_ENC_PPR, 20);
        eeprom_write_byte(EEPROM_ADDR_NUM_CLR, 0);
    }
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

void calibration_save_servo_home(uint8_t sid, uint16_t val) {
    uint8_t addr = (sid == 1) ? EEPROM_ADDR_SERVO1_HOME : EEPROM_ADDR_SERVO2_HOME;
    calibration_write_word(addr, val);
}

void calibration_save_servo_deflect(uint8_t sid, uint16_t val) {
    uint8_t addr = (sid == 1) ? EEPROM_ADDR_SERVO1_DEFL : EEPROM_ADDR_SERVO2_DEFL;
    calibration_write_word(addr, val);
}

void calibration_save_servo_dwell(uint8_t sid, uint16_t val) {
    uint8_t addr = (sid == 1) ? EEPROM_ADDR_SERVO1_DWELL : EEPROM_ADDR_SERVO2_DWELL;
    calibration_write_word(addr, val);
}

void calibration_save_ppr(uint16_t val) {
    calibration_write_word(EEPROM_ADDR_ENC_PPR, val);
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

void calibration_send_servo_config(uint8_t sid) {
    uint8_t addr_home = (sid == 1) ? EEPROM_ADDR_SERVO1_HOME : EEPROM_ADDR_SERVO2_HOME;
    uint8_t addr_defl = (sid == 1) ? EEPROM_ADDR_SERVO1_DEFL : EEPROM_ADDR_SERVO2_DEFL;
    uint8_t addr_dwell = (sid == 1) ? EEPROM_ADDR_SERVO1_DWELL : EEPROM_ADDR_SERVO2_DWELL;

    uint16_t home = calibration_read_word(addr_home);
    uint16_t defl = calibration_read_word(addr_defl);
    uint16_t dwell = calibration_read_word(addr_dwell);

    char buf[64];
    strcpy(buf, "SERVO_CFG:");
    char num[8];
    u16_to_str(num, sid);
    strcat(buf, num);
    strcat(buf, ",");
    u16_to_str(num, home);
    strcat(buf, num);
    strcat(buf, ",");
    u16_to_str(num, defl);
    strcat(buf, num);
    strcat(buf, ",");
    u16_to_str(num, dwell);
    strcat(buf, num);
    strcat(buf, "\n");

    uart_send_str(buf);
}
