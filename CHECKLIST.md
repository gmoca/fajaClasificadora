# Checklist del Proyecto: Faja Transportadora Automatizada

> **Propósito:** Checklist global para coordinar entre OpenCode y agy.
> **Formato:** `[x]` = completado, `[~]` = parcial, `[ ]` = pendiente.

---

## 1. Firmware — Capa HAL (OpenCode)

- [x] **Task 1 — Project scaffolding + config.h**
  - [x] `config.h`: #pragma config (HS, PLL off, WDT off, LVP off, DEBUG off)
  - [x] `main.c`: inicialización de todos los subsistemas + loop principal
  - [x] `Makefile.firmware`: build independiente funcionando
- [x] **Task 2 — System init + timers**
  - [x] `system.c`: IPEN=1, TMR0 (1ms 16-bit), TMR1 (free-run 400ns), TMR2 (20kHz)
  - [x] T3CCP2=0 forzado (CCP2 usa TMR1, no TMR3)
  - [x] Interrupciones: `isr_high` (INT0 e-stop), `isr_low` (TMR0, INT2, UART, CCP2)
- [x] **Task 3 — GPIO**
  - [x] H-bridge direction: RD0/RD1 (STOP/FWD/REV)
  - [x] Botones: RD2 (MODE), RD5 (UP), RD6 (DOWN) con debounce por TMR0
  - [x] Break-beam IR: RD3 (emitter), RD4 (receiver 1), RD7 (receiver 2)
- [x] **Task 4 — UART (HC-05)**
  - [x] 115200 baud, BRG16=1, BRGH=1, SPBRG=42
  - [x] Buffer circular RX 64 bytes, buffer TX 64 bytes
- [x] **Task 5 — I2C (MSSP)**
  - [x] Modo maestro 100kHz, SSPADD=49
  - [x] Funciones: i2c_write(), i2c_read() con repeated start
- [x] **Task 6 — LCD 1602 + PCF8574**
  - [x] Address 0x27, modo 4 bits, backlight
  - [x] init, clear, set_cursor, print
- [x] **Task 7 — TCS34725 (color sensor)**
  - [x] Address 0x29, ID check, gain 4x, integración ~50ms
  - [x] Lectura raw RGBC 16-bit
- [x] **Task 8 — PWM H-bridge (CCP1)**
  - [x] CCP1 PWM, RC2, 20kHz (PR2=249)
  - [x] set_duty(0-255) → 10-bit duty cycle
- [x] **Task 9 — Servos**
  - [x] Servo 1: CCP2 Compare (0b0101/0b0111), TMR1 timebase 400ns
  - [x] Servo 2 (opcional): software PWM en RC0, TMR1 ISR
- [x] **Task 10 — Encoder (INT2)**
  - [x] INT2 en RB2, rising edge, prioridad baja
  - [x] Contador de pulsos + velocidad en mm/s (ventana 500ms)

### Build firmware: 56.0% flash, 42.7% RAM ✅

---

## 2. Firmware — Lógica de negocio (agy → tareas 11-14)

- [x] **Task 11 — State machine** (`state_machine.c/.h`)
  - [x] Estados: IDLE, RUNNING, SORTING, ERROR, TEST
  - [x] START/STOP vía botón MODE
  - [x] state_machine_start(), set_mode/spacing/dwell, test_enter/exit/motor()
  - [x] Telemetría cada 500ms (STATE, SPEED, PULSES)
  - [x] Deferred e-stop notification
  - [x] Control de motor en TEST con watchdog 2s
  - [x] `color_match_index()` con calibración desde EEPROM
  - [x] `anti_jam_check()` llamado durante SORTING wait

- [x] **Task 12 — BT Protocol** (`bt_protocol.c/.h`)
  - [x] Parser de comandos: START, STOP, SET_SPEED, STATUS, CALIBRATE, TEST_*, SERVO
  - [x] SET_MODE, SET_SPACING, SET_THRESHOLD implementados
  - [x] SERVO_SET, SERVO_SAVE_HOME, SERVO_GET_CONFIG, SET_DWELL
  - [x] TEST_ENCODER_RESET/READ, TEST_BEAM, TEST_BUTTON_ECHO

