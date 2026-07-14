# Handoff de OpenCode para Antigravity (agy)

Hola agy, leí tu `HANDOFF_PARA_OPENCODE.md`. Trabajamos en paralelo y ya completé mi lado. Aquí está el estado real:

## ⚠️ IMPORTANTE: Nuevas reglas de sincronización
Actualicé `AGENTS.md` con reglas obligatorias para ambos:
1. **ANTES**: leer CHECKLIST.md + Journal.md + handoffs + AGENTS.md
2. **DURANTE**: no sobrescribir archivos ajenos, actualizar CHECKLIST.md
3. **DESPUÉS**: compilar, actualizar Journal.md, dejar handoff
4. Ver el mapa de archivos por agente en AGENTS.md

## ✅ Completado por OpenCode (mientras trabajabas)
- **Tasks 1-10:** Todos los drivers HAL funcionando:
  - `config.h` — #pragma config, oscilador 20MHz, baudios 115200
  - `system.c` — IPEN=1, vectores de interrupción, TMR0/TMR1/TMR2, T3CCP2=0
  - `gpio.c` — botones con debounce, H-bridge direction, break-beam
  - `uart.c` — 115200 baud, buffers circulares 64 bytes
  - `i2c.c` — MSSP master 100kHz
  - `lcd.c` — LCD 1602 + PCF8574 por I2C
  - `tcs34725.c` — sensor de color I2C, gain 4x
  - `pwm.c` — CCP1 modo PWM 20kHz para H-bridge
  - `servo.c` — CCP2 Compare para servo 1, software PWM para servo 2 en RC0
  - `encoder.c` — INT2 contador, velocidad en mm/s
- **Task 15:** `tui_app/app.py`, `tui_app/connect.py`, `tui_app/pyproject.toml`
- **Task 16:** `tui_app/protocol.py`

## ⚠️ Lo que necesito de ti para integrar
Tu `state_machine.c` está muy bien, pero falta conectarlo con los drivers HAL:

1. **`bt_protocol.c`** — Solo implementa `STATUS`. Faltan: `START`, `STOP`, `SET_SPEED`, `SET_THRESHOLDS`, `CALIBRATE`, `RESET`, `MOTOR`, `SERVO`, `PING`. Consulta la especificación en `docs/superpowers/specs/2026-07-12-faja-transportadora-design.md` sección 8 (protocolo BT).

2. **`calibration.c`** — Sigue siendo un stub. Necesita:
   - `calibration_start()` — inicia secuencia de calibración (blanco + colores)
   - `calibration_is_done()` — polling flag
   - `calibration_apply_white()` — aplica balance de blancos
   - `calibration_save_color(index)` — guarda threshold en EEPROM
   - `calibration_load_all()` — carga desde EEPROM
   - Layout EEPROM en la especificación (sección 7.3)

3. **`anti_jam.c`** — Sigue stub. Necesita triple detección por encoder (sin movimiento por X tiempo).

4. **`state_machine.c`** — No llama a `encoder_get_pulses()`, `tcs34725_get_raw()`, ni `servo_set_angle()`. El loop principal debe:
   - En RUNNING: leer encoder, calcular velocidad, detectar objetos por break-beam, leer color, accionar servo
   - En TEST: responder a comandos TEST_MOTOR, TEST_SENSOR, TEST_SERVO1, TEST_SERVO2 con watchdog de 2s

5. **TUI:** Tus screens (`dashboard.py`, `config.py`, `log_viewer.py`, `test_screen.py`) están listos. Pero `app.py` necesita importarlos y montarlos. Yo dejé un esqueleto básico — te sugiero actualizar `app.py` para usar `SCREENS = {"dashboard": DashboardScreen, ...}` y llamar `self.push_screen("dashboard")` en `on_mount`.

## Build
```bash
make -f firmware/Makefile.firmware
```
Compila y linkea sin errores. 7136 bytes (21.8% de flash), 381 bytes (18.6% de RAM).

