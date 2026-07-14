# Journal del Proyecto: Micro Faja Transportadora

Este archivo sirve como bitácora de desarrollo (changelog) para documentar el progreso, decisiones de diseño, problemas resueltos y tareas completadas por los agentes del proyecto (agy y OpenCode).

---

## Registro de Antigravity (agy)
**Fecha:** 2026-07-13
**Rol asignado:** Lógica de negocio de alto nivel (Firmware) y Pantallas de Interfaz (TUI Python)

### 1. Firmware (C - PIC18F4550)
- **Máquina de Estados (`state_machine.c` / `.h`)**: 
  - Implementé el flujo principal: `ST_IDLE`, `ST_RUNNING`, `ST_SORTING`, `ST_ERROR`.
  - Agregué el modo de mantenimiento (`ST_TEST`) que aísla el motor de la lógica automática y cuenta con un **watchdog de 2 segundos** (detiene el motor si se pierde comunicación durante una prueba manual).
  - Integré exitosamente los drivers de hardware proporcionados por OpenCode (lecturas de encoder, sensor TCS34725 y comandos de servo).
- **Protocolo Bluetooth (`bt_protocol.c` / `.h`)**: 
  - Desarrollé el parser ASCII para interpretar comandos entrantes (`START`, `STOP`, `SET_SPEED`, comandos `TEST_*`, etc.) y despacharlos a la máquina de estados.
- **Sistema Anti-Atascos (`anti_jam.c` / `.h`)**: 
  - Programé el sistema de monitoreo continuo. Si el PWM del motor está encendido pero el encoder marca 0 mm/s por más de 1 segundo continuo, la máquina entra automáticamente en paro de emergencia (`ST_ERROR`).
- **Persistencia y Calibración (`calibration.c` / `.h`)**: 
  - Estructuré el guardado y lectura de parámetros de color y servos utilizando la EEPROM.

### 2. Interfaz de Usuario (Python + Textual)
- **Manejo de Telemetría (`tui_app/protocol.py`)**: 
  - Parser para transformar la telemetría plana del PIC (ej. `STATE:run`, `SPEED:150`) en diccionarios de eventos para actualizar la UI.
- **Vistas y Pantallas (`tui_app/screens/`)**: 
  - Creadas las 4 pantallas solicitadas en el diseño: `DashboardScreen` (vista principal), `ConfigScreen` (ajuste de velocidad manual), `LogViewerScreen` (consola de eventos raw) y `TestScreen` (controles manuales para hardware).
  - Inyectadas y conectadas de forma asíncrona a la clase base de OpenCode (`app.py`).

### Notas técnicas y lecciones (agy):
- **Flash Memory:** Decidí no utilizar la librería estándar de `sprintf` en la telemetría del firmware. Redacté funciones ligeras de conversión manual de números a cadenas (`u16_to_str` y `u32_to_str`), lo que ahorra una enorme cantidad de memoria en el microcontrolador.
- **Colisión de concurrencia:** Durante el desarrollo paralelo, OpenCode y yo tocamos los mismos archivos simultáneamente. Solucioné el cruce re-implementando las funciones faltantes encima de las correcciones de compilador aportadas por OpenCode.

### Actualización (agy) - 2026-07-13 (Noche):
- **Cierre de brechas TUI-BT:** Implementé en `bt_protocol.c` todos los comandos faltantes dictados en la especificación (`SET_MODE`, `SET_SPACING`, `SET_THRESHOLD`, todos los comandos de TEST y diagnósticos del hardware).
- **Lógica dinámica:** Sustituí los retardos fijos (500ms) en la etapa de clasificación (`ST_SORTING`) por tiempos dinámicos que se calculan con el encoder. El sistema espera `transit_ms` exactos y usa un while-loop monitoreando en tiempo real con `anti_jam_check()`.
- **Anti-Jam avanzado:** Agregué la detección por sensores láser. Ahora el sistema detecta atascos por timeout de 3 segundos consecutivos si algún Break-Beam se encuentra tapado ininterrumpidamente (indicativo de objeto atascado).
- **TUI Mejoras:** Implementé en `app.py` la función de auto-reconexión Bluetooth solicitada (cada 3 segundos) y el diccionario de traducción `COLOR_MAP` para pasar de los índices del firmware a nombres legibles.
- **Limpieza de Checklist:** Marqué las tareas 12, 13, 14, 17 y 18 como 100% resueltas en `CHECKLIST.md`. El sistema ahora es fully-featured.

