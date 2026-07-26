# Vehicle Window Control & Fault Detection (Dual ECU, ATmega32)

A distributed embedded system built on two ATmega32 microcontrollers that
communicate over UART. One ECU handles the vehicle hardware — window motors,
temperature and proximity sensing — while the other handles the driver
interface. Faults are recorded to external EEPROM so they survive a power
cycle, which is the point of the whole exercise.

> Embedded Systems Diploma project — Edges For Training.

---

## Architecture

```
        ┌─────────────────────┐                ┌─────────────────────┐
        │      HMI ECU        │     UART       │    CONTROL ECU      │
        │    (ATmega32)       │◄──────────────►│     (ATmega32)      │
        │                     │  9600 baud     │                     │
        │  • 4x4 Keypad       │                │  • 2x Window motors │
        │  • 16x2 LCD         │  Commands ───► │  • LM35 (ADC)       │
        │  • Timer1 CTC tick  │  ◄─── Data     │  • Ultrasonic (ICU) │
        │                     │                │  • EEPROM (I2C)     │
        │  Issues commands    │                │  Reads sensors,     │
        │  Displays results   │                │  logs faults        │
        └─────────────────────┘                └─────────────────────┘
```

Splitting the system this way means the control ECU can keep sampling sensors
and driving motors on a tight loop without ever being blocked by LCD writes or
waiting on keypad input — a display refresh in the same loop as a motor control
decision is exactly the kind of coupling this architecture avoids.

---

## Communication Protocol

The two ECUs speak a small byte protocol over UART.

**Commands (HMI → Control)**

| Code | Command | Effect |
|---|---|---|
| `0x10` | START_OPERATION | Begin sensor sampling and fault monitoring |
| `0x20` | SEND_VALUE | Return current sensor readings and window states |
| `0x30` | SEND_ERRORS | Return stored fault flags from EEPROM |
| `0x40` | STOP_SENSING | Halt sampling |

**Handshake bytes**

| Code | Meaning |
|---|---|
| `0x55` | READY_TO_SEND — sender is about to transmit a payload |
| `0xAA` | ACK — receiver has processed the previous byte |

The handshake matters because UART has no built-in flow control here. Without
it, the control ECU can push a data frame while the HMI is still busy writing
to the LCD, and bytes get dropped silently.

---

## Control ECU

Handles all vehicle-side hardware.

| Function | Implementation |
|---|---|
| Window actuation | Two DC motors, four buttons (open/close per window) |
| Temperature sensing | LM35 via ADC, overheat threshold at 90 °C |
| Proximity sensing | Ultrasonic via Input Capture Unit, minimum distance 10 cm |
| Fault storage | External EEPROM over TWI/I2C |
| Communication | UART slave responding to HMI commands |

**Fault logging.** When a reading crosses a threshold, the corresponding flag is
written to a fixed EEPROM address — `0x70` for overheat, `0x80` for proximity.
Because this is external non-volatile memory rather than RAM, the fault record
survives a power cycle. That is what makes it a fault *detection* system rather
than just a live readout: a technician can query faults that occurred while the
vehicle was running, after it has been switched off.

**Drivers:** `adc`, `pwm`, `icu`, `twi`, `uart`, `gpio` (MCAL);
`lm35_sensor`, `ultrasonic`, `external_eeprom`, `Motor`, `Motor2` (HAL).

---

## HMI ECU

Handles the driver interface.

| Function | Implementation |
|---|---|
| Input | 4x4 keypad, menu-driven |
| Display | 16x2 LCD |
| Timing | Timer1 in CTC mode, 1-second tick |
| Communication | UART master issuing commands |

Timer1 is configured with a compare value of 7812 — with an 8 MHz clock and a
1024 prescaler this gives a one-second interrupt, which drives both the periodic
display refresh and the ten-second sensor polling cycle.

**Drivers:** `gpio`, `timer`, `uart` (MCAL); `keypad`, `lcd` (HAL).

---

## Repository Layout

```
control-ecu/          Control ECU firmware — sensors, motors, EEPROM
hmi-ecu/              HMI ECU firmware — keypad, LCD, menu logic
final_project.pdsprj  Proteus simulation of both ECUs wired together
```

Both ECUs share `gpio`, `uart`, `std_types.h` and `common_macros.h`. The copies
are kept separate per ECU because each builds as its own independent Eclipse
project.

---

## Build & Simulate

Each ECU is a separate build.

1. Create two Eclipse AVR projects targeting the ATmega32.
2. Import `control-ecu/` into one and `hmi-ecu/` into the other.
3. Set `F_CPU` to 8000000 in both, matching the Timer1 compare value.
4. Build each to produce its own `.hex`.
5. Open `final_project.pdsprj` in Proteus, load each `.hex` into its
   corresponding ATmega32, and run.

---

## What I Learned

<!-- TODO: 2-3 honest sentences.

Good material here: why the ACK handshake was necessary and what went wrong
without it, debugging UART framing between two boards, or getting the TWI
timing right for the EEPROM. Something that actually cost you time is worth
more than a feature summary. -->

---

## Author

**Youssef Yassin** — Electrical & Electronics Engineering
[LinkedIn](https://linkedin.com/in/youssef-el-bendary/)
