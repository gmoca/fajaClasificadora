#include "lcd.h"
#include "config.h"

#define LCD_BL  0x08
#define LCD_EN  0x04
#define LCD_RW  0x02
#define LCD_RS  0x01

static void lcd_send_nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble << 4) | (rs ? LCD_RS : 0) | LCD_BL;
    i2c_write(LCD_ADDR, &data, 1);
    data |= LCD_EN;
    i2c_write(LCD_ADDR, &data, 1);
    data &= ~LCD_EN;
    i2c_write(LCD_ADDR, &data, 1);
}

static void lcd_send_byte(uint8_t b, uint8_t rs) {
    lcd_send_nibble(b >> 4, rs);
    lcd_send_nibble(b & 0x0F, rs);
}

static void lcd_cmd(uint8_t cmd) {
    lcd_send_byte(cmd, 0);
    __delay_us(100);
}

void lcd_init(void) {
    __delay_ms(50);
    lcd_send_nibble(0x03, 0); __delay_ms(5);
    lcd_send_nibble(0x03, 0); __delay_ms(5);
    lcd_send_nibble(0x03, 0); __delay_us(150);
    lcd_send_nibble(0x02, 0);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_clear();
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    __delay_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr[] = { 0x00, 0x40, 0x14, 0x54 };
    lcd_cmd(0x80 | (addr[row] + col));
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, 1);
        __delay_us(50);
    }
}
