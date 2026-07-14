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

static void i2c_start(void) {
    SSPCON2bits.SEN = 1;
    while (SSPCON2bits.SEN);
}

static void i2c_stop(void) {
    SSPCON2bits.PEN = 1;
    while (SSPCON2bits.PEN);
}

static uint8_t i2c_write_byte(uint8_t b) {
    SSPBUF = b;
    while (SSPSTATbits.BF);
    return SSPCON2bits.ACKSTAT;
}

static uint8_t i2c_read_byte(uint8_t ack) {
    SSPCON2bits.RCEN = 1;
    while (SSPSTATbits.BF == 0);
    uint8_t b = SSPBUF;
    SSPCON2bits.ACKDT = ack ? 0 : 1;
    SSPCON2bits.ACKEN = 1;
    while (SSPCON2bits.ACKEN);
    return b;
}

uint8_t i2c_write(uint8_t addr, const uint8_t *data, uint8_t len) {
    i2c_start();
    if (i2c_write_byte(addr << 1 | 0)) { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < len; i++) {
        if (i2c_write_byte(data[i])) { i2c_stop(); return 0; }
    }
    i2c_stop();
    return 1;
}

uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_start();
    if (i2c_write_byte(addr << 1 | 0)) { i2c_stop(); return 0; }
    if (i2c_write_byte(reg))           { i2c_stop(); return 0; }
    i2c_start();
    if (i2c_write_byte(addr << 1 | 1)) { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < len; i++)
        buf[i] = i2c_read_byte(i < len - 1);
    i2c_stop();
    return 1;
}
