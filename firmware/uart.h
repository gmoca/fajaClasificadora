#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_send_str(const char *s);
void uart_send_byte(uint8_t b);
void uart_send_data(const uint8_t *buf, uint8_t len);
uint8_t uart_available(void);
uint8_t uart_read_byte(void);
void uart_isr_handler(void);

#endif
