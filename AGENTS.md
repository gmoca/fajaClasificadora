# pryMicroFaja.X

**PIC18F4550** + **XC8 v3.10** (C99), MPLAB X v6.30. Firmware setup completo, **56.0% flash, 42.7% RAM**.  
TUI Python con Textual (4 pantallas). Repo: `https://github.com/gmoca/fajaClasificadora` (master).

## Dual-agent workflow (OpenCode + agy)

| Before work | During | After work |
|-------------|--------|------------|
| Read `CHECKLIST.md`, `Journal.md`, both `HANDOFF_*` files, `AGENTS.md` | Respect file ownership; mark changes with `/* agy: */` or `// OpenCode:` when modifying foreign files; update `CHECKLIST.md` | Run `make -f firmware/Makefile.firmware`; update `Journal.md`; leave handoff in `HANDOFF_PARA_ANTIGRAVITY.md` (if OpenCode) or `HANDOFF_PARA_OPENCODE.md` (if agy). No commits without permission |

## File ownership

| Owner | Files |
|-------|-------|
| **OpenCode** | `firmware/config.h`, `system.{c,h}`, `gpio.{c,h}`, `uart.{c,h}`, `i2c.{c,h}`, `lcd.{c,h}`, `tcs34725.{c,h}`, `pwm.{c,h}`, `servo.{c,h}`, `encoder.{c,h}`, `main.c`, `Makefile.firmware`; `tui_app/app.py`, `connect.py` |
| **agy** | `firmware/state_machine.{c,h}`, `bt_protocol.{c,h}`, `calibration.{c,h}`, `anti_jam.{c,h}`; `tui_app/screens/*.py` |
| **Shared** | `tui_app/protocol.py` |

## Build

| Target | Command | Output |
|--------|---------|--------|
| Firmware (fast) | `make -f firmware/Makefile.firmware` | `dist/default/production/pryMicroFaja.X.production.hex` |
| MPLAB X | `make build` / `make clean` / `make clobber` | same `.hex` |
| TUI | `cd tui_app && pip install -e . && python app.py` | — |

Build requires MPLAB X v6.30 + XC8 v3.10 at `C:\Program Files\Microchip\...`.  
`nbproject/Makefile-local-default.mk` holds per-host tool paths — must exist before `make build`.  
Source files tracked in `nbproject/configurations.xml` (add files there or via IDE).

## XC8 v3.10 quirks (clang-based)

- **`eeprom_write()`/`eeprom_read()`** — removed. Use direct EEDATA/EEADR/EECON1/EECON2 register access.
- **`utoa()`/`ultoa()`** — removed (were HI-TECH C extensions). Use custom `u16_to_str`/`u32_to_str` to avoid `sprintf` flash cost.
- **T3CCP2 = 0** — CCP2 defaults to TMR3 on PIC18F4550; force `T3CONbits.T3CCP2 = 0` for TMR1 binding (required for servo 1).
- **CCP2 modes for servo** — use `0b0101` (force LOW) and `0b0111` (special event + force HIGH), NOT PWM modes `0b1000`/`0b1001`.
- **Stack** — `-mstack=compiled:auto:auto:auto` eliminates hardware stack need, saves RAM.
- **EEPROM race condition** — always `while (EECON1bits.WR);` before read or before clearing WREN after write (~4ms cycle).

## PIC18F4550 Pinout (OBLIGATORIO — NO MODIFICAR)

⚠️ **El PIC18F4550 NO es igual al PIC18F4520 ni al PIC16F877A.** En este chip, el USB ocupa `RC4`/`RC5`, y el MSSP (I2C) se reubicó a `PORTB`. Verificado en datasheet DS39632E y confirmado en simulación Proteus.

| Función | Pin | Registro / Nota |
|---------|-----|-----------------|
| **I2C SDA** | **RB0** (pin 33) | `TRISB0=1` en `i2c.c` |
| **I2C SCL** | **RB1** (pin 34) | `TRISB1=1` en `i2c.c` |
| **E-stop** | **RB3** (pin 36) | Polling cada 1ms en TMR0 ISR (`system.c`) |
| Encoder | RB2 (pin 35) | INT2, rising edge, baja prioridad |
| Servo 1 | RC1 (pin 16) | CCP2 Compare mode |
| Servo 2 | RC0 (pin 15) | Software PWM vía TMR3 ISR |
| PWM H-bridge | RC2 (pin 17) | CCP1 PWM, 20kHz |
| UART TX | RC6 (pin 25) | → HC-05 RX (divisor 1kΩ+2.2kΩ) |
| UART RX | RC7 (pin 26) | ← HC-05 TX |
| USB D- | RC4 (pin 23) | Transceptor USB (NO usar para I2C) |
| USB D+ | RC5 (pin 24) | Transceptor USB (NO usar para I2C) |

## Conventions

- C99 (`-std=c99`), dwarf-3, `-O0`, COFF+ELF hybrid output, no peripheral library linked.