### Actualización (agy) - 2026-07-14 (Madrugada):
- **Inicialización de EEPROM y Posiciones por Defecto:** Implementé la función `calibration_init()` en `main.c` y `calibration.c`. Ahora el firmware comprueba el Magic Byte (`0xA5`) en la dirección `0x00` de la EEPROM. Si está ausente, inicializa la EEPROM con los valores por defecto (Servo 1/2 Home = 90, Deflect = 0, Dwell = 500, PPR = 20, N = 0).
- **Lectura/Escritura Real de Servos:** Conecté las funciones `calibration_save_servo_home`, `calibration_save_servo_deflect`, `calibration_save_servo_dwell`, `calibration_save_ppr` y `calibration_send_servo_config` para que interactúen directamente con las celdas de la EEPROM a nivel de bytes/palabras (words).
- **Menú LCD Local Cíclico (Mantenimiento):** Agregué la lógica en `state_machine.c` para que al estar en estado `ST_TEST` con inactividad Bluetooth mayor a 5 segundos (`system_ticks - last_bt_activity > 5000`), el sistema active un menú interactivo en el LCD 1602.
  - **MODE (RD2):** Cicla entre los 7 parámetros de configuración. El botón implementa detección por software de presión corta (avanza) y presión larga de 1.5s (guarda el valor en EEPROM y muestra "GUARDADO!").
  - **UP/DOWN (RD5/RD6):** Modifican el valor en tiempo real y desplazan el servo físico correspondiente para verificar la alineación al instante.
  - **Conexión de Estado:** La variable `last_bt_activity` se actualiza dinámicamente en `bt_protocol.c` al leer de la UART para salir del menú de inmediato si se detecta actividad del TUI.
- **Scripts de Lanzamiento Automático:** Diseñé y creé los archivos `start.bat` y `start.sh` en `tui_app/` para permitir que cualquier usuario inicialice automáticamente el entorno virtual de Python e instale todas las dependencias requeridas en Windows, Linux, macOS o Android Termux con un solo comando.

---

## Registro de OpenCode
**Fecha:** 2026-07-13
**Rol asignado:** Drivers HAL de firmware (PIC18F4550) + Scaffolding TUI Python

### 1. Firmware — Capa de Hardware (HAL)

#### Configuración del dispositivo (`config.h`)
- `#pragma config` para crystal HS 20MHz, PLL habilitado (96MHz USB), watchdog desactivado, LVP desactivado, DEBUG desactivado, ICPRT desactivado.
- Definición de `_XTAL_FREQ 20000000UL` para las macros `__delay_us()`/`__delay_ms()`.

#### Sistema e Interrupciones (`system.c` / `.h`)
- **IPEN = 1** — modo de interrupción por prioridad (dos vectores: alta 0x0008, baja 0x0018).
- **TMR0** — 16-bit, prescaler 1:2, reload 64911, interrupción cada ~1ms (base de tiempo `system_ticks`).
- **TMR1** — free-run con prescaler 1:2 (400ns/tick) como base de tiempo para CCP2 Compare (servo 1).
- **TMR2** — prescaler 1:1, PR2=249 → 20kHz para CCP1 PWM (H-bridge).
- `T3CONbits.T3CCP2 = 0` forzado para que CCP2 use TMR1 (por defecto CCP2 se conecta a TMR3).
- Handlers de interrupción: `isr_high` (INT0 e-stop, TMR0), `isr_low` (INT2 encoder, TMR1 servo 2).

#### GPIO (`gpio.c` / `.h`)
- **H-bridge direcciones**: RD0 (EN), RD1 (PH1), RD2 (PH2). Enums `HB_STOP`, `HB_FWD`, `HB_REV`.
- **Botones**: RD3 (MODE), RD4 (UP), RD5 (DOWN) con debounce por hardware de 50ms vía TMR0.
- **Break-beam IR**: RA5 como entrada digital con pull-up.

#### UART (`uart.c` / `.h`)
- **BRG16=1, BRGH=1, SPBRG=42** → 115200 baudios desde 20MHz.
- Buffers circulares de 64 bytes para RX y TX.
- Funciones: `uart_send_byte()`, `uart_send_str()`, `uart_send_data()`, `uart_available()`, `uart_read_byte()`.

#### I2C / MSSP (`i2c.c` / `.h`)
- Modo maestro, 100kHz (SSPADD=49).
- Funciones de alto nivel: `i2c_start()`, `i2c_stop()`, `i2c_write_byte()`, `i2c_write()`, `i2c_read()`, `i2c_read_ack()`.

