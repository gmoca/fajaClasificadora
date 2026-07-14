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
        RC0 ─────┤1          40├───── VDD (5V)
        RC1 ─────┤2          39├───── VSS (GND)
        RC2 ─────┤3          38├───── RD7 — Break-beam RX 2
        RC3 ─────┤4          37├───── RD6 — Botón DOWN
        RC4 ─────┤5          36├───── RD5 — Botón UP
        RC5 ─────┤6          35├───── RD4 — Break-beam RX 1
        RC6 ─────┤7          34├───── RD3 — Break-beam emitter
        RC7 ─────┤8          33├───── RD2 — Botón MODE
    ┌── RB0 ─────┤9          32├───── RD1 — H-bridge IN2
    │   RB1 ─────┤10         31├───── RD0 — H-bridge IN1
    │   RB2 ─────┤11         30├───── VSS
    │   RB3 ─────┤12         29├───── VDD
    │   RB4 ─────┤13         28├───── OSC1 (20MHz cristal)
    │   RB5 ─────┤14         27├───── OSC2 (20MHz cristal)
    │   VSS ─────┤15         26├───── RC0/RC1 (servos)
    │   VDD ─────┤16         25├───── RC2 (PWM → L298N ENA)
    │   RA0 ─────┤17         24├───── MCLR (10kΩ → VDD)
    │   RA1 ─────┤18         23├───── RA6 (PIC16 — no conectar)
    │   RA2 ─────┤19         22├───── RA5 (break-beam adicional)
    │   RA3 ─────┤20         21├───── VUSB (dejar NC si no USB)
                └─────────────┘
```

**Notas:**
- RB0 = INT0 (E-stop) — nivel alto fijo, prioridad alta permanente
- RB2 = INT2 (encoder)
- RC1 = CCP2 (servo 1) — modo Compare
- RC0 = GPIO (servo 2 opcional) — software PWM
- RC2 = CCP1 (PWM) → H-bridge ENA
- RC3 = SCL (I2C)
- RC4 = SDA (I2C)
- RC6 = TX (UART) → HC-05 RX
- RC7 = RX (UART) ← HC-05 TX

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
| SCL | RC3 (SCL) |
| SDA | RC4 (SDA) |
| INT | NC (no conectado — se sondea por I2C) |
| LED | 5V (enciende LED interno) |

### LCD 1602 + PCF8574

| LCD I2C | PIC |
|---------|-----|
| VCC | 5V (riel lógico) |
| GND | GND |
| SCL | RC3 (SCL) |
| SDA | RC4 (SDA) |

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
