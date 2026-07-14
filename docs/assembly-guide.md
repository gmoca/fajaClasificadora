# Guía de Ensamble y Cableado

## Componentes necesarios

| Cantidad | Componente | Especificación |
|----------|------------|----------------|
| 1 | PIC18F4550 | DIP-40 |
| 1 | Cristal 20MHz | HC-49/S, 20pF load caps |
| 2 | Capacitor 22pF | Cerámico, para el cristal |
| 1 | Condensador 100nF | Cerámico, desacople VDD-VSS |
| 1 | Condensador 10µF | Electrolítico, bulk en VDD |
| 1 | Módulo HC-05 | Bluetooth esclavo |
| 1 | Sensor TCS34725 | Color I2C (GY-33 o Adafruit) |
| 1 | LCD 1602 + PCF8574 | I2C, address 0x27 |
| 1 | L298N | Puente H para motor DC |
| 1 | Motor DC | 12V con encoder de hendiduras |
| 1 | Encoder óptico | FC-03 o similar |
| 1-2 | Servo | SG90, MG90S o equivalente |
| 2 | Par break-beam IR | LED IR + fototransistor |
| 3 | Botón pulsador | Normalmente abierto (NO) |
| 1 | Pulsador NC + pull-up | Paro de emergencia |
| 3 | Regulador 7805 | Para rieles lógico, servos |
| 1 | Condensador 470µF | Por cada 7805 |
| 1 | Condensador 1000µF | Cerca del L298N |
| 1 | 1kΩ + 2.2kΩ | Divisor de voltaje HC-05 RX |
| 3 | 10kΩ | Pull-ups para botones |

## Diagrama de conexiones

### PIC18F4550 — Asignación de pines

```
                 ┌─────────────┐
        MCLR ────┤1          40├──── RB7 (no conectar)
   RA0 (AN0) ───┤2          39├──── RB6 (no conectar)
   RA1 (AN1) ───┤3          38├──── RB5 (no conectar)
   RA2 (AN2) ───┤4          37├──── RB4 (no conectar)
   RA3 (AN3) ───┤5          36├──── RB3 — E-stop input (Polled, pull-up)
 RA4 (T0CKI) ───┤6          35├──── RB2 — Encoder (INT2)
   RA5 (AN4) ───┤7          34├──── RB1 — SCL (I2C Clock)
   RE0 (AN5) ───┤8          33├──── RB0 — SDA (I2C Data)
   RE1 (AN6) ───┤9          32├──── VDD (5V Logic)
   RE2 (AN7) ───┤10         31├──── VSS (GND)
   VDD Logic ───┤11         30├──── RD7 — Break-beam RX 2
   VSS Logic ───┤12         29├──── RD6 — Botón DOWN
OSC1 (20MHz) ───┤13         28├──── RD5 — Botón UP
OSC2 (20MHz) ───┤14         27├──── RD4 — Break-beam RX 1
RC0 (Servo2) ───┤15         26├──── RC7 — RX (UART)
RC1 (Servo1) ───┤16         25├──── RC6 — TX (UART)
RC2 (PWM ENA) ──┤17         24├──── RC5 (no conectar)
  RC3 (VPO) ────┤18         23├──── RC4 (no conectar)
 RD0 (IN1 PH1) ─┤19         22├──── RD3 — Break-beam emitter
 RD1 (IN2 PH2) ─┤20         21├──── RD2 — Botón MODE
                 └─────────────┘
```

**Notas:**
- **RB0 = SDA (I2C):** Pin 33 de datos I2C. Requiere pull-up externo a 5V.
- **RB1 = SCL (I2C):** Pin 34 de reloj I2C. Requiere pull-up externo a 5V.
- **RB3 = E-stop (Polled):** Pin 36. Monitoreado cada 1ms para seguridad.
- **RB2 = INT2 (Encoder):** Pin 35.
- **RC1 = CCP2 (Servo 1):** Pin 16, modo Compare por hardware.
- **RC0 = GPIO (Servo 2):** Pin 15, software PWM.
- **RC2 = CCP1 (PWM ENA):** Pin 17 → H-bridge ENA.
- **RC6 = TX (UART):** Pin 25 → HC-05 RX (vía divisor resistivo).
- **RC7 = RX (UART):** Pin 26 ← HC-05 TX.

### Fuente de poder — Tres rieles separados

```
Fuente 12V ──┬── L298N (12V input)
             │
             ├── 7805 ── 5V lógico ── PIC, HC-05, TCS34725, LCD, encoder, break-beams
             │
             └── 7805 ── 5V servos ── Servo 1, Servo 2 (opt)
```

| Riel | Voltaje | Corriente | Capacitancia |
|------|---------|-----------|--------------|
| Lógico | 5V | <200mA | 470µF + 100nF |
| Servos | 5V | Hasta 2A pico | 470µF + 100µF cerámico |
| Motor | 12V | 1-3A | 1000µF |