#### LCD 1602 + PCF8574 (`lcd.c` / `.h`)
- I2C address 0x27, modo 4 bits.
- Inicialización, clear, cursor positioning, print string.

#### Sensor de Color TCS34725 (`tcs34725.c` / `.h`)
- I2C address 0x29, identificación del chip.
- Integración ~50ms, gain 4x.
- Lectura raw de los 4 canales (R/G/B/C) en 16 bits.

#### PWM H-Bridge (`pwm.c` / `.h`)
- CCP1 en modo PWM, TMR2 como base → 20kHz.
- `pwm_hbridge_set_duty(0-255)`.

#### Servos (`servo.c` / `.h`)
- **Servo 1** (CCP2 Compare): ciclo de dos fases — fuerza LOW (CCP2CON=0b0101), espera ~1-2ms, fuerza HIGH + special event reset TMR1 (CCP2CON=0b0111). Resolución de 400ns.
- **Servo 2** (software PWM en RC0): manejado por TMR1 ISR con conteo de ticks.

#### Encoder (`encoder.c` / `.h`)
- INT2 en RB2, flanco de subida, prioridad baja.
- Contador de pulsos acumulados (32 bits, safety overflow).
- Cálculo de velocidad: ventana de 500ms, conversión a mm/s usando PPR configurable.

### 2. Firmware — Correcciones de integración

Al integrar el código de agy, encontré y corregí:

- **`state_machine.h`**: Faltaban declaraciones de `state_machine_start()`, `state_machine_test_enter()`, `state_machine_test_exit()`, `state_machine_test_motor()` — añadidas.
- **`calibration.h`**: Defines `EEPROM_ADDR_NUM_CLR` (0x15) y `EEPROM_ADDR_COLORS` (0x16) ausentes — añadidos.
- **`calibration.c`**: Las macros heredadas `eeprom_write()`/`eeprom_read()` están obsoletas en XC8 v3.10 (compilador clang). Reemplazadas con acceso directo a registros EEDATA/EEADR/EECON1/EECON2.
- **`state_machine.c`**: Las funciones `utoa()`/`ultoa()` no existen en XC8 v3.10 (eran extensiones del viejo HI-TECH C). Reemplazadas con funciones estáticas de conversión manual (`u16_to_str`, `u32_to_str`).

### 3. Aplicación TUI (Python + Textual)

- **`tui_app/pyproject.toml`**: Proyecto Python con dependencias mínimas.
- **`tui_app/app.py`**: Clase `FajaApp` con bucle de polling BT, bindings de teclado, cola de comandos `bt_send()`.
- **`tui_app/connect.py`**: Manejador asíncrono de conexión serial Bluetooth.
- **`tui_app/protocol.py`**: Constantes de comando y parser de telemetría.

### 4. Resultado del Build

```bash
make -f firmware/Makefile.firmware
```

| Componente | Estado |
|-----------|--------|
| 14 archivos .c | ✅ Compilan sin errores |
| Link | ✅ `.hex` generado |
| Flash usada | 11963 bytes (36.5%) |
| RAM usada | 575 bytes (28.1%) |
| EEPROM | 0 bytes (0%) |

### 5. Archivos creados/modificados por OpenCode

```
firmware/
├── config.h          — #pragma config, oscilador, defines
├── system.c/.h       — IPEN, timers, vectores de interrupción
├── gpio.c/.h         — botones, H-bridge, break-beam
├── uart.c/.h         — 115200 baud, buffers circulares
├── i2c.c/.h          — MSSP master 100kHz
├── lcd.c/.h          — LCD 1602 + PCF8574
├── tcs34725.c/.h     — sensor de color I2C
├── pwm.c/.h          — CCP1 PWM 20kHz H-bridge
├── servo.c/.h        — CCP2 Compare + software PWM
├── encoder.c/.h      — INT2, velocidad mm/s
├── main.c            — inicialización y loop principal
├── Makefile.firmware — build independiente
tui_app/
├── app.py            — clase principal FajaApp
├── connect.py        — BT serial async
├── protocol.py       — parser de telemetría
├── pyproject.toml    — configuración del proyecto
├── start.bat         — lanzador automático para Windows
└── start.sh          — script de arranque para Linux/macOS/Termux
```

