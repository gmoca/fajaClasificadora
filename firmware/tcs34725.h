#ifndef TCS34725_H
#define TCS34725_H

#include <stdint.h>

typedef struct {
    uint16_t r, g, b, c;
} color_rgbc_t;

#define TCS34725_ADDR   0x29
#define TCS34725_CMD    0x80
#define TCS34725_ID     0x12

uint8_t tcs34725_init(void);
void    tcs34725_get_raw(color_rgbc_t *c);
void    tcs34725_set_gain(uint8_t gain);
void    tcs34725_set_integration_time(uint8_t cycles);

#endif
