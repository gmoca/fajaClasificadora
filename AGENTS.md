# pryMicroFaja.X

MPLAB X IDE v6.30 project for **PIC18F4550** microcontroller, compiled with **XC8 v3.10** (C99).

## Project state

- Firmware completo, compila y linkea. **45.0% flash, 34.6% RAM**.
- TUI Python con Textual, 4 pantallas implementadas.
- No es un repositorio git (aún).

## Reglas de sincronización para agentes (OpenCode + agy)

### ANTES de empezar a trabajar, SIEMPRE:
1. **Leer `CHECKLIST.md`** — estado global de lo completado y pendiente.
2. **Leer `Journal.md`** — bitácora de cambios del otro agente.
3. **Leer `HANDOFF_PARA_OPENCODE.md` y `HANDOFF_PARA_ANTIGRAVITY.md`** — mensajes entre agentes.
4. **Leer `AGENTS.md`** (este archivo) — reglas y contexto del proyecto.

### Durante el trabajo:
5. **No sobrescribir archivos del otro agente sin verificar primero.**
   - Si un archivo tiene lógica del otro agente (e.g. `state_machine.c` es de agy, `system.c` es de OpenCode), editar solo lo que te corresponde.
   - Si necesitas modificar un archivo ajeno, dejar comentario con `/* agy: ... */` o `// OpenCode: ...` marcando el cambio.
6. **Actualizar `CHECKLIST.md`** al completar o descubrir un pendiente.

### DESPUÉS de trabajar, SIEMPRE:
7. **Compilar/verificar** que el proyecto no se rompa:
   ```bash
   make -f firmware/Makefile.firmware
   ```
8. **Actualizar `Journal.md`** con un resumen de lo que hiciste.
9. **Dejar handoff** en `HANDOFF_PARA_OPENCODE.md` (si eres agy) o `HANDOFF_PARA_ANTIGRAVITY.md` (si eres OpenCode) si hay algo que el otro necesita saber.
10. **No commits sin permiso del usuario.**

### Mapa de archivos por agente:

| Archivos | Dueño |
|----------|-------|
| `firmware/config.h`, `system.c/.h`, `gpio.c/.h`, `uart.c/.h`, `i2c.c/.h`, `lcd.c/.h`, `tcs34725.c/.h`, `pwm.c/.h`, `servo.c/.h`, `encoder.c/.h`, `main.c`, `Makefile.firmware` | **OpenCode** |
| `firmware/state_machine.c/.h`, `bt_protocol.c/.h`, `calibration.c/.h`, `anti_jam.c/.h` | **agy** |
| `tui_app/protocol.py` | **Compartido** |
| `tui_app/app.py`, `connect.py` | **OpenCode** |
| `tui_app/screens/*.py` | **agy** |

---

## Build

### Firmware (faster, standalone)
```bash
make -f firmware/Makefile.firmware
```
Output: `dist/default/production/pryMicroFaja.X.production.hex`

### Via MPLAB X (full IDE build)
```bash
make build          # production .hex → dist/default/production/pryMicroFaja.X.production.hex
make clean          # rm -rf build/ dist/
make clobber        # clean all
```

Build requires MPLAB X v6.30 + XC8 v3.10 installed at the standard paths (`C:\Program Files\Microchip\...`). The IDE auto-regenerates `Makefile-local-default.mk` per-host; that file holds tool paths and must exist before `make build` will work.

### TUI app
```bash
cd tui_app && pip install -e .
python app.py
```

## Adding source files

Source files are tracked in `nbproject/configurations.xml` (the `<itemPath>` entries inside `<SourceFiles>` and `<HeaderFiles>` logical folders). After adding files via MPLAB X, the IDE regenerates the `SOURCEFILES`/`OBJECTFILES` variables in `nbproject/Makefile-default.mk`.

## Conventions

- **C99** (`-std=c99`), **dwarf-3** debug format.
- Optimization: `-O0` (disabled, `disable-optimizations=true`).
- Output: COFF+ELF hybrid (`-mcof,+elf`), with `.hex` for production.
- No peripheral library linked (`link-in-peripheral-library=false`).