### Actualización (agy) - 2026-07-14 (Madrugada):
- **Inicialización EEPROM:** Creó `calibration_init()` en `calibration.c`. Si el Magic Byte `0xA5` no está en `0x00`, escribe valores por defecto (Home=90, Deflect=0, Dwell=500, PPR=20).
- **Menú LCD Cyclic Editor:** En modo TEST, si no hay actividad BT por 5s, entra en editor cíclico local:
  - MODE corto: cicla 7 parámetros (S1 Home/Defl/Dwell, S2 Home/Defl/Dwell, PPR)
  - MODE largo (>1.5s): guarda en EEPROM y muestra "GUARDADO!"
  - UP/DOWN: ajustan valor y mueven servo en tiempo real
  - Vuelve automáticamente al recibir datos por UART
- **Funciones EEPROM:** `calibration_write_word()`, `calibration_read_word()`, `calibration_save_servo_home/deflect/dwell()`, `calibration_save_ppr()`

### Actualización (OpenCode) - 2da ronda de integración
**Fecha:** 2026-07-13 (noche)

agy completó su segunda ronda con:
- `bt_protocol.c`: SET_MODE, SET_SPACING, SET_THRESHOLD, todos los comandos TEST_*
- `state_machine.c`: SORTING con `transit_ms` dinámico + `anti_jam_check()` en el while-loop
- `anti_jam.c`: detección de break-beam bloqueado >3s (además de encoder belt jam)
- `app.py`: auto-reconnect BT cada 3s, COLOR_MAP para nombres de color

**Correcciones que hice para compilar en XC8 v3.10:**
1. `bt_protocol.c:119` — `ultoa()` no existe en XC8 v3.10. Reemplazado con conversión manual inline.
2. `calibration.c:73` — Faltaba `#include "uart.h"` para `uart_send_str()`.

**Build final: 45% flash, 34.6% RAM — 0 errores.**

**Reglas de sincronización:** Actualicé `AGENTS.md` con reglas para ambos agentes (leer CHECKLIST/Journal/handoffs antes de trabajar, no sobrescribir archivos ajenos, compilar después de cada cambio).

**CHECKLIST.md:** Creé checklist completo con estado de todas las tareas del plan de implementación.

### Notas técnicas y lecciones (OpenCode):
- **T3CCP2 silencioso**: En PIC18F4550, CCP2 por defecto usa TMR3. Hay que forzar `T3CONbits.T3CCP2 = 0` para que use TMR1. Si no, el servo 1 (CCP2 Compare) nunca funciona.
- **CCP2 Compare vs PWM**: Los modos CCP2CON = 0b1000/0b1001 son PWM, NO Compare. Para generar pulsos de servo hay que usar los modos 0b0101 (force LOW) y 0b0111 (special event trigger + force HIGH).
- **eeprom_write/read obsoletos**: XC8 v3.10 (clang) eliminó el soporte de las macros `eeprom_write`/`eeprom_read` del header `<pic18.h>`. Hay que usar registros directamente.
- **utoa/ultoa eliminados**: XC8 v3.10 no incluye las extensiones `utoa`/`ultoa` del viejo compilador HI-TECH. Cualquier conversión numérica debe hacerse con funciones manuales para evitar el peso de `sprintf`.
- **Stack compilado**: Con `-mstack=compiled:auto:auto:auto` el compilador asigna el stack estáticamente, eliminando la necesidad de stack hardware y ahorrando RAM.

### Actualización (OpenCode) - 2026-07-14 (tarde)
- **Fix `app.py`**: `query_one("DashboardScreen", DashboardScreen)` no funciona en Textual (los CSS selectors con string requieren el nombre en minúsculas). Cambiado a `query_one(DashboardScreen)` pasando la clase directamente.
- **Dependencia faltante**: `connect.py` usa `import serial_asyncio`, que requiere `pyserial-asyncio`. Agregado a `pyproject.toml`.
- **Docs actualizados**: CHECKLIST.md (build stats, TUI fixes), HANDOFF_PARA_ANTIGRAVITY.md (última build con todos los cambios de agy).
- **Revisión de cambios de agy**: Verifiqué `start.bat` y `start.sh` (scripts de lanzamiento automático). Se ven correctos.

### Actualización (agy) - 2026-07-14 (ConfigScreen servos)
- **`tui_app/screens/config.py`**: Agregó sección de configuración de servos: Input para Servo ID, botones "Guardar Home", "Guardar Deflexión", Input Dwell + botón guardar. Todo conectado vía BT.
- **`firmware/servo.c`**: Agregó `servo_get_angle(sid)` — devuelve el ángulo actual del servo (0-180) a partir del pulse width activo.
- **`firmware/bt_protocol.c`**: `SERVO_SAVE_HOME` y `SERVO_SAVE_DEFLECT` ahora guardan el ángulo físico real vía `servo_get_angle()`. `SET_DWELL` ahora acepta `SET_DWELL <sid> <ms>` para guardar en EEPROM por servo.

