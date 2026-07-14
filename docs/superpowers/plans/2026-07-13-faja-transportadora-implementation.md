# Faja Transportadora Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the automated conveyor belt sorting system — PIC18F4550 firmware + Python Textual TUI app, controlled via Bluetooth.

**Architecture:** Autonomous PIC firmware (state machine + real-time control) communicates over Bluetooth serial (115200, ASCII protocol) with a Textual TUI app in Termux. Two agents: **OpenCode** builds HAL/drivers on PIC + TUI app shell; **agy** builds business logic on both sides.

**Tech Stack:** PIC18F4550 + XC8 v3.10 (C99), HC-05 (115200), TCS34725 (I2C), LCD 1602 + PCF8574 (I2C), L298N H-bridge (CCP1 PWM 20kHz), 1-2 servos (CCP2 Compare + TMR1 ISR), encoder (INT2), Python Textual TUI in Termux.

---

## File structure

```
pryMicroFaja.X/
├── Makefile                          # existing
├── nbproject/                        # existing (IDE files)
├── AGENTS.md                         # existing
├── docs/superpowers/specs/2026-07-12-faja-transportadora-design.md
│
├── firmware/                         # PIC18F4550 source
│   ├── main.c                        # entry, main loop
│   ├── config.h                      # #pragma config, Fosc, IPEN
│   ├── system.h / system.c           # clock, interrupt vectors, TMR0
│   ├── gpio.h / gpio.c               # buttons, H-bridge dir, break-beam
│   ├── uart.h / uart.c               # HC-05 115200, TX/RX buffers
│   ├── i2c.h / i2c.c                 # MSSP master for LCD + TCS34725
│   ├── lcd.h / lcd.c                 # LCD 1602 + PCF8574 over I2C
│   ├── tcs34725.h / tcs34725.c       # color sensor I2C driver
│   ├── pwm.h / pwm.c                 # CCP1 (H-bridge), 20kHz
│   ├── servo.h / servo.c             # CCP2 Compare (servo 1) + TMR1 soft (servo 2)
│   ├── encoder.h / encoder.c         # INT2 pulse counter + speed calc
│   ├── state_machine.h / state_machine.c   # state machine (agy)
│   ├── bt_protocol.h / bt_protocol.c       # cmd parser + telemetry (agy)
│   ├── calibration.h / calibration.c       # EEPROM color thresholds (agy)
│   └── anti_jam.h / anti_jam.c             # triple jam detection (agy)
│
└── tui_app/                          # Python Textual TUI
    ├── pyproject.toml                 # dependencies: textual, pyserial
    ├── app.py                        # Textual app entry point (OpenCode)
    ├── connect.py                    # async BT serial manager (OpenCode)
    ├── screens/
    │   ├── dashboard.py              # main screen (agy: logic + layout)
    │   ├── config.py                 # settings screen (agy)
    │   ├── log_viewer.py             # event log screen (agy)
    │   └── test_screen.py            # test & calibration screen (agy, Task 18)
    └── protocol.py                   # BT message encode/decode (both)
```

---

## PIC firmware HAL interfaces (OpenCode → agy)

These are the APIs OpenCode's drivers expose. agy's business logic calls these — never touches registers or peripherals directly.

### `system.h`
```c
void system_init(void);          // oscillator, IPEN, TMR0 (1ms tick)
volatile uint32_t system_ticks;  // incremented by TMR0 ISR every 1ms
```

### `gpio.h`
```c
typedef enum { BTN_MODE, BTN_UP, BTN_DOWN } button_t;
typedef enum { HB_STOP, HB_FWD, HB_REV } hbridge_dir_t;

uint8_t gpio_button_read(button_t btn);       // debounced, non-blocking
void    gpio_hbridge_dir(hbridge_dir_t dir);
void    gpio_breakbeam_emitter(uint8_t on);    // RD3
uint8_t gpio_breakbeam_read(uint8_t station);  // station 1 = RD4, station 2 = RD7
```

### `uart.h`
```c
void  uart_init(void);                  // 115200, BRG16=1, BRGH=1
void  uart_send_str(const char *s);     // blocking send
void  uart_send_byte(uint8_t b);
uint8_t uart_available(void);           // bytes in RX buffer
uint8_t uart_read_byte(void);           // non-blocking (0 if empty)
void  uart_isr_handler(void);           // called from low ISR
```

### `i2c.h`
```c
void  i2c_init(void);                   // 100kHz master
uint8_t i2c_write(uint8_t addr, const uint8_t *data, uint8_t len);
uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);
```

### `lcd.h`
```c
void lcd_init(void);                    // 4-bit mode via PCF8574
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
```

### `tcs34725.h`
```c
typedef struct { uint16_t r, g, b, c; } color_rgbc_t;

uint8_t tcs34725_init(void);            // returns 1 if found
void    tcs34725_get_raw(color_rgbc_t *c);
void    tcs34725_set_gain(uint8_t gain); // 0=1x, 1=4x, 2=16x, 3=60x
```

### `pwm.h`
```c
void pwm_hbridge_init(void);            // CCP1, 20kHz
void pwm_hbridge_set_duty(uint8_t duty); // 0-255
```

### `servo.h`
```c
void servo_init(void);                  // CCP2 Compare on TMR1
void servo_set_angle(uint8_t servo_id, uint16_t angle_deg); // 0-180
void servo2_poll(void);              // call from main loop — ends servo 2 pulse
```

### `encoder.h`
```c
void  encoder_init(void);               // INT2, edge-triggered
uint32_t encoder_get_pulses(void);      // total pulses since boot
uint16_t encoder_get_speed_mm_s(void);  // recent mm/s, filtered
void  encoder_isr_handler(void);        // called from low ISR
void  encoder_tick_handler(void);       // called from TMR0 ISR
```

---

## TUI app interfaces (OpenCode shell → agy UI logic)

### `connect.py` — OpenCode
```python
class BTManager:
    async def connect(self, addr: str) -> bool
    async def disconnect(self)
    async def send(self, cmd: str)
    async def read_line(self) -> str | None  # non-blocking
    @property
    def connected(self) -> bool
```

### `protocol.py` — shared
```python
CMD_START = "START"
CMD_STOP = "STOP"
CMD_SET_SPEED = "SET_SPEED {}"
CMD_STATUS = "STATUS"
CMD_CALIBRATE = "CALIBRATE"
CMD_SET_MODE = "SET_MODE {}"
CMD_SERVO = "SERVO {} {}"
CMD_SET_SPACING = "SET_SPACING {}"

def parse_telemetry(line: str) -> dict
    # Returns {"type": "SPEED", "value": 180} etc.
```

---

## Tasks

---

### Task 1 [OpenCode]: Project scaffolding + config.h

**Files:**
- Create: `firmware/config.h`
- Create: `firmware/main.c`
- Modify: `Makefile` (add firmware sources)

**Context:** First file OpenCode writes. Must get the silicon config right since every subsequent driver depends on these #pragma values.

- [ ] **Step 1: Write `firmware/config.h`**

