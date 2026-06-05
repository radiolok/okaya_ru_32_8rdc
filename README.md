# Okaya RU-32-8-RDC VFD Terminal

Firmware for the Okaya RU-32-8-RDC vacuum fluorescent display module, turning it into a VT100-compatible serial terminal using an Arduino Nano (ATmega328P).

## Hardware Overview

| Component | Spec |
|-----------|------|
| Display | Okaya RU-32-8-RDC, 32 columns × 8 rows |
| Controller | Arduino Nano (ATmega328P @ 16 MHz) |
| Logic supply | 5 V (shared with Nano) |
| Anode supply | 160 V via boost converter |
| Boost inductor | 100 µH / 3 A |
| Output capacitor | 10 µF |
| HV feedback divider | 470 kΩ / 10 kΩ → 3.33 V at 160 V |
| UART baud rate | 38400 |
| Shift registers | 2× 74HC595 (addr + data bus, daisy-chained) |

## Pin Connections

| Nano Pin | ATmega Port | Direction | Display Signal | Function |
|----------|-------------|-----------|---------------|----------|
| D13 | PB5 | OUT | SCK (SHCP) | SPI clock → both 595 |
| D11 | PB3 | OUT | MOSI (DS) | SPI data → 595#1 Q7' → 595#2 |
| D10 | PB2 | OUT | SS | SPI slave-select (forced OUTPUT to stay master) |
| D2 | PD2 | OUT | STCP (RCLK) | Latch both 595 outputs |
| D3 | PD3 | OUT | NS7 — CLOCK | Character data latch (active HIGH pulse) |
| D4 | PD4 | OUT | NS8 — CS | Chip select (active LOW, held LOW during writes) |
| D5 | PD5 | OUT | ~AD — UL | Underline bit (active LOW: LOW = ON, HIGH = OFF) |
| D6 | PD6 | OUT | ~AS — AW | Address write strobe (active LOW pulse) |
| A4 | PC4 | **IN** | NS4 — READY | Display ready (active HIGH, polled with timeout) |
| A0 | PC0 | OUT | ~WR | Unused, held HIGH |
| A1 | PC1 | OUT | ~BL | Blanking (active LOW — blanks ROM output) |
| A2 | PC2 | OUT | ~RESET | Display RAM reset (active LOW pulse) |
| A3 | PC3 | IN | HV FB | HV feedback (ADC3) via 470k/10k divider |
| D9 | PB1 | OUT | HV PWM | Boost converter PWM (OC1A, 80 kHz) |
| D8 | PC5 | IN | BTN | Mode switch button (INPUT_PULLUP, LOW = pressed) |
| D0 | PD0 | IN | UART RX | Serial input |
| D1 | PD1 | OUT | UART TX | Serial output |

## Write Protocol

The protocol is derived from the reference implementation by serjsochi (radiokot.ru, 2024) and adapted for our board revision.

### Signal Definitions

| Signal | Nano | Polarity | Role |
|--------|------|----------|------|
| CLOCK (NS7) | PD3 | HIGH pulse | Latches character data + auto-increments address |
| CS (NS8) | PD4 | LOW = active | Chip select — held LOW for all writes |
| UL (~AD) | PD5 | LOW = ON | Underline attribute bit, latched with data on CLOCK |
| AW (~AS) | PD6 | LOW pulse | Address write — latches address bus A0–A7 |
| READY (NS4) | PC4 | HIGH = ready | Display ready flag, polled before each write |
| WR (~WR) | PC0 | — | Unused, held HIGH |
| BL (~BL) | PC1 | LOW = blank | Display blanking |
| RESET (~RESET) | PC2 | LOW pulse | Resets display RAM |

### Write Sequence (per character)

```
1. shift595_write(addr, data)     — shift 16 bits, pulse RCLK (STCP)
   Data byte goes first → ends up in 595#2 (D0–D5 bus)
   Addr byte goes second → ends up in 595#1 (A0–A7 bus)

2. strobe_delay()                 — wait for bus to settle (~1 µs)

3. wait_ready()                   — poll NS4 until HIGH or timeout (~200 ms)
   If timeout: proceed anyway to avoid hanging the firmware.

4. strobe_aw()                    — pulse ~AS LOW→HIGH
   Latches the address from A0–A7 into the display.

5. set_underline(on)              — set ~AD level (LOW = underline ON, HIGH = OFF)
   Must be set BEFORE strobe_clock() — the UL bit is latched together with D0–D5.

6. strobe_clock()                 — pulse NS7 HIGH→LOW
   Latches D0–D5 + UL into the display character RAM.
   Auto-increments the internal address counter.
```

### Batch Write (auto-increment)

After the first character (steps 1–6 above), subsequent characters in a contiguous block
only need steps 1–3 and 6 — AW is skipped because the display auto-increments its
internal address pointer after each CLOCK pulse:

```
for i in 1..N-1:
    shift595_write(addr + i, data[i])
    strobe_delay()
    wait_ready()
    strobe_clock()                — data latch + auto-increment
```

This is implemented in `display_write_batch()` and is used by `terminal_flush()`
when it detects contiguous dirty ranges in the terminal buffer.

### Timing

| Parameter | Value |
|-----------|-------|
| Strobe pulse width | ~1 µs (2 cycles at 16 MHz) |
| Inter-strobe gap | ~1 µs |
| READY poll interval | ~10 µs per iteration |
| READY timeout | ~200 ms (2000 iterations) |
| CS state during write | Held LOW continuously |
| WR state | Held HIGH always |

### 595 Chain Layout

