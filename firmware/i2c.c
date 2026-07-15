#include "i2c.h"
#include "config.h"

#define SSPADD_VAL 49

void i2c_init(void) {
    TRISBbits.TRISB0 = 1;  // SDA as input
    TRISBbits.TRISB1 = 1;  // SCL as input
    
    SSPCON1bits.SSPM = 0b1000;
    SSPCON1bits.SSPEN = 1;
    SSPADD = SSPADD_VAL;
    SSPSTATbits.SMP = 1;
}

static uint8_t i2c_wait_idle(void) {
    uint16_t timeout = 5000;
    while (((SSPCON2 & 0x1F) || (SSPSTAT & 0x04)) && --timeout);
    return (timeout > 0);
}

static uint8_t i2c_start(void) {
    if (!i2c_wait_idle()) return 0;
    SSPCON2bits.SEN = 1;
    uint16_t timeout = 5000;
    while (SSPCON2bits.SEN && --timeout);
    return (timeout > 0);
}

static uint8_t i2c_stop(void) {
    if (!i2c_wait_idle()) return 0;
    SSPCON2bits.PEN = 1;
    uint16_t timeout = 5000;
    while (SSPCON2bits.PEN && --timeout);
    return (timeout > 0);
}

static uint8_t i2c_write_byte(uint8_t b) {
    if (!i2c_wait_idle()) return 1;
    SSPBUF = b;
    uint16_t timeout = 5000;
    while (SSPSTATbits.BF && --timeout);
    if (timeout == 0) return 1;
    if (!i2c_wait_idle()) return 1;
    return SSPCON2bits.ACKSTAT;
}

static uint8_t i2c_read_byte(uint8_t ack, uint8_t *val) {
    if (!i2c_wait_idle()) return 0;
    SSPCON2bits.RCEN = 1;
    uint16_t timeout = 5000;
    while ((SSPSTATbits.BF == 0) && --timeout);
    if (timeout == 0) return 0;
    *val = SSPBUF;
    if (!i2c_wait_idle()) return 0;
    SSPCON2bits.ACKDT = ack ? 0 : 1;
    SSPCON2bits.ACKEN = 1;
    timeout = 5000;
    while (SSPCON2bits.ACKEN && --timeout);
    return (timeout > 0);
}

uint8_t i2c_write(uint8_t addr, const uint8_t *data, uint8_t len) {
    if (!i2c_start()) return 0;
    if (i2c_write_byte(addr << 1 | 0)) { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < len; i++) {
        if (i2c_write_byte(data[i])) { i2c_stop(); return 0; }
    }
    i2c_stop();
    return 1;
}

uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    if (!i2c_start()) return 0;
    if (i2c_write_byte(addr << 1 | 0)) { i2c_stop(); return 0; }
    if (i2c_write_byte(reg))           { i2c_stop(); return 0; }
    if (!i2c_start()) return 0;
    if (i2c_write_byte(addr << 1 | 1)) { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < len; i++) {
        uint8_t b;
        if (!i2c_read_byte(i < len - 1, &b)) { i2c_stop(); return 0; }
        buf[i] = b;
    }
    i2c_stop();
    return 1;
}

uint8_t i2c_probe(uint8_t addr) {
    if (!i2c_start()) return 0;
    uint8_t ack = i2c_write_byte(addr << 1 | 0);
    i2c_stop();
    return (ack == 0);
}