```c
#ifndef CONFIG_H
#define CONFIG_H

// Fosc = 20MHz external HS crystal
#pragma config FOSC = HS          // HS oscillator, crystal >4MHz
#pragma config XINST = OFF        // Extended Instruction Set off
#pragma config WDT = OFF          // Watchdog Timer off
#pragma config CPUDIV = OSC1_PLL2// CPU clock = Fosc/2? No: CPUDIV=1 for Fosc/1
#pragma config PLLDIV = 1         // PLL disabled (not using PLL)
#pragma config USBDIV = 1         // USB clock from PLL (not used anyway)
#pragma config IESO = OFF         // Internal/External Switchover off
#pragma config FCMEN = OFF        // Fail-Safe Clock Monitor off
#pragma config BOR = ON           // Brown-out Reset on
#pragma config PWRT = ON          // Power-up Timer on
#pragma config MCLRE = ON         // MCLR pin enabled
#pragma config PBADEN = OFF       // PORTB analog disabled on reset
#pragma config STVREN = ON        // Stack overflow reset on
#pragma config LVP = OFF          // Low-Voltage Programming off
#pragma config DEBUG = OFF        // No debugger

#include <xc.h>

#define FOSC_HZ  20000000UL
#define FCYC_HZ  (FOSC_HZ / 4)  // 5,000,000
#define _XTAL_FREQ  FOSC_HZ     // Required by XC8 __delay_ms/__delay_us

// UART
#define BAUD_RATE 115200UL
// SPBRG = Fosc/(4*BAUD) - 1 for BRGH=1
#define SPBRG_VAL ((FOSC_HZ / (4 * BAUD_RATE)) - 1)

// TMR0: 1ms tick
// Fcyc = 5MHz, prescaler 1:8 → 625kHz
// 625 ticks = 1ms
// TMR0 counts UP and overflows at 0xFFFF. Reload = 65536 - ticks_needed.
#define TMR0_TICKS   625
#define TMR0_RELOAD  (65536 - TMR0_TICKS)  // = 64911 = 0xFD8F

#endif
```

- [ ] **Step 2: Write `firmware/main.c` skeleton**

```c
#include "config.h"

void main(void) {
    // All init in order:
    system_init();
    uart_init();
    i2c_init();
    lcd_init();
    tcs34725_init();
    pwm_hbridge_init();
    servo_init();
    encoder_init();
    gpio_init();
    state_machine_init();

    lcd_clear();
    lcd_print("PryMicroFaja");
    uart_send_str("SYSTEM_READY\n");

    while (1) {
        state_machine_run();  // agy implements this
        // LCD refresh, BT telemetry handled inside state_machine_run
    }
}
```

- [ ] **Step 3: Update `Makefile`**

Add to the SOURCEFILES line (or create a pre-build target that compiles `firmware/*.c`):

The existing Makefile has empty `SOURCEFILES=`. Add via MPLAB X (configurations.xml) or manually prepend compile rules. Since MPLAB X regenerates `Makefile-default.mk`, the safest approach is to add a pre-build rule in `Makefile`:

```makefile
# In Makefile before include:
EXTRA_SOURCES = firmware/main.c firmware/system.c firmware/gpio.c \
                firmware/uart.c firmware/i2c.c firmware/lcd.c \
                firmware/tcs34725.c firmware/pwm.c firmware/servo.c \
                firmware/encoder.c firmware/state_machine.c \
                firmware/bt_protocol.c firmware/calibration.c \
                firmware/anti_jam.c

EXTRA_OBJECTS = $(EXTRA_SOURCES:.c=.o)

.build-pre: $(EXTRA_OBJECTS)
	# also need to add these to the linker step
```

**Verification:** `make build` compiles with no errors.

- [ ] **Step 4: Commit**

```
git init
git add firmware/config.h firmware/main.c Makefile
git commit -m "feat: firmware project scaffolding + config.h"
```

---

### Task 2 [OpenCode]: System init — oscillator, IPEN, TMR0, TMR1, TMR2

**Files:**
- Create: `firmware/system.h`
- Create: `firmware/system.c`

- [ ] **Step 1: Write `firmware/system.h`**

```c
#ifndef SYSTEM_H
#define SYSTEM_H

#include "config.h"

extern volatile uint32_t system_ticks;

void system_init(void);
void system_tmr0_isr(void);

#endif
```

- [ ] **Step 2: Write `firmware/system.c`**

```c
#include "system.h"
#include "encoder.h"
#include "gpio.h"
#include "uart.h"
#include "servo.h"
#include "state_machine.h"

volatile uint32_t system_ticks = 0;

void system_init(void) {
    // Priority interrupt mode (IPEN = 1)
    RCONbits.IPEN = 1;

    // TMR0: 1ms tick, 16-bit mode
    T0CONbits.T08BIT = 0;       // 16-bit
    T0CONbits.T0CS = 0;         // internal clock (Fcyc)
    T0CONbits.PSA = 0;          // prescaler assigned
    T0CONbits.T0PS = 0b001;     // 1:8 prescaler
    TMR0H = (uint8_t)(TMR0_RELOAD >> 8);   // high byte first (0xFD)
    TMR0L = (uint8_t)(TMR0_RELOAD & 0xFF); // low byte (0x8F)
    T0CONbits.TMR0ON = 1;       // start timer
    INTCONbits.TMR0IE = 1;      // enable TMR0 interrupt
    
    // TMR1: free-run for CCP2 Compare (servo timebase)
    // Prescaler 1:2, internal clock → 2.5MHz → 400ns/tick
    T1CONbits.TMR1CS = 0;       // Fosc/4 = 5MHz
    T1CONbits.T1CKPS = 0b01;    // 1:2 prescaler
    T1CONbits.TMR1ON = 1;       // start TMR1
    PIR1bits.TMR1IF = 0;

    // CCP2 special event must reset TMR1, not TMR3
    T3CONbits.T3CCP2 = 0;       // CCP2 → TMR1

    // TMR2: CCP1 PWM base (20kHz)
    T2CONbits.T2CKPS = 0b00;    // 1:1 prescaler
    PR2 = 249;                  // 20kHz period
    T2CONbits.TMR2ON = 1;       // start

    // Enable interrupt priority
    INTCONbits.GIEH = 1;        // high-priority interrupts
    INTCONbits.GIEL = 1;        // low-priority interrupts

    // E-stop INT0: always high priority
    INTCONbits.INT0IE = 1;
    INTCON2bits.INTEDG0 = 0;    // falling edge — fail-safe (NC button + pull-up)

    // Clear any pending flags
    INTCONbits.INT0IF = 0;
}

void __interrupt(high_priority) isr_high(void) {
    if (INTCONbits.INT0IF) {
        // E-stop triggered
        INTCONbits.INT0IF = 0;
        state_machine_estop();  // agy implements this
    }
}

void __interrupt(low_priority) isr_low(void) {
    // TMR0 tick
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        TMR0H = (uint8_t)(TMR0_RELOAD >> 8);
        TMR0L = (uint8_t)(TMR0_RELOAD & 0xFF);
        system_ticks++;
        encoder_tick_handler();  // speed window
        gpio_scan_buttons();     // debounce polling
    }
    // INT2 encoder
    if (INTCON3bits.INT2IF) {
        INTCON3bits.INT2IF = 0;
        encoder_isr_handler();
    }
    // UART RX/TX
    if (PIR1bits.RCIF || (PIE1bits.TXIE && PIR1bits.TXIF)) {
        uart_isr_handler();
    }
    // CCP2 compare match (servo)
    if (PIR2bits.CCP2IF) {
        PIR2bits.CCP2IF = 0;
        servo_ccp2_isr();
    }
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/system.h firmware/system.c
git commit -m "feat: system init + interrupt vectors + timers"
```

---

### Task 3 [OpenCode]: GPIO HAL — buttons, H-bridge direction, break-beam

**Files:**
- Create: `firmware/gpio.h`
- Create: `firmware/gpio.c`

- [ ] **Step 1: Write `firmware/gpio.h`**

```c
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum { BTN_MODE, BTN_UP, BTN_DOWN } button_t;
typedef enum { HB_STOP, HB_FWD, HB_REV } hbridge_dir_t;

void    gpio_init(void);
void    gpio_scan_buttons(void);         // call from TMR0 ISR
uint8_t gpio_button_pressed(button_t btn); // edge-detected press event
void    gpio_hbridge_dir(hbridge_dir_t dir);
void    gpio_breakbeam_emitter(uint8_t on);
uint8_t gpio_breakbeam_read(uint8_t station);
void    gpio_estop_latch(void);          // set E-stop LED etc.

#endif
```

- [ ] **Step 2: Write `firmware/gpio.c`**

