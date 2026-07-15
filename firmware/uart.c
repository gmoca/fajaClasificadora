#include "uart.h"
#include "config.h"

#define UART_RX_BUF 64
#define UART_TX_BUF 64

static volatile uint8_t rx_buf[UART_RX_BUF];
static volatile uint8_t tx_buf[UART_TX_BUF];
static volatile uint8_t rx_head = 0, rx_tail = 0, rx_count = 0;
static volatile uint8_t tx_head = 0, tx_tail = 0, tx_count = 0;

void uart_init(void) {
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH = 1;
    SPBRGH = (SPBRG_VAL >> 8) & 0xFF;
    SPBRG  = SPBRG_VAL & 0xFF;

    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    TXSTAbits.TXEN = 1;

    PIE1bits.RCIE = 1;
    IPR1bits.RCIP = 0;
    IPR1bits.TXIP = 0;  // Set TX interrupt to low priority to match isr_low
}

void uart_send_byte(uint8_t b) {
    while (tx_count >= UART_TX_BUF);
    INTCONbits.PEIE = 0;
    tx_buf[tx_head] = b;
    tx_head = (tx_head + 1) % UART_TX_BUF;
    tx_count++;
    INTCONbits.PEIE = 1;
    PIE1bits.TXIE = 1;
}

void uart_send_str(const char *s) {
    while (*s)
        uart_send_byte(*s++);
}

void uart_send_data(const uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++)
        uart_send_byte(buf[i]);
}

uint8_t uart_available(void) {
    return rx_count;
}

uint8_t uart_read_byte(void) {
    if (rx_count == 0) return 0;
    INTCONbits.PEIE = 0;
    uint8_t b = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART_RX_BUF;
    rx_count--;
    INTCONbits.PEIE = 1;
    return b;
}

void uart_isr_handler(void) {
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    if (PIR1bits.RCIF) {
        uint8_t b = RCREG;
        if (rx_count < UART_RX_BUF) {
            rx_buf[rx_head] = b;
            rx_head = (rx_head + 1) % UART_RX_BUF;
            rx_count++;
        }
    }
    if (PIR1bits.TXIF && PIE1bits.TXIE) {
        if (tx_count > 0) {
            TXREG = tx_buf[tx_tail];
            tx_tail = (tx_tail + 1) % UART_TX_BUF;
            tx_count--;
        } else {
            PIE1bits.TXIE = 0;
        }
    }
}
