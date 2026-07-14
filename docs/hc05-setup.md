# Configuración del Módulo HC-05

## Requisitos

- Módulo HC-05
- Cable USB-TTL (3.3V o 5V tolerante) o el propio PIC18F4550 con un programa que envíe AT commands
- Terminal serie (PuTTY, Arduino IDE Monitor Serie, `screen` en Linux, `minicom`)

## Paso 1: Entrar en modo AT

El HC-05 tiene dos modos:

| Modo | Estado | Indicación LED |
|------|--------|----------------|
| AT   | Presionar botón EN mientras se alimenta | LED parpadea lento (~2s) |
| Datos | Normal | LED parpadea rápido (~0.5s) |

Para entrar en modo AT:

1. **Conectar EN (Key) a VCC (3.3V)** — esto fuerza modo AT
2. Alimentar el módulo (3.3V - 5V en VCC, GND común)
3. El LED debe parpadear cada ~2 segundos

**Alternativa sin botón:** Si tu HC-05 viene en una placa breakout, usa un jumper entre EN y VCC.

## Paso 2: Conectar USB-TTL al HC-05

| USB-TTL | HC-05 |
|---------|-------|
| TX      | RX    |
| RX      | TX    |
| 3.3V    | VCC   |
| GND     | GND   |

> **Importante:** El HC-05 funciona a 3.3V, pero tolera 5V en VCC si la placa tiene regulador. Verifica el voltaje de tu breakout.

## Paso 3: Comandos AT para configurar 115200 baudios

Conecta vía terminal serie a **38400 baudios** (baud rate por defecto del HC-05 en modo AT).

```bash
# Linux / Termux
screen /dev/ttyUSB0 38400

# Windows (PuTTY)
# Serial → COM3 → 38400 → Open
```

Cada comando AT debe terminar con `\r\n` (CR+LF). El HC-05 responde con `OK` o `ERROR`.

### Configurar baud rate, nombre y modo maestro/esclavo

```
AT+NAME=FajaClasificadora
AT+UART=115200,0,0
AT+ROLE=0
AT+CMODE=0
AT+BIND=<direccion_del_maestro>
AT+RESET
```

| Comando | Significado |
|---------|-------------|
| `AT+NAME=FajaClasificadora` | Nombre Bluetooth visible |
| `AT+UART=115200,0,0` | Baud=115200, stop bits=1, parity=none |
| `AT+ROLE=0` | Esclavo (el PIC es esclavo, el TUI es maestro) |
| `AT+CMODE=0` | Conectar solo a dirección específica (seguro) |
| `AT+BIND=<addr>` | (Opcional) Vincular a dirección MAC del maestro |
| `AT+RESET` | Reinicia y aplica cambios |

**`AT+UART=115200,0,0` es crítico** — el firmware del PIC está configurado a 115200. Si el HC-05 está en otro baud rate, no hay comunicación.

### Verificar configuración

```
AT+NAME?
AT+UART?
AT+ROLE?
AT+ADDR?
```

### Ejemplo de sesión completa

```
AT
OK
AT+NAME=FajaClasificadora
OK
AT+UART=115200,0,0
OK
AT+ROLE=0
OK
AT+RESET
OK
```

Después del reset, desconecta EN de VCC. El módulo arranca en modo datos a 115200 baudios.

## Paso 4: Conexión al PIC

| HC-05 | PIC18F4550 |
|-------|------------|
| TX    | RC7 (RX)   |
| RX    | RC6 (TX)   |
| VCC   | 5V regulado (rail lógico) |
| GND   | GND común  |

**Divisor de voltaje en RX del HC-05:** El PIC18F4550 funciona a 5V, pero el HC-05 espera 3.3V en su pin RX. Aunque muchos HC-05 toleran 5V, es buena práctica agregar un divisor resistivo:

```
PIC TX (RC6) ──┬─ 1kΩ ── HC-05 RX
               │
               └─ 2.2kΩ ── GND
```

Esto entrega ~3.3V al RX del HC-05. El TX del HC-05 (3.3V) se conecta directo al RX del PIC (RC7) — 3.3V es suficiente para que el PIC lo lea como HIGH.

## Paso 5: Verificar comunicación

Con el TUI Python:
```bash
cd tui_app && pip install -e . && python app.py
```

O con un script rápido:
```python
import serial
s = serial.Serial("COM3", 115200, timeout=2)
s.write(b"STATUS\n")
print(s.readline().decode())
s.close()
```

## Solución de problemas

| Problema | Causa posible | Solución |
|----------|---------------|----------|
| No aparece en escaneo BT | HC-05 no está en modo datos | Verificar EN no esté en VCC |
| Conexión falla | Baud rate incorrecto | Reconfigurar con AT+UART |
| Datos corruptos | Voltaje RX demasiado alto | Agregar divisor resistivo |
| No responde a AT | Baud rate incorrecto o EN no conectado | Probar 38400, verificar EN |
| LED siempre encendido | HC-05 en modo AT | Resetear o desconectar EN |