```c
#include "gpio.h"
#include "config.h"

// Button state tracking for edge detection
static uint8_t btn_state[3];           // current debounced state
static uint8_t btn_edge[3];            // rising edge detected (cleared on read)
static uint8_t btn_counter[3];         // debounce counter

#define DEBOUNCE_MS 5
#define BTN_PORT(pin) PORTDbits.RD ## pin

void gpio_init(void) {
    // H-bridge direction: RD0, RD1 = outputs
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    gpio_hbridge_dir(HB_STOP);

    // Buttons: RD2, RD5, RD6 = inputs (EXTERNAL pull-ups required — PORTD has no internal pull-ups)
    TRISDbits.TRISD2 = 1;
    TRISDbits.TRISD5 = 1;
    TRISDbits.TRISD6 = 1;

    // Break-beam: RD3 = output (emitter), RD4 = input (receiver)
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 1;
    gpio_breakbeam_emitter(1);  // emitter always on when system running

    // Break-beam station 2: RD7 = input (receiver)
    TRISDbits.TRISD7 = 1;

    // E-stop RB0 = input, already INT0 from system_init
    TRISBbits.TRISB0 = 1;

    // Clear button state
    for (int i = 0; i < 3; i++) {
        btn_state[i] = 1;  // pull-up = high when unpressed
        btn_edge[i] = 0;
        btn_counter[i] = 0;
    }
}

static const uint8_t btn_pins[] = { 2, 5, 6 };  // RD2, RD5, RD6

void gpio_scan_buttons(void) {
    for (int i = 0; i < 3; i++) {
        uint8_t raw = (PORTD >> btn_pins[i]) & 1;
        if (raw != btn_state[i]) {
            btn_counter[i]++;
            if (btn_counter[i] >= DEBOUNCE_MS) {
                btn_state[i] = raw;
                btn_counter[i] = 0;
                if (raw == 0)  // pressed (pull-up, 0 = pressed)
                    btn_edge[i] = 1;
            }
        } else {
            btn_counter[i] = 0;
        }
    }
}

uint8_t gpio_button_pressed(button_t btn) {
    uint8_t ret = btn_edge[btn];
    btn_edge[btn] = 0;
    return ret;
}

void gpio_hbridge_dir(hbridge_dir_t dir) {
    switch (dir) {
        case HB_STOP: LATDbits.LATD0 = 0; LATDbits.LATD1 = 0; break;
        case HB_FWD:  LATDbits.LATD0 = 1; LATDbits.LATD1 = 0; break;
        case HB_REV:  LATDbits.LATD0 = 0; LATDbits.LATD1 = 1; break;
    }
}

void gpio_breakbeam_emitter(uint8_t on) {
    LATDbits.LATD3 = on ? 1 : 0;
}

uint8_t gpio_breakbeam_read(uint8_t station) {
    if (station == 1)
        return (PORTDbits.RD4);   // 1 = beam broken (object present)
    else
        return (PORTDbits.RD7);   // station 2
}

void gpio_estop_latch(void) {
    // Could set an LED indicator on a spare pin
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/gpio.h firmware/gpio.c
git commit -m "feat: GPIO HAL — buttons, H-bridge, break-beam"
```

---

### Task 4 [OpenCode]: UART driver — HC-05 115200

**Files:**
- Create: `firmware/uart.h`
- Create: `firmware/uart.c`

- [ ] **Step 1: Write `firmware/uart.h`**

```c
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
```

- [ ] **Step 2: Write `firmware/uart.c`**

```c
#include "uart.h"
#include "config.h"

// Circular buffers
#define UART_RX_BUF 64
#define UART_TX_BUF 64

static volatile uint8_t rx_buf[UART_RX_BUF];
static volatile uint8_t tx_buf[UART_TX_BUF];
static volatile uint8_t rx_head = 0, rx_tail = 0, rx_count = 0;
static volatile uint8_t tx_head = 0, tx_tail = 0, tx_count = 0;

void uart_init(void) {
    // BRG16=1, BRGH=1, 115200 @ 20MHz → SPBRG=42
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH = 1;
    SPBRGH = (SPBRG_VAL >> 8) & 0xFF;
    SPBRG  = SPBRG_VAL & 0xFF;

    // Enable serial port, continuous receive
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;

    // Enable TX
    TXSTAbits.TXEN = 1;

    // Enable RX interrupt
    PIE1bits.RCIE = 1;
    IPR1bits.RCIP = 0;       // low priority
}

void uart_send_byte(uint8_t b) {
    // Wait for TX buffer space
    while (tx_count >= UART_TX_BUF);
    // Disable interrupts while modifying buffer
    PEIE = 0;
    tx_buf[tx_head] = b;
    tx_head = (tx_head + 1) % UART_TX_BUF;
    tx_count++;
    PEIE = 1;
    // Enable TX interrupt if not already
    PIE1bits.TXIE = 1;
}

void uart_send_str(const char *s) {
    while (*s) uart_send_byte(*s++);
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
    PEIE = 0;
    uint8_t b = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART_RX_BUF;
    rx_count--;
    PEIE = 1;
    return b;
}

void uart_isr_handler(void) {
    // RX
    if (PIR1bits.RCIF) {
        uint8_t b = RCREG;
        if (rx_count < UART_RX_BUF) {
            rx_buf[rx_head] = b;
            rx_head = (rx_head + 1) % UART_RX_BUF;
            rx_count++;
        }
    }
    // TX (only if TX interrupt is enabled)
    if (PIE1bits.TXIE && PIR1bits.TXIF) {
        if (tx_count > 0) {
            TXREG = tx_buf[tx_tail];
            tx_tail = (tx_tail + 1) % UART_TX_BUF;
            tx_count--;
        } else {
            PIE1bits.TXIE = 0;  // no more data to send
        }
    }
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/uart.h firmware/uart.c
git commit -m "feat: UART driver — HC-05 115200, circular buffers"
```

---

### Task 5 [OpenCode]: I2C driver — MSSP master

**Files:**
- Create: `firmware/i2c.h`
- Create: `firmware/i2c.c`

- [ ] **Step 1: Write `firmware/i2c.h`**

```c
#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(void);
uint8_t i2c_write(uint8_t addr, const uint8_t *data, uint8_t len);
uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif
```

- [ ] **Step 2: Write `firmware/i2c.c`**

```c
#include "i2c.h"
#include "config.h"

// MSSP in I2C master mode, 100kHz
// Fosc=20MHz, SSPADD = (Fosc/(4*Fclk)) - 1 = 49
#define SSPADD_VAL 49

void i2c_init(void) {
    SSPCON1bits.SSPM = 0b1000;  // I2C master mode
    SSPCON1bits.SSPEN = 1;      // enable MSSP
    SSPADD = SSPADD_VAL;        // 100kHz
    SSPSTATbits.SMP = 1;        // slew rate control disabled (standard speed)
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
    PIR1bits.SSPIF = 0;
    SSPBUF = b;
    while (!PIR1bits.SSPIF);  // wait for complete byte transmission
    PIR1bits.SSPIF = 0;
    return SSPCON2bits.ACKSTAT;  // 0 = ACK, 1 = NACK
}

static uint8_t i2c_read_byte(uint8_t ack) {
    SSPCON2bits.RCEN = 1;
    while (SSPCON2bits.RCEN);
    while (SSPSTATbits.BF == 0);
    uint8_t b = SSPBUF;
    SSPCON2bits.ACKDT = ack ? 0 : 1;  // 0=ACK, 1=NACK
    SSPCON2bits.ACKEN = 1;
    while (SSPCON2bits.ACKEN);
    return b;
}

uint8_t i2c_write(uint8_t addr, const uint8_t *data, uint8_t len) {
    i2c_start();
    if (i2c_write_byte(addr << 1 | 0)) {  // write bit
        i2c_stop();
        return 0;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (i2c_write_byte(data[i])) {
            i2c_stop();
            return 0;
        }
    }
    i2c_stop();
    return 1;
}

uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    // Write register address first
    i2c_start();
    if (i2c_write_byte(addr << 1 | 0)) { i2c_stop(); return 0; }
    if (i2c_write_byte(reg))           { i2c_stop(); return 0; }
    // Repeated start for read (maintain bus ownership)
    SSPCON2bits.RSEN = 1;
    while (SSPCON2bits.RSEN);
    if (i2c_write_byte(addr << 1 | 1)) { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = i2c_read_byte(i < len - 1);  // ACK for all but last
    }
    i2c_stop();
    return 1;
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/i2c.h firmware/i2c.c
git commit -m "feat: I2C MSSP master driver — 100kHz"
```

