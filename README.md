# Granular Synth

Design → code → hardware → repeat.

A granular-synth learning project on **NUCLEO-F767ZI** using a proof ladder approach (control layer first, audio later).

## What you need (so far)
**Hardware**
- NUCLEO-F767ZI (or compatible Nucleo-144)
- 1× potentiometer (any value is fine, e.g. 10k) + jumper wires  
  - one outer leg → **3V3**
  - other outer leg → **GND**
  - wiper (middle) → **A0**

**Built-in controls**
- Uses the on-board **USER button** (this repo’s board tested on `PC_13` with `PullNone`)
- Uses the 3 on-board LEDs **LED1/LED2/LED3**

**Software**
- VS Code + PlatformIO
- Framework: MBed (PlatformIO)

## Proof ladder (current progress)
### ✅ Proof 1 — Pot → LED zones
- Read potentiometer on A0
- Map value into 3 ranges and display on LED1/2/3

### ✅ Proof 1.5 — “Instrument feel” controls
- Smoothing (one-pole low-pass)
- Clamp to 0..1
- 3-zone hysteresis (no flicker near boundaries)

### ✅ Proof 2 — Multi-parameter control layer
- Button cycles which parameter the pot controls (`activeParam = 0..2`)
- 2-flash LED indicator confirms a param switch
- Per-parameter memory: `params[3]`

### ✅ Proof 2.5 — Pickup / soft takeover
- Prevents parameter jumps when switching params
- Param changes only after the knob “catches” the stored value

### ✅ Proof 2.6 — Pickup feedback UI
- If a param isn’t picked up yet: slow blink all LEDs (“turn the knob to catch”)
- Once picked up: returns to normal zone display

## Build / upload
Open in VS Code with PlatformIO, then Build/Upload to the NUCLEO-F767ZI.

## Next up
- Proof 3: Scenes/presets (store/recall sets of params)
- Then: audio output path + DSP engine skeleton

## Repo tour
- `src/main.cpp` — current proof ladder implementation