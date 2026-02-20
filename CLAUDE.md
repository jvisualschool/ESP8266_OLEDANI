# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP8266 (NodeMCU v2) firmware that drives a 128×64 SSD1306 OLED display with GIF-based animations and a Wi-Fi web control panel. Animations are stored as raw bitmap frames in C header files in `include/`.

## Build & Flash Commands

```bash
# Build firmware
pio run

# Build and upload to device
pio run --target upload

# Monitor serial output (74880 baud)
pio device monitor
```

## Configuration

Before building, copy `include/config.h.example` to `include/config.h` and fill in Wi-Fi credentials:

```bash
cp include/config.h.example include/config.h
# Edit WIFI_SSID and WIFI_PASSWORD
```

`config.h` is gitignored (it holds credentials and must not be committed).

## GIF → Header File Pipeline

Adding or updating animations requires converting GIFs to 64×64 PBM frame files, then to C headers:

1. **Resize a GIF to 64×64** and export individual frames as PBM (P1 format) into `frames/`, using a tool like ImageMagick or Ezgif.
2. **Run the conversion scripts** from the project root:
   ```bash
   # For the "idea" animation (uses frames/f_%03d-*.pbm naming)
   python3 convert_pbm_to_h.py

   # For line_chart, meteor-rain, social-media animations
   python3 convert_rest.py
   ```
   These regenerate the `include/animation_*.h` header files.

Frame naming conventions:
- Idea: `frames/f_001-<n>.pbm`
- Others: `frames/f_<anim-name>_<n>.pbm` (e.g. `frames/f_line_chart_000.pbm`)

## Architecture

### Display Layout

The screen is always split vertically at x=64:
- **Left half (0–63)**: 64×64 animation bitmap rendered via `display.drawBitmap()`
- **Right half (64–127)**: Text rendered by `drawWrappedTextRight()` — wraps at 64px width, centers vertically

### Animation Headers (`include/animation_*.h`)

Each header defines:
- `PROGMEM` 2D byte array of frames: `const uint8_t <name>_frames[][512]` (512 bytes = 64×64÷8)
- `#define <NAME>_FRAMES` — total frame count
- `#define <NAME>_WIDTH/HEIGHT` — always 64

All headers are included in `src/main.cpp`. Each animation cycles through its frames at ~25 FPS (40 ms interval).

### Web Server

- Runs on port 80; mDNS hostname: `billboard.local`
- `GET /` — serves the HTML control panel (embedded as a C string literal in `main.cpp`)
- `GET /update?msg=<text>&anim=<1-4>` — updates the displayed message and active animation
  - `anim` values: 1=Idea, 2=Line Chart, 3=Meteor Rain, 4=Social Media
  - Empty `msg` defaults to the animation's title

### State Machine (`loop()`)

- Before any `/update` command is received (`hasReceivedCommand == false`): shows IP address screen
- After receiving a command: switches to the selected animation with the provided message

### Hardware Pin Mapping

| Signal | GPIO | NodeMCU Pin |
|--------|------|-------------|
| I2C SDA | GPIO14 | D5 |
| I2C SCL | GPIO12 | D6 |
| OLED I2C address | 0x3C | — |

## Debug / Test Scripts

```bash
# Read serial output from device (port: /dev/cu.usbserial-10, baud: 74880)
python3 read_serial.py

# Ping both HTTP endpoints via mDNS
python3 test_ping.py

# Continuously send update requests (uses hardcoded IP — edit before use)
python3 test_loop.py

# Send one update and capture serial response
python3 test_monitor.py
```

> The hardcoded IP `192.168.0.111` in `test_loop.py` and `test_monitor.py` must be updated to match the device's actual IP shown on the OLED at boot.