```
MOSI (PB3) → 595#1.DS → 595#1.Q7' → 595#2.DS
SCK  (PB5) → 595#1.SHCP + 595#2.SHCP
RCLK (PD2) → 595#1.STCP + 595#2.STCP
```

- **595#1** (nearest to MOSI): receives addr byte → outputs A0–A7
- **595#2** (chained from #1): receives data byte → outputs D0–D5

SPI transfer order: `SPI.transfer(data)` first, `SPI.transfer(addr)` second.
The data byte shifts through 595#1 into 595#2 via Q7'.

### Address Map

Linear: `addr = col + row × 32` (0–255). Character code 0x20 = space.

### Hardware Notes

- **~AD is UL (Underline), not WIDE.** On our board revision, PD5 controls the underline
  attribute. If testing shows otherwise, the `set_underline()` function can be replaced
  with `set_wide()` — the signal function is determined by the display's ROM/decoder.
- **READY (NS4)** is defined as INPUT in the reference code but was never polled there.
  Our implementation polls it with a 200 ms timeout per character to avoid data loss
  while preventing firmware hangs if the pin is floating or grounded.
- **Auto-increment** is assumed to work as in the reference display. If the display
  does not auto-increment, `display_write_batch()` will need to be modified to assert
  AW on every write instead of just the first.

## Boost Converter

| Parameter | Value |
|-----------|-------|
| Frequency | 80 kHz (Timer1 Fast PWM, ICR1=24, prescaler 8) |
| Resolution | 25 steps (5-bit) |
| Regulation | P-controller @ 9.6 kHz (ADC free-running + ISR) |
| Soft-start | 1.5 s linear ramp, then regulator takeover |
| Timeout | 3 s to reach target, emergency shutdown on failure |
| Target ADC | 683 (10-bit, AVCC ref) = 160 V ÷ 480k × 10k = 3.33 V |

## Features

- **32×8** character buffer with dirty-tracking (partial refresh)
- **Insert/replace** modes
- **Scrolling** on overflow, controllable via `ESC[?4h` / `ESC[?4l`
- **Demo mode** — standalone slideshow with 7 animated text effects, 7 text screens stored in Flash
- **Mode switch** via button on PC5 (D8): long press (>1 s) toggles terminal ↔ demo, short press in demo switches effect
- **VT100 subset**:
  - `ESC[H`, `ESC[row;colH` / `ESC[row;colf` — cursor positioning
  - `ESC[nA` / `B` / `C` / `D` — cursor movement
  - `ESC[J` / `ESC[0J` / `ESC[1J` / `ESC[2J` — screen erase
  - `ESC[K` / `ESC[0K` / `ESC[1K` / `ESC[2K` — line erase
  - `ESC[4h` / `ESC[4l` — insert/replace mode
  - `ESC[?4h` / `ESC[?4l` — scroll enable/disable
  - `ESC c` — terminal reset
- **Control characters**: CR, LF, BS, TAB
- **Blanking** during full-screen refresh (no flicker)
- **Async regulator**: ADC interrupt-driven, no blocking in main loop

### Demo mode effects

| Effect | Description |
|--------|-------------|
| Typewriter | Line-by-line text output with 100–400 ms delay between lines; long texts scroll naturally |
| Top-down | Screen fills row-by-row from top |
| Bottom-up | Screen fills row-by-row from bottom |
| Left-right | Screen fills column-by-column from left |
| Right-left | Screen fills column-by-column from right |
| Diagonal | Screen fills diagonally from a random corner |
| Random | Pixels appear in random order with linear-scan fallback for last cells |

Effects auto-switch every 5–10 seconds. Short button press forces immediate switch to a new text + random effect.

## Build & Upload

Requires [PlatformIO](https://platformio.org/). Build:

```bash
pio run
```

Upload to connected Arduino Nano:

```bash
pio run --target upload
```

Monitor serial output:

```bash
pio device monitor --baud 38400
```

### Memory

| Resource | Used | Available |
|----------|------|-----------|
| Flash | 8316 B | 30.7 KB |
| RAM | 1045 B | 2048 B |

## Project Structure

```
okaya_vfd_terminal/
├── platformio.ini
├── src/
│   ├── main.cpp           # Setup, main loop (serial → parser → flush, demo mode dispatch)
│   ├── pins.h             # All pin definitions
│   ├── hal_595.h/cpp      # 74HC595 shift register driver
│   ├── hal_hv.h/cpp       # HV boost converter (Timer1 PWM + ADC ISR)
│   ├── display.h/cpp      # Display protocol: strobe sequencing
│   ├── terminal.h/cpp     # 32×8 buffer, dirty tracking, cursor logic, scroll control
│   ├── esc_parser.h/cpp   # VT100 ESC-sequence state machine
│   └── demo.h/cpp         # Demo mode: button handling, 7 text effects, PROGMEM text bank
```

## Known Limitations

- **Character ROM** is fixed — no custom characters. The ROM map should be verified on hardware.
- **~AD pin function** is assumed to be UL (Underline). If the display uses it as WIDE instead, `set_underline()` needs to be replaced with `set_wide()`.
- **Auto-increment** is assumed to work per the reference display. Verification on hardware pending.
- **~BL** implements binary blanking only (no PWM brightness control).
- **DEBUG_PATTERN** build flag (`platformio.ini`) enables a test loop that writes 0x55/0xAA alternating to all 256 addresses — useful for hardware debugging.

---

*This project was generated by [Kilo](https://kilo.ai) using `deepseek-v4-pro` model.*
