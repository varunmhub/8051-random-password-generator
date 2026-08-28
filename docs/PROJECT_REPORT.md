# Random Password Generator using 8051 Microcontroller with LCD Display

**Microcontrollers (MC) Project – I**

Submitted in partial fulfilment of the requirements of the B.E. programme in Electronics and Communication Engineering.

| | |
|---|---|
| **Submitted by** | M Mohit Srinivasa (160123735040)<br>Muppidi Varun (160123735047) |
| **Under the guidance of** | N. Jagan Mohan Reddy, Assistant Professor, Dept. of ECE |
| **Department** | Electronics and Communication Engineering |
| **Institution** | Chaitanya Bharathi Institute of Technology (A), Gandipet, Hyderabad – 500075 |
| **Academic Year** | 2025–2026 |

---

## Abstract

Weak, human-chosen passwords remain one of the most common causes of account compromise. This project implements a standalone hardware random password generator on an AT89S52 microcontroller from the 8051 family. On each press of a push button, the system produces an 8-character password composed of uppercase letters, lowercase letters, digits and special symbols, and displays it on a 16x2 alphanumeric LCD.

Randomness is derived from the free-running Timer 0 counter registers, which are sampled at the exact moment the button press is confirmed. Because the timer advances roughly 921,600 times per second while human button presses have millisecond-scale jitter, the sampled value is effectively unpredictable across presses. The complete system is offline: it has no network interface and no non-volatile storage, so a generated password exists only on the display.

---

## Chapter 1 — Introduction

### 1.1 Background

Authentication systems still overwhelmingly rely on passwords. Users, left to their own devices, choose short strings built from names, dates and dictionary words, and reuse them across services. Software password managers solve part of this problem but introduce their own attack surface: they run on a general-purpose operating system, are usually network-connected, and store credentials in a vault that becomes a single point of failure.

A dedicated hardware generator sidesteps those concerns. With no OS, no network stack and no storage, the device's only output channel is a display that a human reads directly.

### 1.2 Problem Statement

Design and implement a low-cost embedded system that, on demand, generates a strong random password of fixed length from a mixed character set and presents it on a local display, without requiring a computer or an internet connection.

### 1.3 Objectives

1. Generate an 8-character password on each button press.
2. Draw characters from uppercase letters, lowercase letters, digits and special symbols.
3. Ensure consecutive presses produce different passwords.
4. Display the result on a 16x2 LCD interfaced to the microcontroller.
5. Keep the design entirely offline and buildable from standard lab components.
6. Provide a response time fast enough to feel instantaneous to the user.

### 1.4 Scope

The project covers circuit design, power supply design, firmware development in Embedded C, compilation and HEX generation in Keil µVision, flashing the AT89S52 and functional testing on assembled hardware. Cryptographic-grade entropy generation and secure credential storage are explicitly out of scope and are identified as future work.

---

## Chapter 2 — Literature Survey

| Approach | Mechanism | Limitation addressed by this project |
|---|---|---|
| Software password managers | Deterministic CSPRNG on a host OS, encrypted vault | Requires a trusted host; vault is a single point of failure; network exposure |
| Online password generators | Server-side generation, delivered over HTTPS | Password transits a network and is visible to the service operator |
| Linear Feedback Shift Register (LFSR) in firmware | Deterministic bit sequence from a fixed seed | Fully reproducible if the seed is known; identical output after every reset |
| Timer/counter sampling on a microcontroller | Free-running counter read at an externally triggered instant | Non-reproducible across presses without additional hardware — the approach adopted here |
| True hardware RNG (noise diode, ring oscillator jitter) | Physical noise source digitised by the MCU | Higher component cost and calibration effort; proposed as an upgrade path |

The timer-sampling technique was selected because it needs no additional components, keeps the bill of materials to what is available in a standard microcontrollers lab, and still satisfies the practical requirement that no two consecutive presses yield the same password.

---

## Chapter 3 — System Design

### 3.1 Block Diagram

```
  230 V AC
     |
  Step-down transformer
     |
  Bridge rectifier + filter capacitor
     |
  7805 regulator  ->  +5 V rail
     |
     +--------------------------+--------------------------+
     |                          |                          |
  Push button (P0.0)      AT89S52 MCU              16x2 LCD
                          11.0592 MHz XTAL      D0-D7 -> Port 1
                          Reset: 10 uF + 10k    RS/RW/EN -> P2.4/P2.5/P2.6
```

### 3.2 Hardware Components

| Component | Specification | Function |
|---|---|---|
| Microcontroller | AT89S52, 8 KB in-system programmable flash, 256 B RAM, 32 I/O lines, 3 timers | Executes the firmware |
| Crystal oscillator | 11.0592 MHz with two 33 pF capacitors to ground | Provides the system clock; the value gives exact standard baud rates and a convenient 921.6 kHz machine-cycle-derived timer rate |
| Reset circuit | 10 µF electrolytic capacitor in series with the RST pin, 10 kΩ pull-down | Generates a clean power-on reset pulse |
| LCD module | 16 characters x 2 lines, HD44780-compatible controller | Displays the idle prompt and the generated password |
| Push button | Momentary, normally open, active low | User trigger |
| Transformer | 230 V AC to 12 V AC step-down | Mains isolation and voltage reduction |
| Rectifier | Full-wave bridge with smoothing capacitor | AC to unregulated DC |
| Regulator | 7805 linear regulator | Stable +5 V for the MCU and LCD |
| Passives | Contrast potentiometer, current-limiting resistor for LCD backlight, jumper wires | Assembly and biasing |