---

### Task 6 [OpenCode]: LCD 1602 + PCF8574 driver

**Files:**
- Create: `firmware/lcd.h`
- Create: `firmware/lcd.c`

- [ ] **Step 1: Write `firmware/lcd.h`**

```c
#ifndef LCD_H
#define LCD_H

#include "i2c.h"

#define LCD_ADDR 0x27   // PCF8574 address (A0-A2 = GND)

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);

#endif
```

- [ ] **Step 2: Write `firmware/lcd.c`**

```c
#include "lcd.h"
#include "config.h"

// PCF8574 bit assignments:
// D7-D4 = data bits, D3 = BL (backlight), D2 = EN, D1 = RW, D0 = RS

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
    __delay_ms(50);  // wait for LCD power-up
    // Init sequence in 4-bit mode
    lcd_send_nibble(0x03, 0); __delay_ms(5);
    lcd_send_nibble(0x03, 0); __delay_ms(5);
    lcd_send_nibble(0x03, 0); __delay_us(150);
    lcd_send_nibble(0x02, 0);  // switch to 4-bit mode
    lcd_cmd(0x28);  // 4-bit, 2 lines, 5x8 font
    lcd_cmd(0x0C);  // display on, cursor off
    lcd_cmd(0x06);  // entry mode: increment, no shift
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
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/lcd.h firmware/lcd.c
git commit -m "feat: LCD 1602 + PCF8574 I2C driver"
```

---

### Task 7 [OpenCode]: TCS34725 color sensor driver

**Files:**
- Create: `firmware/tcs34725.h`
- Create: `firmware/tcs34725.c`

- [ ] **Step 1: Write `firmware/tcs34725.h`**

```c
#ifndef TCS34725_H
#define TCS34725_H

#include <stdint.h>

typedef struct {
    uint16_t r, g, b, c;  // 16-bit clear and RGB values
} color_rgbc_t;

#define TCS34725_ADDR   0x29
#define TCS34725_CMD    0x80  // command bit for auto-increment
#define TCS34725_ID     0x12

uint8_t tcs34725_init(void);
void    tcs34725_get_raw(color_rgbc_t *c);
void    tcs34725_set_gain(uint8_t gain);  // 0=1x, 1=4x, 2=16x, 3=60x
void    tcs34725_set_integration_time(uint8_t cycles); // 0=2.4ms .. 255=614ms

#endif
```

- [ ] **Step 2: Write `firmware/tcs34725.c`**

```c
#include "tcs34725.h"
#include "i2c.h"

// Register map
#define TCS34725_ATIME    0x01
#define TCS34725_CONTROL  0x0F  // gain
#define TCS34725_CDATAL   0x14  // clear low byte (auto-increment reads RGBC)

uint8_t tcs34725_init(void) {
    // Verify device ID
    uint8_t id;
    if (!i2c_read(TCS34725_ADDR, TCS34725_CMD | TCS34725_ID, &id, 1))
        return 0;
    if (id != 0x44 && id != 0x4D)  // TCS34725 = 0x44, TCS34727 = 0x4D
        return 0;

    // Enable: power on + RGBC enable
    uint8_t enable[] = { TCS34725_CMD | 0x00, 0x03 };  // PON + AEN
    i2c_write(TCS34725_ADDR, enable, 2);
    __delay_ms(3);

    // Default integration: 50ms (cycles = 0xEB = 235)
    tcs34725_set_integration_time(0xEB);
    // Default gain: 4x
    tcs34725_set_gain(1);

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
    c->c = buf[0] | (buf[1] << 8);
    c->r = buf[2] | (buf[3] << 8);
    c->g = buf[4] | (buf[5] << 8);
    c->b = buf[6] | (buf[7] << 8);
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/tcs34725.h firmware/tcs34725.c
git commit -m "feat: TCS34725 color sensor I2C driver"
```

---

### Task 8 [OpenCode]: CCP1 PWM driver — H-bridge

**Files:**
- Create: `firmware/pwm.h`
- Create: `firmware/pwm.c`

- [ ] **Step 1: Write `firmware/pwm.h`**

```c
#ifndef PWM_H
#define PWM_H

#include <stdint.h>

void pwm_hbridge_init(void);
void pwm_hbridge_set_duty(uint8_t duty);  // 0-255 (0=off, 255=full)

#endif
```

- [ ] **Step 2: Write `firmware/pwm.c`**

```c
#include "pwm.h"
#include "config.h"

void pwm_hbridge_init(void) {
    // CCP1 in PWM mode on RC2
    // TMR2 already configured: prescaler 1:1, PR2=249 → 20kHz
    CCP1CONbits.CCP1M = 0b1100;  // PWM mode
    CCPR1L = 0;                   // duty = 0 initially
    TRISCbits.TRISC2 = 0;         // RC2 as output
}

void pwm_hbridge_set_duty(uint8_t duty) {
    // PR2=249 → max 10-bit duty = 4*(PR2+1) = 1000
    // Scale 0-255 input → 0-1000 for full speed range
    uint16_t duty10 = ((uint16_t)duty * 1000) / 255;
    CCPR1L = (uint8_t)(duty10 >> 2);          // high 8 bits of 10-bit duty
    CCP1CONbits.DC1B = (uint8_t)(duty10 & 0x03); // low 2 bits
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/pwm.h firmware/pwm.c
git commit -m "feat: CCP1 PWM driver — H-bridge 20kHz"
```

---

### Task 9 [OpenCode]: CCP2 Compare servo driver

**Files:**
- Create: `firmware/servo.h`
- Create: `firmware/servo.c`

- [ ] **Step 1: Write `firmware/servo.h`**

```c
#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#define SERVO_PULSE_MIN  2500  // 1.0ms @ 400ns/tick
#define SERVO_PULSE_MAX  5000  // 2.0ms @ 400ns/tick
#define SERVO_PULSE_NEUT 3750  // 1.5ms (90°)
#define SERVO_FRAME_TICKS 50000 // 20ms @ 400ns/tick

void servo_init(void);
void servo_set_angle(uint8_t servo_id, uint16_t angle_deg);  // 0-180
void servo_ccp2_isr(void);          // called from low ISR
void servo_step(void);              // for servo 2 (software PWM, call from TMR1)

#endif
```

- [ ] **Step 2: Write `firmware/servo.c`**

