# V1 Spec — “Granular DJ Workstation” (A/B Live Granular Decks)

This project is a proof-ladder build toward a live-performance granular instrument with DJ-style controls.

## Core identity
A **DJ-style live granular workstation** where the “decks” are **two live granular engines** (A/B).

## Engine architecture (V1)
- **2 engines:** A and B
- Each engine has its own **5-second circular buffer**
- **Always-recording** behavior:
  - A single **record bus** targets **A or B** (one at a time)
  - **Record follows selection immediately** (no separate arm step for V1)
- **Position model (live):** “time-back” from now (relative to the write head)
  - **biased toward near-now** for better performance resolution
- **Position meaning:** Position sets a **center point**; grains start around it (spray later)
- **Macros per engine (V1):**
  - Position / Density / Size

## Freeze behavior (per engine)
- **Freeze A** and **Freeze B** are **dedicated buttons** (per-engine)
- Freeze = **stop buffer write** (capture a snapshot)
- When frozen:
  - Position becomes **absolute scrub** inside the captured 5s buffer
- Unfreeze:
  - resume recording immediately (no wipe/clear)
- Freeze has priority:
  - if the record bus is pointed at a frozen engine, it **does not record**

## Mixer / performance model
- Core gesture: **A/B performance**
  - record A live, freeze B, crossfade between them (and vice versa)
- **Faders:** A, B, Main
- **Crossfader:** A ↔ B (main mix)
- **Cue (headphones):** DJ-style cueing
  - V1 cue logic: **Option 1 (“last pressed wins”)**
  - Cue buttons will be **LED pushbuttons** under each track fader

## LED semantics (V1 prototype)
- Prototype LEDs focus on **engine state**, not macro selection:
  - LED1 = A Frozen (on/off)
  - LED2 = B Frozen (on/off)
  - LED3 optional / system status

## Larger vision (V2+ scope)
- 4 outputs + headphone/cue output
- 2 combo mic pres (Hi-Z capable)
- 2 stereo line inputs
- Reverb (later phase)
- Ideally 3–4 engines (but V1 is 2)
- Possible future mode: each engine can become a “digital reel/tape deck” style looper

## Open questions (parked)
- Exact grain engine details (grain count, interpolation, window, etc.)
- How “spray” and “pitch/rate” enter the macro set (V1.1)
- How scenes/presets map onto the DJ surface in hardware