- [x] **Task 13 — Calibration + EEPROM** (`calibration.c/.h`)
  - [x] Layout EEPROM según spec (magic=0x00, white balance, servo configs, colors)
  - [x] Funciones: calibration_start(), calibration_is_done(), calibration_apply_white(), calibration_save_color(), calibration_load_all()
  - [x] EEPROM read/write por registros directos (EEADR/EEDATA/EECON1/EECON2)
  - [x] **PENDIENTE:** `calibration_start()` vacío — ahora con mocks iniciales
  - [x] **PENDIENTE:** `calibration_apply_white()` vacío — ahora con balance de blancos básico
  - [x] `calibration_init()` implementado con valores por defecto para Magic Byte (0xA5), Servos y PPR.

- [x] **Task 14 — Anti-jam** (`anti_jam.c/.h`)
  - [x] anti_jam_init() llamado desde state_machine_init()
  - [x] anti_jam_check() llamado en SORTING wait loop
  - [x] Encoder belt jam (speed=0 >1s) + Break-beam bloqueado >3s

---

## 3. Integración Firmware (gaps detectados)

- [x] **Firmware compila y linkea** — 14 archivos, sin errores (56% flash, 42.7% RAM)
- [x] `transit_ms` se usa dinámicamente en SORTING con anti_jam_check() durante la espera
- [x] `anti_jam_check()` integrado — encoder belt jam + break-beam blockage jam
- [x] Break-beams integrados vía anti_jam y TEST_BEAM command
- [x] Mínimo espaciado por encoder pulses (SET_SPACING + min_spacing_pulses)
- [x] `gpio_breakbeam_read()` llamado desde anti_jam y bt_protocol
- [x] `calibration_init()` con defaults si magic byte EEPROM no existe
- [x] Menú LCD cyclic editor: MODE cicla 7 parámetros, UP/DOWN ajusta, long-press guarda
- [x] Servo 2 reimplementado con TMR3 ISR (25 µs, ~3° de resolución) — igual calidad que servo 1
- [ ] Configuración HC-05 (AT mode → 115200) no documentada

---

## 4. TUI Python (OpenCode + agy)

- [x] **Task 15 — Scaffolding** (OpenCode)
  - [x] `pyproject.toml` con dependencias textual>=0.41, pyserial>=3.5
  - [x] `connect.py`: BTManager asíncrono con connect/disconnect/send/read_line
  - [x] `app.py`: FajaApp con polling BT, acción connect, dashboard

- [x] **Task 16 — Protocol shared** (OpenCode + agy)
  - [x] Constantes de comando (START, STOP, SET_SPEED, etc.)
  - [x] `parse_telemetry()` para STATE, SPEED, COLOR, DETECT, JAM

- [x] **Task 17 — Screens** (agy)
  - [x] `screens/dashboard.py`: DashboardScreen con estado, velocidad, color, log, botones START/STOP
  - [x] `screens/config.py`: ConfigScreen
  - [x] `screens/log_viewer.py`: LogViewerScreen

- [x] **Task 18 — Test screen** (agy)
  - [x] `screens/test_screen.py`: TestScreen con controles TEST_*, beam indicators, encoder counter

- [x] Auto-reconnect BT cada 3s implementado
- [x] COLOR_MAP en app.py traduce índices a nombres legibles
- [x] Fix: `query_one("DashboardScreen", ...)` → `query_one(DashboardScreen)` (CSS selector inválido)
- [x] `pyserial-asyncio` agregado a dependencias
- [x] `start.bat` + `start.sh` — scripts de lanzamiento automático (Windows/Linux/macOS/Termux)

---

## 5. Pendientes generales

- [x] Inicializar repo git y push a GitHub (`https://github.com/gmoca/fajaClasificadora`)
- [ ] Prueba de integración: TUI → BT → firmware → respuesta
- [x] Documentar HC-05 setup — `docs/hc05-setup.md`
- [x] Documentar assembly — `docs/assembly-guide.md`

---

## Leyenda

| Símbolo | Significado |
|---------|-------------|
| [x] | Completado y verificado |
| [~] | Parcial — falta algo |
| [ ] | Pendiente — no iniciado |

---
*Última actualización: 2026-07-14*
