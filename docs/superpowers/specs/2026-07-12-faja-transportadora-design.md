# Faja Transportadora Automatizada — Design Spec

**Goal:** Build an automated conveyor belt that sorts objects by color, controlled via Bluetooth from a phone TUI app.

**Architecture:** Autonomous PIC18F4550 firmware (state machine + telemetry) communicates over Bluetooth serial with a Python Textual TUI app running in Termux. PIC handles all real-time control even when Bluetooth is disconnected.

**Tech Stack:** PIC18F4550 + XC8 v3.10 (C99), HC-05 Bluetooth, TCS34725 color sensor (I2C), LCD 1602 + PCF8574 (I2C), L298N H-bridge, 1-2 servos, Python Textual TUI in Termux.

---

## Work split

- **OpenCode:** PIC18F4550 firmware (HAL/drivers — PWM, I2C, UART, GPIO) + TUI app shell (Bluetooth connection, screen rendering).
- **Antigravity (agy):** Business logic on both sides — state machine, conveyor control, object sorting algorithm, speed-servo coordination, jam detection on PIC; UI logic, menu system, command interpretation on TUI.

---

## Hardware pin mapping (PIC18F4550)

| Pin  | Function                                     | Peripheral        |
|------|----------------------------------------------|-------------------|
| RC3  | SCL (I2C) — LCD + TCS34725                   | MSSP (I2C)        |
| RC4  | SDA (I2C) — LCD + TCS34725                   | MSSP (I2C)        |
| RC6  | TX (UART, 115200) → HC-05 RX                 | USART             |
| RC7  | RX (UART, 115200) ← HC-05 TX                 | USART             |
| RC2  | CCP1 (PWM) → H-bridge ENA                    | ECCP (PWM)        |
| RC1  | **CCP2 (Compare mode)** → Servo 1              | CCP2              |
| RC0  | GPIO, TMR1 ISR → Servo 2 (opt)                | GPIO              |
| RD0  | H-bridge IN1                                 | GPIO              |
| RD1  | H-bridge IN2                                 | GPIO              |
| RD2  | Button 1 (mode)                              | GPIO (poll TMR0)  |
| RD3  | Break-beam IR emitter (both stations)        | GPIO              |
| RD4  | Break-beam IR receiver — station 1           | GPIO              |
| RD5  | Button 2 (up)                                | GPIO (poll TMR0)  |
| RD6  | Button 3 (down)                              | GPIO (poll TMR0)  |
| RD7  | Break-beam IR receiver — station 2           | GPIO              |
| RB0  | **Emergency Stop (INT0, fixed high priority)**| **INT0**          |
| RB2  | **Encoder input (INT2, configurable priority)**| **INT2**         |

### Pin rationale

- **E-stop en RB0 (INT0):** INT0 es la única interrupción externa con prioridad alta fija por hardware (no tiene bit IP configurable). Esto garantiza que el E-stop siempre tenga máxima prioridad, incluso si firmware desconfigura los bits de prioridad. No es configurable, es inseparable del silicio.
- **Encoder en RB2 (INT2):** Prioridad configurable via INT2IP. Suficiente para encoder — perder un pulso ocasional no es catastrófico. INT2 está en RB2, no RB3 (el PIC18F4550 no tiene INT en RB3).
- **Buttons moved to PORTD:** RD2, RD5, RD6. Polled via TMR0 ISR (already handles debounce at 1ms ticks). No interrupt pins wasted. **External 10kΩ pull-up resistors required for buttons** — PORTD has no internal pull-ups (only PORTB has weak pull-ups via `RBPU`).
- **TCS34725 INT pin dropped:** Sensor is polled every ~100ms in the main loop, making the INT pin redundant.
- **I2C bus** is shared: LCD PCF8574 at 0x27, TCS34725 at 0x29.
- **Servo 1 via CCP2 Compare mode (not PWM):** CCP hardware PWM max period is ~819µs (prescaler 16) — cannot reach 20ms at 20MHz. But CCP2 in **Compare mode** (CCP2CON = 0b1001 or 0b1000) generates a precise match interrupt and auto-sets/resets the output pin. TMR1 free-runs with prescaler 1:2 (tick = 400ns). On compare match, CCP2 toggles RC1. The ISR loads the next compare value for the next edge. Resolution: **400ns** (~0.07°), with only 2 interrupts per 20ms period. CCP2MX left at default (RC1).
- **Servo 2 (optional) via TMR1 ISR:** If a second servo is needed, RC0 is toggled by software in the same TMR1-based scheme (lower resolution but adequate for a second station).