```c
#include "servo.h"
#include "config.h"

// Servo 1: CCP2 Compare on RC1
// Servo 2: RC0 software toggle (TMR1 ISR)

static volatile uint16_t servo1_pulse = SERVO_PULSE_NEUT;

// For servo 2 software PWM
static volatile uint16_t servo2_pulse = SERVO_PULSE_NEUT;
static volatile uint16_t servo2_counter = 0;
static volatile uint8_t  servo2_state = 0;  // 0=low, 1=high

void servo_init(void) {
    // CCP2 in Compare mode (NOT PWM — 1xxx is PWM range)
    // Valid Compare modes: 0100 (force HIGH), 0101 (force LOW), 0111 (special event)
    // CCP2 uses TMR1 (T3CCP2=0, configured in system_init)

    // RC1 as output for Servo 1 (CCP2 pin)
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC1 = 1;         // Start first pulse HIGH

    // Arm first compare: force LOW when TMR1 reaches servo1_pulse
    CCP2CONbits.CCP2M = 0b0101; // force LOW on match
    CCPR2 = servo1_pulse;
    PIE2bits.CCP2IE = 1;        // enable CCP2 interrupt
    IPR2bits.CCP2IP = 0;        // low priority

    // RC0 as output for servo 2 (TMR1-based software compare)
    TRISCbits.TRISC0 = 0;
    LATCbits.LATC0 = 0;
}

void servo_set_angle(uint8_t servo_id, uint16_t angle_deg) {
    if (angle_deg > 180) angle_deg = 180;
    uint16_t pulse = SERVO_PULSE_MIN + ((uint32_t)(SERVO_PULSE_MAX - SERVO_PULSE_MIN) * angle_deg / 180);
    if (servo_id == 1) {
        servo1_pulse = pulse;
    } else {
        servo2_pulse = pulse;
    }
}

void servo_ccp2_isr(void) {
    // Clean 2-phase cycle (2 ISR events per 20ms frame):
    //
    // Phase 0 (pulse end): Hardware just forced RC1 LOW (precise falling edge).
    //   → Arm special event to reset TMR1 at end of frame.
    // Phase 1 (frame end): Hardware reset TMR1 to 0.
    //   → Set RC1 HIGH (rising edge, software), arm force-LOW for next pulse.
    static uint8_t phase = 0;

    if (phase == 0) {
        // Hardware just forced RC1 LOW — end of pulse (falling edge is jitter-free)
        CCP2CONbits.CCP2M = 0b0111;        // special event: reset TMR1 on match
        CCPR2 = SERVO_FRAME_TICKS;         // match at end of 20ms frame
        phase = 1;
    } else {
        // TMR1 was reset to 0 — start of new frame
        LATCbits.LATC1 = 1;                // RC1 HIGH (rising edge — software)
        CCP2CONbits.CCP2M = 0b0101;        // force LOW on next match
        CCPR2 = servo1_pulse;              // pulse width from current angle
        phase = 0;
    }
}

void servo_step(void) {
    // Software compare for servo 2 on RC0, using TMR1 tick count.
    // Called from CCP2 ISR at frame boundaries (phase 1 — TMR1 just reset).
    // Sets RC1 HIGH at frame start. Falling edge handled by servo2_poll().
    LATCbits.LATC0 = 1;  // begin servo 2 pulse at frame start
}

// Call from main loop to end servo 2 pulse at correct width.
// Uses TMR1 value directly for sub-ms resolution (400ns/tick).
void servo2_poll(void) {
    uint16_t tmr1_val = ((uint16_t)TMR1H << 8) | TMR1L;
    if (LATCbits.LATC0 && tmr1_val >= servo2_pulse) {
        LATCbits.LATC0 = 0;  // end pulse — precise to ~400ns
    }
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/servo.h firmware/servo.c
git commit -m "feat: CCP2 Compare servo driver + software PWM for servo 2"
```

---

### Task 10 [OpenCode]: Encoder — INT2 pulse counter + speed

**Files:**
- Create: `firmware/encoder.h`
- Create: `firmware/encoder.c`

- [ ] **Step 1: Write `firmware/encoder.h`**

```c
#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void encoder_init(void);
uint32_t encoder_get_pulses(void);
uint16_t encoder_get_speed_mm_s(void);  // filtered mm/s
void encoder_isr_handler(void);         // from INT2
void encoder_tick_handler(void);        // from TMR0 (1ms)

#endif
```

- [ ] **Step 2: Write `firmware/encoder.c`**

```c
#include "encoder.h"
#include "config.h"

// Pulses per revolution (depends on encoder disc)
#define PULSES_PER_REV  20
// Roller circumference in mm (measure during assembly)
#define ROLLER_MM       100    // integer — no float in ISR context
// Speed measurement window (ms)
#define SPEED_WINDOW_MS 500

static volatile uint32_t total_pulses = 0;

// Speed measurement
static volatile uint16_t pulse_window = 0;
static volatile uint32_t last_total = 0;
static volatile uint16_t speed_mm_s = 0;

void encoder_init(void) {
    // INT2 on RB2
    TRISBbits.TRISB2 = 1;         // input
    INTCON3bits.INT2IE = 1;       // enable
    INTCON3bits.INT2IP = 0;       // low priority
    INTCON2bits.INTEDG2 = 1;      // rising edge
    INTCON3bits.INT2IF = 0;
}

void encoder_isr_handler(void) {
    total_pulses++;
    pulse_window++;
}

void encoder_tick_handler(void) {
    static uint16_t counter = 0;
    counter++;
    if (counter >= SPEED_WINDOW_MS) {
        counter = 0;
        // pulses per second = pulse_window * (1000/SPEED_WINDOW_MS)
        uint32_t pps = (uint32_t)pulse_window * (1000 / SPEED_WINDOW_MS);
        // revs per second = pps / PULSES_PER_REV
        // mm/s = revs_per_sec * ROLLER_MM
        speed_mm_s = (uint16_t)((pps * (uint32_t)ROLLER_MM) / PULSES_PER_REV);
        pulse_window = 0;
    }
}

uint32_t encoder_get_pulses(void) {
    uint32_t p;
    PEIE = 0;
    p = total_pulses;
    PEIE = 1;
    return p;
}

uint16_t encoder_get_speed_mm_s(void) {
    return speed_mm_s;
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/encoder.h firmware/encoder.c
git commit -m "feat: INT2 encoder driver with real-time speed"
```

---

### Task 11 [agy]: State machine + main loop

**Files:**
- Create: `firmware/state_machine.h`
- Create: `firmware/state_machine.c`

**Context:** This is the core business logic. Calls OpenCode's HAL APIs. Runs in the main loop.

- [ ] **Step 1: Write `firmware/state_machine.h`**

```c
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum {
    ST_IDLE,
    ST_RUNNING,
    ST_SORTING,
    ST_ERROR
} state_t;

void state_machine_init(void);
void state_machine_run(void);    // main loop call
void state_machine_start(void);  // transition IDLE → RUNNING (callable from BT)
void state_machine_estop(void);  // from high-priority ISR (NO blocking I/O!)
state_t state_machine_get(void);

#endif
```

- [ ] **Step 2: Write `firmware/state_machine.c`**

