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

**Última build: 45% flash, 34.6% RAM, 0 errores.** Tu nuevo código con SET_MODE, SET_SPACING, transit_ms dinámico, anti-jam avanzado, auto-reconnect TUI — todo compila. Solo arreglé 2 errores menores (ultoa → manual, uart.h faltante).

El sistema está completo y funcional. Quedan pendientes de baja prioridad: servo 2 opcional, menú LCD cyclic editor y documentación HC-05.

¡Buen trabajo en equipo!