### Power architecture (issue #4)

Three separate rails, all sharing a common ground:

| Rail   | Voltage | Devices                                              | Notes                        |
|--------|---------|------------------------------------------------------|------------------------------|
| Logic  | 5V reg  | PIC18F4550, HC-05, TCS34725, LCD, encoder, break-beam| Low current (<200mA)         |
| Servos | 5V reg  | 1-2 hobby servos                                     | Separate regulator. Inrush up to 1A each when moving |
| Motor  | 12V/9V  | L298N + DC motor                                     | Direct from battery/power brick. L298N regulates 5V internally for its logic only |

**Constraints:**
- Each rail gets its own voltage regulator (7805 for servos, 7805 or buck for logic).
- Bulk capacitance: 470µF electrolytic on each regulator output. Additional 100µF ceramic near servo connectors. Additional 1000µF near L298N power input.
- Motor start-up and servo movement can draw 1-2A peak — the logic rail must be isolated from these transients.
- No single regulator powers both motor and servos.
- Ground: star ground at the power input. Separate power and signal ground returns (avoid ground loops through sensor cables).

### Pin clarification (issue #7)

RB0 is **exclusively** E-stop — INT0, fixed high priority by hardware. No dual function. Button 4 features are accessed via long-press on Button 1 (mode).

### Encoder (issue #1)

A slotted optical encoder (e.g., FC-03 or homemade with IR LED + phototransistor + slotted disc) on the motor shaft (or a belt roller if direct shaft access is unavailable). Connected to RB2 (INT2) for edge-triggered pulse counting (low priority, non-critical).

The encoder provides:
- **Real belt speed** via pulse count per time window (replaces the constant `belt_speed_mm_per_s` assumption)
- **Belt jam detection** — if pulses stop while motor PWM is active → JAM
- **Object spacing** — pulses between color detections give a load-independent measure of inter-object distance
- **Auto-recalibration** — speed is always live-measured, no calibration needed

For absolute speed in mm/s: measure roller circumference once during assembly, store as constant. Encoder pulses per revolution is known (slots in disc). Speed = (pulses_per_second / pulses_per_rev) * circumference_mm.

### Break-beam IR sensors (issues #2, #5)

One IR break-beam pair per servo station. **Mandatory, not optional.** Station 1 receiver on **RD4**, station 2 receiver on **RD7**. Both emitters share the control pin **RD3**. This provides:
- **Object arrival confirmation** at each servo station (not just estimated transit time)
- **Servo jam detection**: beam doesn't clear within servo dwell window → JAM
- **Inter-object spacing enforcement**: if a second object breaks the beam before the first is done sorting → spacing violation (system can hold or divert to overflow)
- **Second servo station** gets its own confirmation beam — no longer depends on a fixed offset from the first

---

## XC8 project configuration

These are **silicon-level decisions** that firmware must match or the system will produce unreproducible bugs (UART errors, interrupt priority violations, timer miscalibration).

### Oscillator: Fosc = 20MHz, external HS crystal

| Parameter   | Value                  | Rationale                                                  |
|-------------|------------------------|------------------------------------------------------------|
| Fosc        | 20 MHz                 | External crystal, HS mode (`#pragma config FOSC = HS`)     |
| Fcyc        | 5 MHz                  | Fosc/4 (instruction clock for PIC18)                       |
| Crystal     | 20 MHz, 18-20pF caps   | Standard HC-49/S crystal. Load caps per datasheet.         |

**Why 20MHz and not 8MHz internal:** At 8MHz internal, baud rate 115200 has ~8.5% error — well above the HC-05 tolerance (~2-3%). At 20MHz with `BRG16=1` (16-bit baud rate generator), error drops to ~0.9%. This prevents intermittent UART failures that are indistinguishable from "Bluetooth disconnected" bugs.