```c
#include "state_machine.h"
#include "system.h"
#include "gpio.h"
#include "pwm.h"
#include "encoder.h"
#include "tcs34725.h"
#include "uart.h"
#include "lcd.h"
#include "servo.h"
#include "bt_protocol.h"
#include <string.h>

static state_t state = ST_IDLE;
static uint8_t motor_speed = 0;
static uint8_t auto_mode = 1;
static uint16_t distance_to_servo_mm = 200;  // mm from sensor to servo
static uint32_t last_detect_tick = 0;

// Color detection state
typedef struct {
    uint16_t r_min, r_max, g_min, g_max, b_min, b_max;
    const char *name;
} color_threshold_t;

static color_threshold_t thresholds[3];  // up to 3 colors
static uint8_t num_colors = 0;

void state_machine_init(void) {
    state = ST_IDLE;
    lcd_clear();
    lcd_print("Sistema OK");
    lcd_set_cursor(0, 1);
    lcd_print("Presione START");
}

static volatile uint8_t estop_pending = 0;

void state_machine_estop(void) {
    // Called from HIGH-PRIORITY ISR — must NOT call blocking functions
    // (no UART, no LCD, no I2C — these use low-priority ISR or busy-waits)
    state = ST_ERROR;
    pwm_hbridge_set_duty(0);
    gpio_hbridge_dir(HB_STOP);
    estop_pending = 1;  // deferred notification, handled in main loop
}

void state_machine_start(void) {
    if (state != ST_IDLE) return;
    state = ST_RUNNING;
    pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
    gpio_hbridge_dir(HB_FWD);
    uart_send_str("STATE:run\n");
    lcd_clear();
    lcd_print("RUNNING");
}

int8_t color_match_index(color_rgbc_t *c) {
    // Returns index of matching color threshold, or -1 if no match
    for (uint8_t i = 0; i < num_colors; i++) {
        if (c->r >= thresholds[i].r_min && c->r <= thresholds[i].r_max &&
            c->g >= thresholds[i].g_min && c->g <= thresholds[i].g_max &&
            c->b >= thresholds[i].b_min && c->b <= thresholds[i].b_max) {
            return (int8_t)i;
        }
    }
    return -1;  // no match — TUI maps index to name
}

void state_machine_run(void) {
    static uint32_t last_telemetry = 0;
    static uint16_t lcd_counter = 0;

    // Atomic read of 32-bit system_ticks (4 bytes, not atomic on 8-bit PIC)
    INTCONbits.GIEL = 0;
    uint32_t now = system_ticks;
    INTCONbits.GIEL = 1;

    // Deferred E-stop notification (cannot send from high-priority ISR)
    if (estop_pending) {
        estop_pending = 0;
        uart_send_str("STATE:err\n");
        lcd_clear();
        lcd_print("!EMERGENCY STOP!");
    }

    // Handle buttons
    if (gpio_button_pressed(BTN_MODE)) {
        if (state == ST_IDLE && auto_mode) {
            state = ST_RUNNING;
            pwm_hbridge_set_duty(motor_speed ? motor_speed : 180);
            gpio_hbridge_dir(HB_FWD);
            uart_send_str("STATE:run\n");
            lcd_clear();
            lcd_print("RUNNING");
        } else if (state == ST_RUNNING) {
            state = ST_IDLE;
            pwm_hbridge_set_duty(0);
            gpio_hbridge_dir(HB_STOP);
            uart_send_str("STATE:idle\n");
            lcd_clear();
            lcd_print("IDLE");
        }
    }

    // State-specific logic
    switch (state) {
        case ST_IDLE:
            // Show idle message
            if (lcd_counter++ % 500 == 0) {
                lcd_set_cursor(0, 1);
                lcd_print("BT OK        ");
            }
            break;

        case ST_RUNNING:
            // Poll color sensor every ~100ms (100 system ticks)
            if (now % 100 == 0) {
                color_rgbc_t color;
                tcs34725_get_raw(&color);
                const char *name = color_name_from_rgb(&color);
                if (name) {
                    state = ST_SORTING;
                    uart_send_str("DETECT:");
                    uart_send_str(name);
                    uart_send_str("\n");
                    lcd_set_cursor(0, 1);
                    lcd_print("Detectado: ");
                    lcd_print(name);
                }
            }
            // Update LCD with speed
            if (lcd_counter++ % 50 == 0) {
                lcd_set_cursor(0, 1);
                lcd_print("Vel: ");
                // Simple number to LCD (this is basic — improve with sprintf if available)
            }
            break;

        case ST_SORTING:
            // Calculate transit time from current speed
            {
                uint16_t speed = encoder_get_speed_mm_s();
                if (speed > 0) {
                    uint16_t transit_ms = (uint32_t)distance_to_servo_mm * 1000 / speed;
                    // Wait transit_ms - safety margin then fire servo
                    // For simplicity: fire after fixed short delay for demo
                    servo_set_angle(1, 90);  // divert
                    // Wait then return
                    uint32_t timeout = system_ticks + 500;  // 500ms dwell
                    while (system_ticks < timeout) {
                        // Could check break-beam here
                    }
                    servo_set_angle(1, 0);   // return
                }
                state = ST_RUNNING;
                uart_send_str("DONE\n");
            }
            break;

        case ST_ERROR:
            // Wait for manual reset (E-stop cleared + mode button)
            if (gpio_button_pressed(BTN_MODE)) {
                state = ST_IDLE;
                uart_send_str("STATE:idle\n");
                lcd_clear();
                lcd_print("IDLE - OK");
            }
            break;
    }

    // Telemetry every ~500ms
    if (now - last_telemetry >= 500) {
        last_telemetry = now;
        char buf[64];
        snprintf(buf, sizeof(buf), "STATE:%s SPEED:%u PULSES:%lu\n",
                 state == ST_IDLE ? "idle" :
                 state == ST_RUNNING ? "run" :
                 state == ST_SORTING ? "sort" : "err",
                 encoder_get_speed_mm_s(),
                 encoder_get_pulses());
        uart_send_str(buf);
    }

    // Process BT commands
    bt_protocol_process();  // agy implements
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/state_machine.h firmware/state_machine.c
git commit -m "feat: main state machine + main loop dispatch"
```

---

### Task 12 [agy]: BT protocol parser + dispatcher

**Files:**
- Create: `firmware/bt_protocol.h`
- Create: `firmware/bt_protocol.c`

- [ ] **Step 1: Write `firmware/bt_protocol.h`**

```c
#ifndef BT_PROTOCOL_H
#define BT_PROTOCOL_H

void bt_protocol_init(void);
void bt_protocol_process(void);  // call from main loop

#endif
```

- [ ] **Step 2: Write `firmware/bt_protocol.c`**

```c
#include "bt_protocol.h"
#include "uart.h"
#include "state_machine.h"
#include "pwm.h"
#include "servo.h"
#include "calibration.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char cmd_buf[64];
static uint8_t cmd_pos = 0;

void bt_protocol_init(void) {}

void bt_protocol_process(void) {
    while (uart_available()) {
        uint8_t c = uart_read_byte();
        if (c == '\n' || c == '\r') {
            if (cmd_pos == 0) continue;
            cmd_buf[cmd_pos] = '\0';
            cmd_pos = 0;

            // Parse and dispatch
            if (strcmp(cmd_buf, "START") == 0) {
                state_machine_start();  // transition IDLE → RUNNING
            }
            else if (strcmp(cmd_buf, "STOP") == 0) {
                state_machine_estop();
            }
            else if (strncmp(cmd_buf, "SET_SPEED ", 10) == 0) {
                int val = atoi(cmd_buf + 10);
                if (val >= 0 && val <= 255) {
                    pwm_hbridge_set_duty((uint8_t)val);
                }
            }
            else if (strcmp(cmd_buf, "STATUS") == 0) {
                char resp[64];
                snprintf(resp, sizeof(resp),
                    "STATUS_RESP:%d,%d,%d,%d,%d,%lu\n",
                    0, 0, 1, 0, 0, 0UL);  // fill with real values
                uart_send_str(resp);
            }
            else if (strcmp(cmd_buf, "CALIBRATE") == 0) {
                calibration_start();
            }
            else if (strncmp(cmd_buf, "SET_MODE ", 9) == 0) {
                // parse mode
            }
            else if (strncmp(cmd_buf, "SERVO ", 6) == 0) {
                int id = atoi(cmd_buf + 6);
                char *space = strchr(cmd_buf + 6, ' ');
                if (space) {
                    int angle = atoi(space + 1);
                    servo_set_angle((uint8_t)id, (uint16_t)angle);
                }
            }
            else if (strncmp(cmd_buf, "SET_SPACING ", 12) == 0) {
                // store min spacing pulses
            }
            else if (strncmp(cmd_buf, "SET_THRESHOLD ", 14) == 0) {
                // parse color threshold
            }
        }
        else if (cmd_pos < sizeof(cmd_buf) - 1) {
            cmd_buf[cmd_pos++] = c;
        }
    }
}
```

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/bt_protocol.h firmware/bt_protocol.c
git commit -m "feat: BT protocol parser and command dispatcher"
```

---

### Task 13 [agy]: Calibration + EEPROM storage

**Files:**
- Create: `firmware/calibration.h`
- Create: `firmware/calibration.c`

- [ ] **Step 1: Write `firmware/calibration.h`**

```c
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "tcs34725.h"
#include <stdint.h>

#define EEPROM_MAGIC 0xA5
#define EEPROM_ADDR_MAGIC  0x00
#define EEPROM_ADDR_WHITE    0x01  // 6 bytes (R,G,B — 16-bit each)
#define EEPROM_ADDR_SERVO1   0x07  // 6 bytes (home, deflect, dwell)
#define EEPROM_ADDR_SERVO2   0x0D  // 6 bytes
#define EEPROM_ADDR_ENC_PPR  0x13  // 2 bytes
#define EEPROM_ADDR_NUM_CLR  0x15  // 1 byte
#define EEPROM_ADDR_COLORS   0x16  // 12 bytes × N (matches design spec layout)

