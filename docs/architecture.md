# System architecture

## Purpose

A small, low-power microcontroller (e.g. Arduino-class) is wired **inline** with the existing camera trigger cable:

1. **Inputs** from the motion trigger electronics: half-press (HP) and full-press (FP) lines.
2. **Outputs** to the camera: HP and FP lines (same electrical semantics as a physical remote — switch closure, typically active-low).

The MCU does not replace the Camtraptions controller; it **conditions timing** so the camera sees stable focus-wake and shutter pulses despite irregular PIR behavior.

## Actors (swim-lane owners)

| Lane | Role |
|------|------|
| **Motion trigger system** (Camtraptions / contraptions) | Two sensing paths: wide PIR (wake) and narrow PIR (subject in frame). Emits HP and FP on separate logical inputs. External PIR gap/lockout is configured on the trigger unit, not invented by the MCU. |
| **Timing MCU** | Debounce inputs, latch HP, enforce minimum pre-shutter focus time, run **sequences** (`FrameCount` frames per accepted FP), cap **sequences** per activity (`MaxSequenceCount`), release HP on timeout. |
| **Camera** | Receives HP (AF / wake) and FP (shutter). Assumed to remain in **single-shot** drive mode; the MCU synthesizes burst timing by issuing multiple FP pulses. |

## Signal semantics

### Half-press (HP) — “wake”, not a stable camera button

- The **wide-angle PIR** produces periodic, **temporally unstable** pulses on the HP input.
- These are interpreted as **wake / pre-focus**, not as “the user is holding half-press.”
- On wake, the MCU **asserts and latches** camera HP so the body stays in AF, expecting FP soon.

HP may **not** precede FP in every real event (animal can enter narrow FOV quickly). The MCU must handle FP-without-prior-HP and FP-during-burst.

### Full-press (FP) — shutter request

- The **narrow FOV PIR** (or equivalent) asserts FP when a target is in the capture zone.
- FP may arrive **before** sufficient HP lead time; the MCU may need to assert HP and **wait** up to `minHalfPressBeforeShutter` before the first FP output pulse (or any later pulse only if HP had been dropped).
- Each accepted FP event contributes to a **burst schedule** (see [behavior-spec.md](behavior-spec.md)).

## External vs MCU configuration

| Setting | Where configured | Notes |
|---------|------------------|-------|
| PIR gap / lockout after trigger | **MCU** (`fullPressIgnoreGap`) | PIR **Gap = minimum (0.5 s)**; see [pir-sensor-settings.md](pir-sensor-settings.md) |
| Wide PIR = wake only | **PIR** menu | Wide sensor **Wake** mode — HP only |
| Narrow PIR = shutter trigger | **PIR** menu | Far sensor **Normal** — FP |
| Burst frame count, inter-frame delay, HP timeouts | **MCU parameters** | See [parameters.md](parameters.md) |
| Camera drive mode (single vs continuous) | **Camera menu** | Not an MCU parameter; camera stays single-shot; MCU creates burst |

## Electrical assumptions (verify per harness)

- **Input polarity:** typically active-low (line shorted to ground when active).
- **Output drive:** open-drain or opto-isolated — mimic a mechanical switch, do not drive supply into the camera body.

Downstream integrators should confirm polarity and pin mapping on the actual inline adapter PCB.