**UART config consequence:**
```c
// 115200 @ 20MHz, BRG16=1, BRGH=1 (high speed)
// SPBRGH:SPBRG = (Fosc / (4 * 115200)) - 1 = 42.4 → 42 (0.9% error)
#define BAUD_RATE      115200
#define SPBRG_VALUE    ((20000000 / (4 * BAUD_RATE)) - 1)   // = 42
```

### Interrupt priority mode: IPEN = 1

By default (`RCONbits.IPEN = 0`), all interrupts share a single vector at 0x0008. The "fixed high priority" of INT0 / E-stop **only exists in silicon** when IPEN = 1 (two-vector priority mode).

**Required configuration:**

```c
RCONbits.IPEN = 1;      // Enable priority interrupt mode
INTCONbits.GIEH = 1;    // Enable high-priority interrupts
INTCONbits.GIEL = 1;    // Enable low-priority interrupts

    // E-stop (INT0) is always high priority by hardware — no IP bit exists.
INTCON3bits.INT2IP = 0; // INT2 (encoder) → low priority
INTCONbits.INT0IE = 1;  // Enable E-stop interrupt
    INTCON2bits.INTEDG0 = 0;    // falling edge — fail-safe (NC button + pull-up on RB0)

// CCP2 special event must target TMR1, not TMR3
T3CONbits.T3CCP2 = 0;   // CCP2 → TMR1

// TMR0 16-bit mode: reload value = 64911 (exceeds 8-bit max of 255)
T0CONbits.T08BIT = 0;   // 16-bit mode

// Explicit two-vector ISR scheme:
void __interrupt(high_priority) isr_high(void) {
    // INT0 only — E-stop (takes precedence over everything)
}

void __interrupt(low_priority) isr_low(void) {
    // INT2 (encoder), TMR0 (tick/debounce), CCP2 (servo compare),
    // USART (BT RX/TX), MSSP (I2C polling assist)
}
```

**Timer prescalers at 20MHz / 5MHz Fcyc:**

| Timer | Desired period | Prescaler | Period register | Effective |
|-------|---------------|-----------|----------------|-----------|
| TMR0  | 1 ms          | 1:8       | 65536-625 = 64911 | 1 ms (5MHz/8 = 625kHz, reload = 65536-625, counts UP to overflow) |
| TMR1  | 20 ms (servo) | 1:2       | free-run 16-bit | 400ns/tick. CCP2 compare triggers at exact edge times |
| TMR2  | PWM base (H-bridge) | 1:1       | PR2 = 249     | 50 µs / 20kHz (CCP1) |
| USART | 115200        | BRGH=1    | SPBRG = 42    | 115200 (0.9% error) |
| INT2  | encoder edge  | N/A       | N/A           | Sampled from INT2 pin on edge |

**CCP2 Compare servo cycle (50Hz) — corrected CCP2CON values:**

`CCP2CON<3:0>` modes for Compare on PIC18F4550:
- `0b0100` = Compare: force output pin **HIGH** on match
- `0b0101` = Compare: force output pin **LOW** on match
- `0b0111` = Compare: special event trigger (resets associated timer, no pin change)
- `1xxx` = **PWM mode** (NOT Compare — 0b1000/0b1001 are invalid here)

Also: `T3CONbits.T3CCP2 = 0` ensures CCP2 is mapped to **TMR1**, not TMR3.

**Two-phase cycle (no single-register auto-toggle exists):**

```
TMR1 free-runs at 400ns/tick (prescaler 1:2, Fcyc=5MHz)
20ms frame = 50,000 ticks

Phase A — start of pulse (software):
  1. RC1 set HIGH by software
  2. CCP2CON = 0b0101 (force LOW on match), CCPR2 = pulse_width_ticks

Phase B — on first match (hardware):
  3. Hardware forces RC1 LOW automatically → no jitter
  4. ISR fires:
     a. CCP2CON = 0b0111 (special event, reset TMR1), CCPR2 = frame_ticks
     b. [No pin change — hardware doesn't touch RC1 in mode 0b0111]

Phase C — on second match (hardware):
  5. TMR1 reset to 0 by hardware
  6. ISR fires:
     a. RC1 set HIGH by software (next frame starts)
     b. CCP2CON = 0b0101, CCPR2 = new_pulse_width_ticks
     → back to Phase A
```