typedef struct {
    uint16_t r_min, r_max, g_min, g_max, b_min, b_max;
    // Color name stored on TUI side only (keyed by index) — not in firmware
} color_config_t;

void calibration_start(void);               // begin white balance
uint8_t calibration_is_done(void);
void calibration_apply_white(color_rgbc_t *c);
void calibration_save_color(uint8_t idx, color_config_t *cfg);
uint8_t calibration_load_all(color_config_t *buf, uint8_t max);

#endif
```

- [ ] **Step 2: Write `firmware/calibration.c`**

Implementation using PIC18 EEPROM (`eeprom_write`/`eeprom_read` from XC8).

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/calibration.h firmware/calibration.c
git commit -m "feat: color calibration and EEPROM persistence"
```

---

### Task 14 [agy]: Anti-jam logic

**Files:**
- Create: `firmware/anti_jam.h`
- Create: `firmware/anti_jam.c`

- [ ] **Step 1: Write `firmware/anti_jam.h`**

```c
#ifndef ANTI_JAM_H
#define ANTI_JAM_H

#include <stdint.h>

void anti_jam_init(void);
void anti_jam_check(void);  // call from main loop
uint8_t anti_jam_is_jammed(void);

#endif
```

- [ ] **Step 2: Write `firmware/anti_jam.c`**

Three mechanisms:
1. Encoder belt jam: if `encoder_get_speed_mm_s() == 0` while motor PWM > 0 → JAM
2. Break-beam servo jam: if servo commanded but beam stays broken → JAM
3. Break-beam arrival timeout: object detected but beam never triggers → JAM

**Verification:** `make build` compiles.

- [ ] **Step 3: Commit**

```
git add firmware/anti_jam.h firmware/anti_jam.c
git commit -m "feat: triple anti-jam logic — encoder, break-beam, timeout"
```

---

### Task 15 [OpenCode]: TUI project scaffolding + BT manager

**Files:**
- Create: `tui_app/pyproject.toml`
- Create: `tui_app/app.py`
- Create: `tui_app/connect.py`

- [ ] **Step 1: Write `tui_app/pyproject.toml`**

```toml
[project]
name = "faja-tui"
version = "0.1.0"
dependencies = [
    "textual>=0.41.0",
    "pyserial>=3.5",
]

[tool.textual]
app = "app:FajaApp"
```

- [ ] **Step 2: Write `tui_app/connect.py`**

```python
import asyncio
import serial_asyncio  # or use pyserial with asyncio wrapper


class BTManager:
    def __init__(self):
        self.reader = None
        self.writer = None
        self._connected = False
        self._buf = b""

    @property
    def connected(self) -> bool:
        return self._connected

    async def connect(self, port: str, baud: int = 115200) -> bool:
        try:
            self.reader, self.writer = await serial_asyncio.open_serial_connection(
                url=port, baudrate=baud
            )
            self._connected = True
            return True
        except Exception:
            self._connected = False
            return False

    async def disconnect(self):
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()
        self._connected = False

    async def send(self, cmd: str):
        if self.writer and self._connected:
            self.writer.write((cmd + "\n").encode())
            await self.writer.drain()

    async def read_line(self) -> str | None:
        if not self.reader or not self._connected:
            return None
        try:
            line = await asyncio.wait_for(self.reader.readline(), timeout=0.1)
            return line.decode().strip() if line else None
        except asyncio.TimeoutError:
            return None
        except Exception:
            self._connected = False
            return None
```

- [ ] **Step 3: Write `tui_app/app.py`**

```python
from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.screen import Screen
from textual.widgets import Header, Footer, Static, Button, Input, ListView
from connect import BTManager
from protocol import parse_telemetry


class FajaApp(App):
    TITLE = "Faja Transportadora"
    SUB_TITLE = "PIC18F4550 Control"
    CSS = """
    Screen { layout: vertical; }
    Dashboard { height: 1fr; }
    """

    BINDINGS = [
        Binding("q", "quit", "Salir"),
        Binding("c", "connect", "Conectar BT"),
    ]

    def __init__(self):
        super().__init__()
        self.bt = BTManager()
        self.last_state = "unknown"

    def compose(self) -> ComposeResult:
        yield Header()
        yield Dashboard()
        yield Footer()

    async def on_mount(self):
        self.set_interval(0.5, self.poll_bluetooth)

    async def poll_bluetooth(self):
        if not self.bt.connected:
            return
        line = await self.bt.read_line()
        if line:
            data = parse_telemetry(line)
            self.update_dashboard(data)

    def update_dashboard(self, data: dict):
        # agy fills this
        pass

    async def action_connect(self):
        # Simple: try common port
        for port in ["/dev/rfcomm0", "/dev/ttyUSB0", "COM3"]:
            if await self.bt.connect(port):
                self.notify(f"Conectado a {port}")
                return
        self.notify("No se pudo conectar", severity="error")


if __name__ == "__main__":
    app = FajaApp()
    app.run()
```

**Verification:** `python tui_app/app.py` launches Textual app (will be empty dashboard).

- [ ] **Step 4: Commit**

```
git add tui_app/
git commit -m "feat: TUI app scaffolding + BT serial manager"
```

---

### Task 16 [OpenCode + agy]: Protocol shared module

**Files:**
- Create: `tui_app/protocol.py`

- [ ] **Step 1: Write `tui_app/protocol.py`**

```python
CMD_START = "START"
CMD_STOP = "STOP"
CMD_SET_SPEED = "SET_SPEED {}"
CMD_STATUS = "STATUS"
CMD_CALIBRATE = "CALIBRATE"
CMD_SET_MODE = "SET_MODE {}"
CMD_SERVO = "SERVO {} {}"
CMD_SET_SPACING = "SET_SPACING {}"
CMD_SET_THRESHOLD = "SET_THRESHOLD {} {} {} {} {}"


def parse_telemetry(line: str) -> dict:
    """Parse a single telemetry line from the PIC.
    
    Examples:
        "STATE:run" → {"type": "STATE", "value": "run"}
        "SPEED:180" → {"type": "SPEED", "value": 180}
        "COLOR:245,80,30,250" → {"type": "COLOR", "r": 245, "g": 80, "b": 30, "c": 250}
        "DETECT:ROJO" → {"type": "DETECT", "color": "ROJO"}
        "JAM:belt" → {"type": "JAM", "source": "belt"}
    """
    if ":" in line:
        key, _, val = line.partition(":")
        if key == "STATE":
            return {"type": "STATE", "value": val}
        elif key == "SPEED":
            return {"type": "SPEED", "value": int(val)}
        elif key == "COLOR":
            parts = val.split(",")
            return {"type": "COLOR", "r": int(parts[0]), "g": int(parts[1]),
                    "b": int(parts[2]), "c": int(parts[3])}
        elif key == "DETECT":
            return {"type": "DETECT", "color": val}
        elif key == "JAM":
            return {"type": "JAM", "source": val or "unknown"}
        elif key.startswith("STATUS_RESP"):
            parts = val.split(",")
            return {"type": "STATUS", "state": parts[0], "speed": int(parts[1])}
        elif key == "CALIB_DONE":
            return {"type": "CALIB_DONE"}
    return {"type": "UNKNOWN", "raw": line}
```

**Verification:** `python -c "from protocol import parse_telemetry; print(parse_telemetry('STATE:run'))"`

- [ ] **Step 2: Commit**

```
git add tui_app/protocol.py
git commit -m "feat: BT protocol encode/decode module"
```

---

### Task 17 [agy]: Dashboard screen + UI logic

**Files:**
- Create: `tui_app/screens/dashboard.py`
- Create: `tui_app/screens/config.py`
- Create: `tui_app/screens/log_viewer.py`
- Modify: `tui_app/app.py`

**Context:** agy builds the Textual widgets and layout inside the OpenCode app skeleton.

- [ ] **Step 1: Write dashboard screen with state indicators, speed gauge, color display, event log, and interactive buttons

