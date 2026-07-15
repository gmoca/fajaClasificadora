#include "tcs34725.h"
#include "config.h"
#include "i2c.h"

#define TCS34725_ATIME    0x01
#define TCS34725_CONTROL  0x0F
#define TCS34725_CDATAL   0x14

static uint8_t is_present = 0;

uint8_t tcs34725_is_present(void) {
    return is_present;
}

uint8_t tcs34725_init(void) {
    uint8_t id;
    is_present = 0;
    if (!i2c_read(TCS34725_ADDR, TCS34725_CMD | TCS34725_ID, &id, 1))
        return 0;
    if (id != 0x44 && id != 0x4D)
        return 0;

    uint8_t enable[] = { TCS34725_CMD | 0x00, 0x03 };
    i2c_write(TCS34725_ADDR, enable, 2);
    __delay_ms(3);

    tcs34725_set_integration_time(0xEB);
    tcs34725_set_gain(1);
    is_present = 1;
    return 1;
}

void tcs34725_set_gain(uint8_t gain) {
    uint8_t data[] = { TCS34725_CMD | TCS34725_CONTROL, gain & 0x03 };
    i2c_write(TCS34725_ADDR, data, 2);
}

void tcs34725_set_integration_time(uint8_t cycles) {
    uint8_t data[] = { TCS34725_CMD | TCS34725_ATIME, cycles };
    i2c_write(TCS34725_ADDR, data, 2);
}

void tcs34725_get_raw(color_rgbc_t *c) {
    uint8_t buf[8];
    i2c_read(TCS34725_ADDR, TCS34725_CMD | TCS34725_CDATAL, buf, 8);
    c->c = (uint16_t)(buf[0] | (uint16_t)(buf[1] << 8));
    c->r = (uint16_t)(buf[2] | (uint16_t)(buf[3] << 8));
    c->g = (uint16_t)(buf[4] | (uint16_t)(buf[5] << 8));
    c->b = (uint16_t)(buf[6] | (uint16_t)(buf[7] << 8));
}