### 3.3 Power Supply Design

230 V AC mains is stepped down by the transformer, rectified by the bridge, smoothed by the filter capacitor and regulated to a fixed +5 V by the 7805. The AT89S52 and the LCD module both operate directly from this rail. The regulator provides ample headroom for the modest combined current draw of the microcontroller and the LCD backlight.

### 3.4 Pin Assignment (as implemented in firmware)

| Signal | Pin | Direction | Notes |
|---|---|---|---|
| LCD D0–D7 | P1.0–P1.7 | Output | `#define LCD P1`, 8-bit data mode |
| LCD RS | P2.4 | Output | 0 selects the instruction register, 1 selects the data register |
| LCD RW | P2.5 | Output | Tied low in firmware; the LCD is only ever written to |
| LCD EN | P2.6 | Output | High-to-low transition latches the byte on the data bus |
| Button | P0.0 | Input | Written to 1 at start-up so the internal structure floats high; pressing pulls it to ground |

> **Documentation note.** The original written report describes the button on P1.0 and the LCD data bus on Port 2. The submitted firmware — reproduced verbatim in `src/password_generator.c` — uses Port 1 for data, P2.4/P2.5/P2.6 for control and P0.0 for the button. The firmware is authoritative and the hardware was wired to match it.

---

## Chapter 4 — Implementation

### 4.1 Development Environment

| Tool | Role |
|---|---|
| Keil µVision IDE with the C51 compiler | Editing, compiling and linking the Embedded C source; generating the Intel HEX output |
| `<reg51.h>` | Standard header exposing 8051 SFRs (`P0`–`P3`, `TMOD`, `TH0`, `TL0`, `TR0`) as C identifiers |
| Flash programmer utility | Writing the HEX image into the AT89S52's in-system programmable flash |
| Proteus / breadboard prototype | Verifying the LCD interface and button logic before soldering |

Build configuration: target device **Atmel AT89S52**, crystal frequency **11.0592 MHz**, **Create HEX File** enabled in the output options.

### 4.2 Firmware Architecture

The firmware is a single translation unit organised as a small set of layered routines:

| Routine | Responsibility |
|---|---|
| `delay(unsigned int ms)` | Blocking busy-wait built from nested loops with an inner count of 1275, giving roughly one millisecond per unit at 11.0592 MHz |
| `lcd_cmd(unsigned char cmd)` | Places a byte on the data bus with `RS = 0` and pulses `EN` to issue an LCD instruction |
| `lcd_data(unsigned char dat)` | Same strobe with `RS = 1` to write a displayable character |
| `lcd_string(unsigned char *str)` | Walks a null-terminated string, calling `lcd_data` per character |
| `lcd_init(void)` | Issues the initialisation sequence `0x38`, `0x0C`, `0x01`, `0x06` |
| `timer0_init(void)` | Sets `TMOD = 0x01` (Timer 0, Mode 1, 16-bit) and `TR0 = 1` to start free-running |
| `get_random(void)` | Returns `(TH0 + TL0) % CHARSET_SIZE` as an index into the character pool |
| `main(void)` | Initialises peripherals, shows the prompt, then polls, debounces, generates and waits for release in an infinite loop |

### 4.3 Character Set

```c
unsigned char code charset[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#$%&*";
```

26 uppercase letters, 26 lowercase letters, 10 digits and 6 symbols (`@`, `#`, `$`, `%`, `&`, `*`) — **68 characters** in total. The pool is declared in the `code` memory space so it occupies program flash rather than the scarce internal RAM.

### 4.4 Randomness Generation

Timer 0 runs in 16-bit mode with no reload and is never stopped. At 11.0592 MHz the 8051 machine cycle is twelve oscillator periods, so the timer increments approximately **921,600 times per second** — the pair `(TH0, TL0)` completes a full 65,536-count wrap roughly every 71 ms.

When a press is confirmed, `get_random()` reads both halves, adds them and reduces the sum modulo `CHARSET_SIZE`. Because the human press instant is not synchronised to the timer, and because 50 ms of delay separates each generated character, the eight indices produced within one press are drawn from widely separated points in the counter's cycle.

### 4.5 Debouncing and Control Flow

Mechanical contacts chatter for several milliseconds after closing. The firmware handles this in software rather than with an RC network:

1. Poll `P0.0` continuously; a low reading indicates a possible press.
2. Wait 50 ms, then re-read. If the pin is still low, the press is genuine; if not, it was noise and is ignored.
3. Clear the display (`0x01`), home the cursor (`0x80`), then emit the eight characters with a 50 ms gap between each.
4. Block in `while (button == 0);` until the user releases the button, then wait a further 200 ms before returning to the polling loop.