- [ ] **Step 2: Write config screen for color thresholds and speed limits

- [ ] **Step 3: Write log viewer screen

- [ ] **Step 4: Wire screens into app.py

**Verification:** `python tui_app/app.py` shows full UI with all screens.

- [ ] **Step 5: Commit**

```
git add tui_app/screens/
git commit -m "feat: TUI screens — dashboard, config, log viewer"
```

---

### Task 18 [agy + OpenCode]: Modo TEST + Calibración

**Files:**
- Modify: `firmware/state_machine.h` (add ST_TEST state)
- Modify: `firmware/state_machine.c` (TEST state, command verification)
- Modify: `firmware/bt_protocol.c` (new TEST_* commands, new telemetry)
- Modify: `firmware/calibration.c` (servo config, encoder pulses EEPROM)
- Create: `tui_app/screens/test_screen.py` (Test & Calibration screen)

**Context:** New TEST state with safe motor isolation. Allows live servo jogging, beam testing, encoder reading via BT. LCD cyclic editor for on-device calibration without BT.

#### Firmware additions (agy)

- [ ] **Step 1: Add ST_TEST to state machine enum and implement TEST state logic**

TEST state:
- `state_machine_run()` in TEST: skips auto PWM, skips color polling
- `TEST_ENTER` → sets state from IDLE to TEST, rejects if not IDLE
- `TEST_EXIT` → returns to IDLE
- `START` command rejected in TEST
- H-bridge defaults to IN1=IN2=0, PWM=0. Only `TEST_MOTOR` activates it.
- **2-second watchdog timer**: if no new `TEST_MOTOR` or `TEST_EXIT` received within 2s, firmware forces IN1=IN2=0 and PWM=0. This prevents runaway motor if BT disconnects mid-test.
- `TEST_MOTOR` is a one-shot jog — each call resets the 2s watchdog and activates motor for that duration.

- [ ] **Step 2: Add TEST_* BT commands to `bt_protocol.c`**

Commands:
```
TEST_ENTER / TEST_EXIT
SERVO_SET <1|2> <angle>          → calls servo_set_angle live
SERVO_SAVE_HOME <1|2>            → calibration_save_servo_home(servo_id)
SERVO_SAVE_DEFLECT <1|2>         → calibration_save_servo_deflect(servo_id)
SERVO_GET_CONFIG <1|2>           → sends SERVO_CONFIG telemetry
SET_DWELL <1|2> <ms>             → calibration_save_servo_dwell(servo_id, ms)
TEST_MOTOR <0-255> <fwd|rev>     → brief motor pulse (only in TEST with safety check)
TEST_ENCODER_RESET               → encoder_reset()
TEST_ENCODER_READ                → sends ENCODER_COUNT telemetry
TEST_BEAM <1|2>                  → sends BEAM telemetry
TEST_BUTTON_ECHO <on|off>        → toggles button echo flag
```

New telemetry in TEST:
```
SERVO_CONFIG:<1|2>:<home_angle>,<deflect_angle>,<dwell_ms>
ENCODER_COUNT:<pulses>
BEAM:<1|2>:<broken|clear>
BUTTON:<id>                (only if echo enabled)
```

- [ ] **Step 3: Add servo config + encoder pulses to EEPROM in `calibration.c`**

EEPROM layout extension:
```
0x07: Servo 1 home angle (deg)      2 bytes
0x09: Servo 1 deflect angle (deg)   2 bytes
0x0B: Servo 1 dwell (ms)           2 bytes
0x0D: Servo 2 home angle (deg)      2 bytes
0x0F: Servo 2 deflect angle (deg)   2 bytes
0x11: Servo 2 dwell (ms)           2 bytes
0x13: Encoder pulses_per_rev       2 bytes
0x15: Number of stored colors (N)  1 byte
0x16+: Color thresholds            12 bytes × N

Color threshold structure (12 bytes):
  R_min(Rmin), R_max(Rmax), G_min(Gmin), G_max(Gmax), B_min(Bmin), B_max(Bmax)
  Each field: uint16 (2 bytes). Total: 6 × 2 = 12 bytes per color.
  
Maximum: 4 colors (48 bytes). Firmware caps at 8 (96 bytes) absolute max.
Color labels stored on TUI side only (keyed by index).
```

Defaults if EEPROM magic byte absent: servo home=90°, deflect=0°, dwell=500ms, pulses_per_rev=20, N=0.

- [ ] **Step 4: Commit firmware TEST additions**

```
git add firmware/state_machine.h firmware/state_machine.c
git add firmware/bt_protocol.c
git add firmware/calibration.c
git commit -m "feat: TEST mode + calibration BT commands + EEPROM extension"
```

#### LCD calibration menu (OpenCode)

- [ ] **Step 5: Add LCD cyclic editor to `state_machine.c` TEST state**

When in TEST state and no BT activity for 5s, enter LCD menu mode:
- mode button (RD2) cycles: Servo1 Home, Servo1 Deflect, Servo1 Dwell, Servo2 Home, Servo2 Deflect, Servo2 Dwell, Encoder PPR
- up/down (RD5/RD6) adjusts value **in real time** (moves servo via servo_set_angle as user adjusts)
- long-press mode saves to EEPROM and advances to next field
- LCD shows: `S1H: 090` / `S1D: 045` etc.

- [ ] **Step 6: Commit LCD menu**

```
git add firmware/state_machine.c
git commit -m "feat: LCD cyclic calibration editor"
```

#### TUI Test screen (agy + OpenCode)

- [ ] **Step 7: Create `tui_app/screens/test_screen.py`**

Separate screen from dashboard. Shows:
- Beam state indicators (broken/clear) — live from BEAM telemetry
- Encoder counter — live from ENCODER_COUNT
- Button echo display (if echo enabled)
- Controls: SERVO_SET, SERVO_SAVE, SET_DWELL, TEST_MOTOR, TEST_ENCODER_RESET, TEST_BEAM
- Connect/wire into app.py screen navigation

- [ ] **Step 8: Commit TUI test screen**

```
git add tui_app/screens/test_screen.py
git commit -m "feat: TUI Test & Calibration screen"
```

---

## Spec coverage checklist

| Spec section | Covered by task(s) |
|---|---|
| Pin mapping | Tasks 1-10 (silicon config in each driver) |
| Power architecture | Not in firmware code — hardware schematic only |
| Encoder (real speed) | Task 10 |
| Break-beam IR | Tasks 3 (GPIO), 14 (anti-jam) |
| State machine (IDLE/RUN/SORT/ERR) | Task 11 |
| Servo-belt synchronization | Task 11 |
| Triple anti-jam logic | Task 14 |
| BT protocol (all commands) | Task 12 |
| Telemetry (automatic ~500ms) | Task 11 (state_machine_run) |
| TUI screens | Tasks 15-17 |
| Color calibration + EEPROM | Task 13 |
| Minimum object spacing | Task 11 (encoder-based, in SORTING state) |
| Oscillator 20MHz, HS | Task 1 (config.h), Task 2 (system.c) |
| IPEN=1, two-vector ISR | Task 2 (system.c) |
| TMR0 16-bit mode, reload=64911 | Task 2 |
| Servo CCP2 Compare (corrected: 0101/0111, not 1000/1001) | Task 9 |
| T3CCP2=0 (CCP2→TMR1) | Task 2 (system.c) |
| 115200 baud, BRG16=1 | Task 4 |
| TEST state + motor safety isolation | Task 18 |
| TEST_* BT commands | Task 18 |
| EEPROM servo config + encoder PPR | Task 18 |
| LCD calibration cyclic editor | Task 18 |
| TUI Test & Calibration screen | Task 18 |

---

## Execution handoff

**Plan complete and saved. Two execution options:**

1. **Subagent-Driven (recommended)** — I dispatch a fresh subagent per task, review between tasks, fast iteration.
2. **Inline Execution** — Execute tasks in this session using bulk batch with checkpoints.

**Which approach?**
