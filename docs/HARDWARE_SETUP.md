# Hardware Setup

Wiring reference for the 8051 random password generator. **These connections match the firmware in `src/password_generator.c`, which is the authoritative source.**

📌 For pin-accurate ASCII schematics, a system block diagram, the firmware flowchart and strobe timing, see **[`DIAGRAMS.md`](DIAGRAMS.md)**.

---

## 0. Two ways to build this

The firmware is identical either way — only how you get signals to the pins differs.

### Option A — 8051 trainer / development board (how this project was built)

Most college 8051 trainer boards (Vi Microsystems, Dyna, Scientech, and similar) bring each MCU port out to a labelled header, and carry a set of pre-wired peripheral blocks silkscreened for standard lab experiments — `ELEVATOR`, `TRAFFIC LIGHT`, `STEPPER MOTOR`, `SEVEN SEGMENT`, and so on. Each block is just a bank of switches or LEDs with its own pull-ups or drivers already fitted.

For this project you only need three things from the board:

| Need | On a trainer board |
|---|---|
| 8-bit LCD data bus | Ribbon from the **Port 1** header to the onboard 16x2 LCD, or to the LCD header |
| 3 LCD control lines | Three jumpers from the **Port 2** header (bits 4, 5, 6) to the LCD `RS`, `RW`, `EN` pins |
| 1 trigger button | One jumper from any momentary switch in a peripheral block to **P0.0** |

The switch block labelled `ELEVATOR` (or any other experiment block) works perfectly well as the trigger — the label only describes the experiment it was intended for, not what it is electrically. It is a momentary switch to ground, which is exactly what this project needs.

> ✅ **The trainer board usually solves the Port 0 pull-up problem for you.** Peripheral switch blocks on these boards normally have their own pull-up resistors fitted. That is very likely why this project works on P0.0 despite Port 0 having no internal pull-ups — see §4.

On a trainer board you can also skip **all** of §2 and §5: the crystal, reset circuit and regulated +5 V supply are already built into the board.

### Option B — Discrete build on breadboard or custom PCB

Wire everything from §2 through §5 yourself. This is the version documented in the project report and in [`DIAGRAMS.md`](DIAGRAMS.md).

---

## 1. Bill of Materials

*Required for Option B only. On a trainer board, most of this is already on the board.*

| Qty | Component | Specification |
|---|---|---|
| 1 | Microcontroller | AT89S52 (40-pin DIP) |
| 1 | IC socket | 40-pin DIP, recommended |
| 1 | Crystal | 11.0592 MHz |
| 2 | Ceramic capacitor | 33 pF |
| 1 | Electrolytic capacitor | 10 µF, 16 V (reset) |
| 1 | Resistor | 10 kΩ (reset pull-down) |
| 1 | LCD module | 16x2, HD44780 compatible |
| 1 | Potentiometer | 10 kΩ (LCD contrast) |
| 1 | Resistor | 220 Ω (LCD backlight current limit) |
| 1 | Push button | Momentary, normally open |
| 1 | Resistor | 10 kΩ (button pull-up — required, see §4) |
| 1 | Transformer | 230 V AC → 12 V AC step-down |
| 1 | Bridge rectifier | 1 A |
| 1 | Electrolytic capacitor | 1000 µF, 25 V (filter) |
| 1 | Voltage regulator | 7805 |
| 2 | Ceramic capacitor | 0.1 µF (regulator decoupling) |
| — | Misc | Breadboard or PCB, jumper wires, header pins |

---

## 2. Microcontroller Support Circuitry

*Already present on a trainer board — skip to §3.*

### Clock

| AT89S52 pin | Connection |
|---|---|
| XTAL1 (pin 19) | One leg of the 11.0592 MHz crystal, plus a 33 pF capacitor to ground |
| XTAL2 (pin 18) | Other leg of the crystal, plus a 33 pF capacitor to ground |

Keep the crystal and both capacitors as close to the pins as possible and keep their ground return short.

### Reset

| Node | Connection |
|---|---|
| RST (pin 9) | Junction of the 10 µF capacitor and the 10 kΩ resistor |
| 10 µF capacitor | Other terminal to +5 V |
| 10 kΩ resistor | Other terminal to ground |

This produces an active-high reset pulse on power-up that decays as the capacitor charges.

### Power and mode pins

| AT89S52 pin | Connection |
|---|---|
| VCC (pin 40) | +5 V |
| GND (pin 20) | Ground |
| EA/VPP (pin 31) | **+5 V** — required so the MCU executes from internal flash |

---

## 3. LCD Interface

The LCD is driven in 8-bit mode with the full data bus on Port 1.

| LCD pin | Name | Connect to | AT89S52 physical pin |
|---|---|---|---|
| 1 | VSS | Ground | — |
| 2 | VDD | +5 V | — |
| 3 | VEE / V0 | Wiper of the 10 kΩ contrast potentiometer (ends to +5 V and ground) | — |
| 4 | RS | **P2.4** | 25 |
| 5 | RW | **P2.5** | 26 |
| 6 | EN | **P2.6** | 27 |
| 7 | D0 | **P1.0** | 1 |
| 8 | D1 | **P1.1** | 2 |
| 9 | D2 | **P1.2** | 3 |
| 10 | D3 | **P1.3** | 4 |
| 11 | D4 | **P1.4** | 5 |
| 12 | D5 | **P1.5** | 6 |
| 13 | D6 | **P1.6** | 7 |
| 14 | D7 | **P1.7** | 8 |
| 15 | LED+ | +5 V through the 220 Ω resistor | — |
| 16 | LED− | Ground | — |