The release wait prevents a single sustained press from being interpreted as multiple triggers.

### 4.6 Timing Budget

| Phase | Duration |
|---|---|
| Debounce confirmation | ~50 ms |
| Character generation and display (8 x 50 ms) | ~400 ms |
| Post-release settling | ~200 ms |
| **Perceived press-to-password latency** | **~400 ms** |

---

## Chapter 5 — Results and Discussion

### 5.1 Functional Testing

On power-up the LCD reliably displayed the idle prompt `Press Key`. Each subsequent button press cleared the display and printed a fresh 8-character password.

### 5.2 Sample Outputs

Passwords captured from the assembled board during testing:

| Trial | Password | Composition |
|---|---|---|
| 1 | `A0hP$wdL` | Upper, digit, lower, symbol |
| 2 | `Q&vdKarZ` | Upper, symbol, lower |
| 3 | `JkSEz90%` | Upper, lower, digits, symbol |
| 4 | `41TA0hP9` | Digits, upper, lower |
| 5 | `MoWD3kRE` | Upper, lower, digit |
| 6 | `UB1iQ*we` | Upper, digit, lower, symbol |

A representative example quoted in the report is `mZ3@Kp#7`.

### 5.3 Observations

- **No repetition.** Across all trials, no two consecutive presses produced the same string, confirming that timer sampling supplies adequate variability for the intended use.
- **Character class mixing.** Every observed output drew from at least three of the four character classes without any explicit rule enforcing it, which follows from the composition of the pool.
- **Responsiveness.** Press-to-full-display latency of roughly 400 ms was perceived as immediate; the staggered per-character rendering also gave useful visual feedback that generation was in progress.
- **Display stability.** With the contrast potentiometer correctly biased, characters were crisp and stable, and no flicker or ghosting was observed during rapid repeated presses.

### 5.4 Limitations

1. **Not cryptographically secure.** The entropy source is a deterministic counter, not physical noise. An attacker with precise knowledge of the reset instant and the press timing could in principle reconstruct the sequence.
2. **`CHARSET_SIZE` off-by-two.** The macro is defined as `70` while the pool holds 68 characters. Indices 68 and 69 address the string terminator and the byte immediately after it, so a blank or unintended glyph can occasionally appear. Defining `CHARSET_SIZE` as `68` — or better, as `sizeof(charset) - 1` — corrects it.
3. **No storage.** The password is lost on the next press or on power-down, so it must be transcribed immediately.
4. **Display constraint.** A 16x2 LCD limits practical password length.
5. **Single output channel.** There is no way to transfer the password to a phone or computer other than manual entry.
6. **Fixed parameters.** Length and character classes are compile-time constants and cannot be changed at runtime.

---

## Chapter 6 — Conclusion and Future Scope

### 6.1 Conclusion

The project successfully demonstrates that a useful random password generator can be built on an 8-bit microcontroller with no specialised hardware. The AT89S52-based system reliably produced distinct 8-character passwords from a 68-character mixed pool, displayed them on a 16x2 LCD within roughly 400 ms of a button press, and did so entirely offline. Along the way it exercised the core skills targeted by the course: LCD interfacing in 8-bit mode, timer configuration and use, software debouncing of mechanical inputs, regulated power supply design, and the full Embedded C toolchain from source to flashed HEX.

### 6.2 Future Scope

| Enhancement | Description |
|---|---|
| True hardware RNG | Digitise avalanche/thermal noise or exploit ring-oscillator jitter to obtain physical entropy suitable for security-critical use |
| EEPROM storage | Retain generated passwords in non-volatile memory behind a PIN-protected retrieval menu |
| Configurable output | Runtime selection of password length and per-class inclusion via additional buttons or a keypad |
| UART interface | Stream the password to a PC terminal so it can be pasted rather than retyped |
| Wireless delivery | HC-05 Bluetooth or SIM900 GSM module to send the password to a paired phone or via SMS |
| IoT integration | Connect to a credential-provisioning service for managed multi-device deployments |
| OLED display | Replace the 16x2 LCD with a graphical OLED for longer passwords, strength indicators and a richer interface |
| Fix `CHARSET_SIZE` | Derive the modulus from the array length to eliminate the out-of-range index defect |

---

## References

1. Muhammad Ali Mazidi, Janice Gillispie Mazidi, Rolin D. McKinlay — *The 8051 Microcontroller and Embedded Systems: Using Assembly and C*, Pearson.
2. Atmel Corporation — *AT89S52 8-bit Microcontroller with 8K Bytes In-System Programmable Flash* datasheet.
3. Hitachi — *HD44780U (LCD-II) Dot Matrix Liquid Crystal Display Controller/Driver* datasheet.
4. Keil (ARM) — *C51 Compiler User's Guide / µVision IDE documentation*.
5. STMicroelectronics — *L7805 Positive Voltage Regulator* datasheet.