Only **Phase B is fully hardware-timed** (the critical falling edge). Phase A and C have software latency but it's on the rising edge (not critical — tolerance is ~100µs).

Pulse width in ticks (1 tick = 400ns):
- 1.0ms (0°)  = 2500 ticks
- 1.5ms (90°) = 3750 ticks
- 2.0ms (180°)= 5000 ticks

Resolution: 1 tick = 400ns ≈ 0.07°. The `SERVO <1|2> <0-180>` command maps linearly to 2500-5000 ticks.

---

## Firmware architecture

### State machine

```
IDLE ──START──▶ RUNNING ──detect──▶ SORTING ──done──▶ RUNNING
  ▲                                    │
  └────────────STOP────────────────────┘
  ◀─────────────────────────────────────
ERROR ◀─────────jam detected────────────
```

- **IDLE:** Belt stopped. LCD shows menu. Awaiting START (BT or button).
- **RUNNING:** Motor active. TCS34725 polled every ~100ms. Color samples integrated and compared against calibrated thresholds.
- **SORTING:** Object identified. Transit time calculated from belt speed. Servo activated at calculated ETA, held for dwell window, then returned. Telemetry sent: `DETECT:<color>`.
- **ERROR:** Jam detected (servo stalled or obstruction). Motor stopped. BT notifies `JAM`. Requires manual reset.

### Timing and interrupts

| Timer  | Period        | Purpose                               |
|--------|---------------|---------------------------------------|
| TMR0   | 1 ms          | System tick, button debounce, speed window |
| TMR1   | free-run 16b  | CCP2 Compare timebase (400ns/tick)    |
| TMR2   | 50 µs         | CCP1 PWM base (20kHz H-bridge)        |
| CCP2   | Compare mode  | Servo 1 edge timing (auto-toggle RC1) |
| INT2   | ext event     | Encoder pulse count (edge triggered)  |

Main loop: state machine dispatch, LCD refresh (~50ms), BT buffer processing, telemetry send (~500ms).

### Synchronization (servo-belt)

When `DETECT:<color>` fires at the sensor station, the PIC computes object transit time to the first servo:

```
transit_ms = distance_mm / belt_speed_mm_per_ms
```