**Reglas:**
- Masa común (estrella en la entrada de poder)
- Reguladores independientes — no compartir riel lógico con servos
- Diodo de protección inversa en cada 7805 (1N4007)
- Capacitor de tantalio/cerámico 100nF cerca de cada VDD del PIC

### L298N — Puente H

| L298N | Conexión |
|-------|----------|
| VCC (12V) | Batería/fuente 12V |
| GND | Masa común |
| 5V output | NC (no usar — usar riel lógico independiente) |
| ENA | RC2 (PWM del PIC) |
| IN1 | RD0 |
| IN2 | RD1 |
| Motor A | Motor DC |

### Break-beam IR

```
RD3 ──┬── 220Ω ── LED IR emisor ── GND  (estación 1 y 2, mismo pin)
      │
      └── 220Ω ── LED IR emisor ── GND  (estación 2)

RD4 ──┬──<fototransistor>── 10kΩ ── 5V  (receptor estación 1)
      │
      └── GND

RD7 ──┬──<fototransistor>── 10kΩ ── 5V  (receptor estación 2)
      │
      └── GND
```

Los emisores IR se conectan en paralelo desde RD3. Ambos se encienden/apagan juntos.

### Botones (pull-up externos requeridos — PORTD no tiene pull-ups internos)

```
RD2 (MODE) ──┬── pulsador ── GND
             │
             10kΩ
             │
             5V

RD5 (UP) ──┬── pulsador ── GND
           │
           10kΩ
           │
           5V

RD6 (DOWN) ──┬── pulsador ── GND
             │
             10kΩ
             │
             5V
```

### Paro de emergencia (E-stop)

```
RB0 ──┬── pulsador NC ── GND
      │
      10kΩ
      │
      5V
```

Normalmente RB0 está en HIGH (5V por pull-up). Al presionar E-stop, RB0 baja a GND → INT0 dispara.

### Encoder de hendiduras (FC-03)

| Encoder | PIC |
|---------|-----|
| VCC | 5V (riel lógico) |
| GND | GND |
| DO (salida digital) | RB2 (INT2) |

### Sensor de color TCS34725

| TCS34725 | PIC |
|----------|-----|
| VIN | 5V (riel lógico) |
| GND | GND |
| SCL | RB1 (SCL) |
| SDA | RB0 (SDA) |
| INT | NC (no conectado — se sondea por I2C) |
| LED | 5V (enciende LED interno) |

### LCD 1602 + PCF8574

| LCD I2C | PIC |
|---------|-----|
| VCC | 5V (riel lógico) |
| GND | GND |
| SCL | RB1 (SCL) |
| SDA | RB0 (SDA) |

Address I2C: **0x27** (por defecto en la mayoría de módulos PCF8574).

### Servos

| Servo 1 | PIC |
|---------|-----|
| Rojo (VCC) | 5V (riel servos) |
| Negro (GND) | GND común |
| Naranja (señal) | RC1 (CCP2) |

| Servo 2 (opcional) | PIC |
|--------------------|-----|
| Rojo (VCC) | 5V (riel servos) |
| Negro (GND) | GND común |
| Naranja (señal) | RC0 (GPIO, software PWM) |

## Lista de verificación pre-encendido

- [ ] Polaridad de alimentación verificada (ningún riel al revés)
- [ ] PIC: cristal 20MHz con capacitores 22pF a GND
- [ ] PIC: MCLR con pull-up 10kΩ a 5V
- [ ] PIC: 100nF entre cada par VDD-VSS
- [ ] 7805: capacitores de entrada y salida según datasheet
- [ ] Divisor resistivo 1kΩ + 2.2kΩ en TX del PIC → RX del HC-05
- [ ] Pull-ups de 10kΩ en los 3 botones (PORTD no tiene pull-ups)
- [ ] Diodo de protección inversa en cada regulador
- [ ] Masas: todas comparten el mismo punto de tierra
- [ ] E-stop: resistor pull-up a 5V, pulsador NC a GND

## Calibración inicial después del ensamble

1. **Verificar I2C:** Conectar solo PIC + LCD, ejecutar scan de direcciones I2C. Debe aparecer 0x27.
2. **Agregar TCS34725:** Scan debe mostrar 0x29 además de 0x27.
3. **Probar servos:** Enviar `SERVO_SET 1 90` desde TUI. El servo debe moverse a 90°.
4. **Probar motor:** Enviar `TEST_MOTOR 180 fwd`. El motor debe girar por 2s.
5. **Configurar HC-05:** Seguir `docs/hc05-setup.md` para fijar 115200 baudios.
6. **Probar comunicación:** Conectar TUI, enviar `STATUS`. Debe responder.
