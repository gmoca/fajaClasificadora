#ifndef LCD_H
#define LCD_H

#include "i2c.h"

#define LCD_ADDR 0x20

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);

#endif