**`belt_speed_mm_per_ms` is NOT a constant** — it is continuously recalculated from the encoder pulse rate (pulses/s from the optical encoder, multiplied by `circumference_mm / pulses_per_rev`). This automatically compensates for load, friction, and battery voltage changes (issue #1).

The servo arm triggers at `transit_ms - safety_margin`. The safety margin accounts for encoder quantization and is tuned empirically (start at ~100ms, adjust if objects arrive early/late).

For the second servo station: the second **break-beam IR sensor** confirms actual object arrival. The PIC waits for the break-beam trigger, not a calculated offset. If the beam doesn't trigger within `transit_ms + timeout_window` of the color detection, → JAM (issue #5).

### Anti-jam logic (issues #2, #3)

Three independent mechanisms:

1. **Encoder-based belt jam** — if encoder pulses stop while motor PWM is active for >500ms → JAM. Detects belt stall, motor seizing, or object jamming the belt itself (not just the servo).

2. **Break-beam servo jam** — when the PIC commands the servo, it expects the break-beam to clear (object passes through) within the servo dwell window + margin. If the beam remains broken after the window → JAM (servo failed to push object through, or object stuck at the chute entrance).

3. **Break-beam arrival timeout** — after a `DETECT:color` event, each servo station's break-beam must trigger within `transit_ms + 2*transit_ms` (generous timeout for heavy loads). If the beam never triggers → JAM (object lost, fallen off belt, or misdirected).

**Jam response:** STOP motor immediately, send `JAM:<source>` via BT (e.g. `JAM:belt`, `JAM:servo1`, `JAM:servo2`, `JAM:timeout_servo1`), enter ERROR state, flash LCD. Manual clear required (E-stop or power cycle clears after physical unblock).

---

## Bluetooth communication protocol

ASCII text, lines terminated with `\n`. Bidirectional.

### Commands (TUI → PIC)

```
START                   Start conveyor
STOP                    Emergency stop
SET_SPEED <0-255>       Motor PWM duty
CALIBRATE               Calibrate color sensor (white balance)
STATUS                  Request full status
SET_MODE <auto|manual>  Operation mode
SERVO <1|2> <0-180>     Manual servo positioning
SET_THRESHOLD <color> <Rmax> <Gmax> <Bmax>  Adjust color detection
SET_SPACING <pulses>                        Min encoder pulses between objects
```

### Telemetry (PIC → TUI, automatic ~500ms)

```
SPEED:<0-255>
COLOR:<R,G,B,C>
DETECT:<color_name>
STATE:<idle|run|sort|err>
SERVO:<1|2>:<angle>
JAM
CALIB_DONE
```

### Status response (PIC → TUI, on demand)

```
STATUS_RESP:<state>,<speed>,<mode>,<servo1_angle>,<servo2_angle>,<jams>
```

If Bluetooth disconnects, PIC continues with last known config. Telemetry is silently dropped. On reconnect, PIC immediately sends `STATE:<current>`.

---

## App TUI (Termux — Python + Textual)

### Screens

1. **Main dashboard:** state indicator, speed gauge, color readout, servo positions, connect/disconnect, event log.
2. **Config screen:** color thresholds per category, speed limits, servo calibration angles.
3. **Log screen:** scrollable event history with timestamps.

### Key behaviors

- Async Bluetooth I/O via `asyncio` + `pyserial` (or `bluedot` / `pybluez` equivalent in Termux).
- Reconnection retry every 3s on disconnect.
- No blocking operations — Textual runs on `asyncio` event loop.
- Commands sent on button press, not on every key.

---

### Minimum object spacing (issue #5)

Spacing is enforced by encoder pulses, not time. After `DETECT:color`, the system expects no new color detection until `min_spacing_pulses` have been counted from the encoder. If a second object arrives too soon:

- If servo station is free and both objects are low-priority: process sequentially (queue depth = 1).
- If queue is full: the second object passes through to an overflow chute (no color match triggered).
- The break-beam at each servo station acts as a final arbiter — no servo movement happens unless the beam is broken first.

`min_spacing_pulses` is configurable via `SET_SPACING <pulses>` from the TUI.

---

## Color sensor calibration

1. User places white object under sensor → `CALIBRATE` → PIC stores white balance correction.
2. User presents each color to classify → TUI prompts for label → PIC stores min/max RGB thresholds.
3. Thresholds persisted in PIC EEPROM (256 bytes available on PIC18F4550).

---

---

## Modo TEST y Calibración

### State machine addition

New state `TEST`, parallel to IDLE/RUNNING/SORTING/ERROR. Entered from IDLE via `TEST_ENTER` BT command or long-press of mode button (RD2). While TEST:

- `START` is rejected. No auto-operation.
- H-bridge defaults to IN1=IN2=0, PWM=0. It is **only** activated by explicit `TEST_MOTOR` command, and deactivated by:
  - `TEST_EXIT` (returns to IDLE, motor off)
  - A **2-second inactivity watchdog**: if no new `TEST_MOTOR` or keepalive arrives within 2s, firmware forces IN1=IN2=0 and PWM=0. This prevents runaway motor if BT disconnects mid-test.
- TEST_MOTOR is a **one-shot jog** — each invocation activates the motor for max 2s. To sustain motion, the TUI must re-send TEST_MOTOR before the watchdog fires (e.g. every 1s while a button is held).
- `TEST_EXIT` returns to IDLE. Motor stays off — no auto-start.

### BT commands (TEST only)

```
TEST_ENTER / TEST_EXIT
SERVO_SET <1|2> <angle>           Jog servo to angle (live, 0-180°)
SERVO_SAVE_HOME <1|2>             Save current angle as "home" to EEPROM
SERVO_SAVE_DEFLECT <1|2>          Save current angle as "deflect" to EEPROM
SERVO_GET_CONFIG <1|2>            Read stored config for servo
SET_DWELL <1|2> <ms>              Servo dwell time in ms
TEST_MOTOR <0-255> <fwd|rev>      Brief motor jog (ignored if H-bridge safety condition not met)
TEST_ENCODER_RESET                Reset encoder count to 0
TEST_ENCODER_READ                 Read current encoder count
TEST_BEAM <1|2>                   Read break-beam state
TEST_BUTTON_ECHO <on|off>         Enable/disable button echo telemetry
```

### New telemetry (TEST only)

```
SERVO_CONFIG:<1|2>:<home_angle>,<deflect_angle>,<dwell_ms>
ENCODER_COUNT:<pulses>
BEAM:<1|2>:<broken|clear>
BUTTON:<id>              (only if TEST_BUTTON_ECHO on)
```

### EEPROM persistence

| Address | Content | Size |
|---------|---------|------|
| 0x00    | Magic byte (0xA5) | 1 |
| 0x01    | White balance R | 2 |
| 0x03    | White balance G | 2 |
| 0x05    | White balance B | 2 |
| 0x07    | Servo 1 home angle (deg) | 2 |
| 0x09    | Servo 1 deflect angle (deg) | 2 |
| 0x0B    | Servo 1 dwell (ms) | 2 |
| 0x0D    | Servo 2 home angle (deg) | 2 |
| 0x0F    | Servo 2 deflect angle (deg) | 2 |
| 0x11    | Servo 2 dwell (ms) | 2 |
| 0x13    | Encoder pulses_per_rev | 2 |
| 0x15    | Number of stored colors (N) | 1 |
| 0x16+   | Color thresholds | 12 × N |

**Color threshold structure** (12 bytes per entry):
```
offset 0: R_min (uint16)
offset 2: R_max (uint16)
offset 4: G_min (uint16)
offset 6: G_max (uint16)
offset 8: B_min (uint16)
offset 10: B_max (uint16)
```
Total = 6 fields × 2 bytes = 12 bytes per color.

**Color label/name** is stored only on the TUI side (keyed by index). The PIC firmware stores min/max thresholds only.

**Maximum supported categories:** 4 colors (48 bytes) leaves 187 bytes free for future use. 8 colors (96 bytes) is the hard upper limit with 235 remaining bytes.

**Budget check:** Fixed fields = 21 bytes (0x00-0x14). Remaining = 235 bytes. At 12 bytes/color: 19 colors max — but firmware caps at 8 for YAGNI.

Defaults if EEPROM magic byte is missing: servo home=90°, deflect=0°, dwell=500ms, pulses_per_rev=20, N=0.

### LCD calibration menu (on-device, no BT)

Cyclic editor: mode button (RD2) cycles through fields, up/down (RD5/RD6) adjusts value in **real time** (moves the actual servo via CCP2 while adjusting), long-press mode saves to EEPROM and advances.

Fields:
1. Servo 1 Home angle
2. Servo 1 Deflect angle
3. Servo 1 Dwell (ms)
4. Servo 2 Home angle
5. Servo 2 Deflect angle
6. Servo 2 Dwell (ms)
7. Encoder pulses_per_rev

### TUI "Test & Calibration" screen

Separate screen from dashboard/config/log. Shows:
- Beam state indicators (broken/clear) for each station — live update
- Encoder count live counter
- Button echo display (if enabled)
- Controls for all `TEST_*` commands

Purpose: diagnose wiring without depending on the physical LCD.

---

## Conventions

- C99, `-O0`, dwarf-3, COFF+ELF hybrid output.
- No peripheral library linked.
- Python TUI uses Textual ≥0.41.
- Bluetooth: HC-05 in AT mode for config, data mode for communication. **Baud rate fixed at 115200** (issue #6). This is set once via AT commands during HC-05 setup and matched in both firmware UART config and Python `pyserial`.
- **Fosc = 20MHz** external HS crystal (`#pragma config FOSC = HS`). Required for 115200 UART accuracy.
- **BRG16=1, BRGH=1** for USART. SPBRG = 42.
- **IPEN = 1** (two-vector priority mode). E-stop on INT0 (high vector), all other interrupts on low vector (0x0018).
- **TMR0: 16-bit mode** (`T08BIT = 0`). Reload value 64911 exceeds 8-bit range. TMR0 counts UP to 0xFFFF overflow.
- **Servo 1 via CCP2 Compare mode** — TMR1 free-runs at 400ns/tick (prescaler 1:2). CCP2 matches at exact edge times, auto-toggles RC1. Resolution ~0.07°. **Servo 2 (optional)** via software PWM with TMR1 ISR.