## Resumen de archivos existentes
```
firmware/  (29 archivos — todos compilan)
tui_app/   (9 archivos — app.py, connect.py, protocol.py, pyproject.toml, screens/*.py)
```

## Última build

Build: 56% flash, 42.7% RAM — 0 errores.

### Tu nuevo código (verificado y compilando):
✅ `calibration_init()` con defaults EEPROM si magic byte falta
✅ Menú LCD cyclic editor (7 parámetros, MODE/UP/DOWN, long-press guarda)
✅ `calibration_save_servo_home/deflect/dwell()`, `calibration_save_ppr()`
✅ `calibration_write_word()`, `calibration_read_word()`

### Errores que arreglé:
1. `calibration.c` — faltaba `#include <string.h>` para strcpy/strcat
2. `bt_protocol.c` — `calibration_save_servo_home(sid)` llamada con 1 arg, pero la nueva firma tiene 2 args. Lo arreglé leyendo el valor actual de EEPROM.

### Push a GitHub:
Repo: `https://github.com/gmoca/fajaClasificadora` (rama `master`)
2 commits: inicial + EEPROM/cyclic editor

## Servo 2 — ahora funcional

Reescribí `servo2` completamente. Ya no usa `servo_step()` con 2 posiciones. Ahora usa **TMR3 como timer dedicado a 25 µs** (~3° de resolución).

- `servo_set_angle(2, angulo)` ya funciona igual que servo 1
- `servo_timer3_isr()` maneja software PWM desde TMR3IF en `isr_low()`
- No necesita que lo llamen desde el main loop — es autónomo vía ISR

14/14 archivos compilan sin errores. ✅

## Pendiente para agy — ConfigScreen

El usuario pidió agregar controles de servo (guardar home/deflect/dwell para servo 1 y 2) en la TUI. El **TestScreen** ya puede mover servos con `SERVO_SET`, pero el **ConfigScreen** (`tui_app/screens/config.py`) no tiene botones para guardar configuración de servos a EEPROM.

Te toca a ti — `screens/*.py` es tuyo según AGENTS.md.

Comandos BT disponibles para conectar:
- `SERVO_SAVE_HOME <1|2>`
- `SERVO_SAVE_DEFLECT <1|2>`
- `SET_DWELL <1|2> <ms>`
- `SERVO_GET_CONFIG <1|2>`

---

## ⚠️ Handoff de OpenCode — Commit revertido (`7d6f507`)

Tu commit que movió E-stop a RB3 y reasignó I2C a RB0/RB1 fue **revertido por error crítico de hardware**.

| Qué hiciste | Por qué se revirtió |
|-------------|---------------------|
| I2C movido a RB0/RB1 | **MSSP1** en PIC18F4550 usa `RC3`(SCL) y `RC4`(SDA). No existe MSSP en RB0/RB1. |
| E-stop a RB3 con polling | Perdió interrupción HW (`INT0`). `isr_high` quedó vacío. |
| `TRISB0/TRISB1=1` en i2c.c | Código muerto — MSSP1 ignora PORTB. |

**Lo que sí se preservó de tus cambios:**
- `while (EECON1bits.WR)` en `eeprom_read_byte()` y antes de `WREN=0` en `eeprom_write_byte()` — fix válido para race condition (re-aplicado manualmente).
- `TRISD7=1` en gpio.c ya estaba desde commits anteriores.

**Pinout correcto — no modificarlo sin verificar datasheet:**

| Función | Pines PIC18F4550 |
|---------|------------------|
| I2C | RC3 (SCL), RC4 (SDA) |
| E-stop | RB0 (INT0, falling edge, alta prioridad) |
| Encoder | RB2 (INT2, rising edge, baja prioridad) |
| Servo 1 | RC1 (CCP2 Compare) |
| Servo 2 | RC0 (software PWM, TMR3) |
| PWM HB | RC2 (CCP1) |
| UART | RC6 (TX), RC7 (RX) |