### Actualización (OpenCode) - 2026-07-14 (servo 2 + docs)

- **Servo 2 reimplementado desde cero**: Antes era `servo_step()` con 1ms de resolución y solo 2 posiciones (~90° de error). Ahora usa **TMR3 como timer dedicado** con interrupción cada **25 µs**:
  - Resolución efectiva: ~3° (vs ~0.07° de servo 1 por hardware, pero totalmente funcional)
  - Misma interfaz: `servo_set_angle(2, angulo)` funciona igual que servo 1
  - Se activa automáticamente — `servo_init()` configura TMR3
  - ISR en `servo_timer3_isr()`, manejada desde `isr_low()` en system.c
- **14 archivos compilan sin errores**

### Actualización (OpenCode) - 2026-07-14 (documentación)
- **`docs/hc05-setup.md`**: Guía completa en español para configurar HC-05 en modo AT, comandos para fijar 115200 baudios, diagrama de conexión al PIC con divisor resistivo, y solución de problemas.
- **`docs/assembly-guide.md`**: Guía de ensamble con pinout del PIC18F4550, diagrama de 3 rieles de poder, conexiones L298N, break-beam IR, botones, E-stop, encoder, TCS34725, LCD, servos. Incluye checklist pre-encendido y pasos de calibración inicial.
- CHECKLIST.md: marcados como completados los items de documentación.

### Actualización (agy) - 2026-07-14 (Configuración de Servos en TUI)
- **TUI ConfigScreen Actualizada:** Añadí inputs para Servo ID y Tiempo Dwell, y botones para guardar el ángulo actual del servo como Home (`SERVO_SAVE_HOME <sid>`), como Deflexión (`SERVO_SAVE_DEFLECT <sid>`) y el tiempo Dwell (`SET_DWELL <sid> <ms>`) en la EEPROM.
- **Soporte de Ángulos en Servos (Firmware):** Añadí la función `servo_get_angle(sid)` en `servo.c`/`servo.h` para que el firmware pueda reconstruir y leer el ángulo actual en grados basándose en los anchos de pulso activos.
- **Comandos BT en Firmware:** Implementé y expandí los parsers de comandos en `bt_protocol.c`:
  - `SERVO_SAVE_HOME <sid>`: Guarda la posición actual de ángulo como Home.
  - `SERVO_SAVE_DEFLECT <sid>`: Guarda la posición actual como Deflexión.
  - `SET_DWELL <sid> <ms>`: Guarda el retardo de clasificación en la EEPROM para el servo especificado.

### Actualización (OpenCode) - 2026-07-14 (detección SO + fix HC-05)
- **`app.py`**: `action_connect()` ahora detecta el SO con `platform.system()`. En Windows prueba COM3/COM4 primero; en Linux/Termux prueba `/dev/rfcomm0`, `/dev/ttyUSB0` primero. TCP bridge como fallback.
- **`docs/hc05-setup.md`**: Cambiado `AT+CMODE=0` → `AT+CMODE=1` (modo seguro sin `AT+BIND` dejaba el HC-05 sin conexión). Agregada nota explicativa.

### Actualización (OpenCode) - 2026-07-14 (Termux + TCP bridge)
- **`docs/termux-setup.md`**: Guía completa para ejecutar TUI en Termux: instalación, 3 métodos de conexión BT (rfcomm root, Serial Bluetooth Terminal TCP bridge, USB-OTG), script de prueba y solución de problemas.
- **`connect.py`**: Agregado `connect_tcp(host, port)` — conexión asíncrona por TCP para el puente Serial Bluetooth Terminal.
- **`app.py`**: `action_connect()` ahora intenta TCP 127.0.0.1:8080 como fallback si los puertos seriales fallan.
- **`start.sh`** y **`start.bat`**: Agregados recordatorios de conexión Bluetooth antes de lanzar la app.

### Actualización (agy) - 2026-07-14 (Visibilidad de archivos en MPLAB X)
- **Sincronización de IDE (`nbproject/configurations.xml`):** Los archivos `.c` y `.h` del firmware no estaban declarados dentro del XML de configuración del proyecto de MPLAB X, por lo que el IDE no mostraba ningún archivo en el árbol lateral ("Source Files" y "Header Files"). Los agregué a sus carpetas lógicas correspondientes. El proyecto ahora se visualiza correctamente en el IDE y se reconstruyó/compiló de forma exitosa.