**Notes**

- `RW` is held low by the firmware, so it may alternatively be tied directly to ground to free P2.5.
- If the display powers up blank or shows solid black blocks, adjust the contrast potentiometer before suspecting the wiring.
- Port 1 has internal pull-ups and drives the LCD data bus directly with no external components.
- On a trainer board, the LCD data ribbon goes to the **Port 1** header and the three control jumpers to **Port 2** bits 4–6. Contrast and backlight are normally already handled onboard.

---

## 4. Push Button

| Node | Connection |
|---|---|
| Button terminal A | **P0.0 — physical pin 39** |
| Button terminal B | Ground |
| Pull-up | 10 kΩ from P0.0 to +5 V |

> ⚠️ **Pin numbering.** On the 8051 DIP-40, Port 0 runs *backwards*: **P0.0 is pin 39** and P0.7 is pin 32. Wiring the button to pin 32 is a very common mistake.

### Why the pull-up matters

Port 0 is open-drain on the 8051 and has **no internal pull-ups** — unlike Ports 1, 2 and 3. The firmware writes `button = 1` at start-up, which only *releases* the pin; it does not drive it high. Without a pull-up the pin floats, and a floating input can read as a random 0 or 1, producing either no response at all or spurious passwords.

The pull-up can come from either source:

| Build | Pull-up source |
|---|---|
| **Trainer board** | Usually already fitted inside the peripheral switch block (`ELEVATOR`, `TRAFFIC LIGHT`, etc.). Nothing to add. |
| **Discrete build** | You must fit a 10 kΩ resistor from P0.0 to +5 V yourself. |

If you are on a trainer board and the button does nothing, check whether that switch block is pulled up before assuming a wiring fault — some blocks are pull-*down* and idle low, which this firmware would read as a permanently held button.

Debouncing is handled entirely in software (50 ms confirm-and-recheck), so no RC filter is needed in either case.

### On peripheral block labels

Silkscreen labels like `ELEVATOR` describe the lab experiment a block was laid out for, not what the hardware is. An elevator-experiment switch bank is electrically just momentary switches to ground, so borrowing one as this project's trigger is completely valid. Only the port bit the jumper lands on matters to the firmware, and that bit must be **P0.0**.

---

## 5. Power Supply

*Already present on a trainer board — skip to §6.*

```
230 V AC ──► Transformer (12 V AC) ──► Bridge rectifier ──► 1000 µF filter ──► 7805 ──► +5 V
                                                                                │
                                                                        0.1 µF decoupling
                                                                                │
                                                                    AT89S52 + LCD module
```

- Place 0.1 µF ceramic capacitors on both the input and output pins of the 7805, close to the device.
- The 7805 will warm up; fit a small heatsink if the board runs continuously.
- A USB 5 V supply or a regulated bench supply can substitute for the mains section during development and is safer for breadboard work.

> ⚠️ **Safety.** The transformer primary carries mains voltage. Insulate the primary side fully, never probe it while energised, and prefer a low-voltage supply for bench testing.

---

## 6. Flashing the Firmware

1. Build `src/password_generator.c` in Keil µVision with target **AT89S52** and **Create HEX File** enabled.
2. Remove the AT89S52 from the circuit, or use the trainer board's in-circuit programming header / bootloader utility.
3. Load `password_generator.hex` into your flash programmer utility and write it to the device.
4. Reseat the microcontroller, apply +5 V, and confirm `Press Key` appears on the LCD.
5. Press the button — eight characters should appear one after another.

---

## 7. Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| LCD completely blank | Contrast not biased, or VDD/VSS swapped | Adjust the potentiometer; verify pins 1 and 2 |
| Solid black blocks on both rows | LCD powered but never initialised | Check RS/RW/EN on P2.4–P2.6 (pins 25–27) and the Port 1 data ribbon |
| Garbled characters | Loose data-bus wire, or `EN` strobe too short | Reseat P1.0–P1.7 (pins 1–8); verify the crystal is oscillating |
| Nothing happens on button press | Missing pull-up on P0.0, wired to pin 32 instead of 39, or button not grounded | Add the 10 kΩ pull-up (or use a pulled-up trainer block); move the wire to **pin 39**; confirm the other terminal reaches ground |
| Passwords appear continuously without pressing | Button pin idling low — floating P0.0, or a pull-*down* switch block | Fit a pull-up to +5 V, or move the jumper to a pulled-up switch block |
| Multiple passwords from one press | Release wait skipped | Confirm the `while (button == 0);` loop and 200 ms delay are present |
| Occasional blank character in the password | `CHARSET_SIZE` is 70 but the pool holds 68 characters | Optional: change the macro to `68`, or use `sizeof(charset) - 1` |
| Program does not run at all | `EA/VPP` left floating or grounded | Tie pin 31 to +5 V |
