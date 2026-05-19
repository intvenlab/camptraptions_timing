# Camtraptions PIR v4 — deployment settings

> **Status:** Target configuration for use with the [inline timing MCU](architecture.md).  
> **Manual:** Camtraptions PIR v4, firmware **v1.19** (`pir_v4_manual_fw_v1_19.pdf`).  
> **MCU timing:** [parameters.md](parameters.md) — burst, gaps, and HP hold are **not** configured on the PIR when using this setup.

## Summary

| Goal | PIR role | MCU role |
|------|----------|----------|
| Early detection | **Wide** sensor → **Wake** (half-press only) | Latch camera HP; `wakeHalfPressHoldTime` |
| Shutter trigger | **Far** sensor → **Normal** (full press) | Debounce FP; run burst (`FrameCount`, spacing, `fullPressIgnoreGap`) |
| Retrigger / burst spacing | **Gap = minimum**; **NUM = 1** | `fullPressIgnoreGap`, `StartFrameSpacingMin`, etc. |
| Multi-frame capture | Do **not** use PIR NUM/FPS | MCU generates FP pulses |

**Operating mode:** **Still (S)** only. Video mode menus are not used for this product.

**Connection:** **Wired** from PIR shutter output to MCU trigger **inputs**; MCU outputs to the camera. Disable the PIR wireless transmitter unless you use wireless only between PIR and MCU (unusual).

---

## Settings registry

Values below are what we want on a deployed unit. Menu names match the PIR v4 screen labels. Hold-actions are noted where the manual describes them.

### Mode & still burst (MCU owns burst timing)

| Menu / item | Manual ref. | Value | Notes |
|-------------|-------------|-------|-------|
| **Set Mode** | Stills / Video mode | **S** (Still) | Home screen shows **S**. |
| **NUM** (Number) | Still Mode → Number | **1** | One PIR “shot” per detection; MCU issues the real burst. **Do not** set 2–6. |
| **FPS** (Frame rate) | Still Mode → FPS | **3.0** (or any) | Irrelevant when **NUM = 1**; leave at default or maximum. |

### Global — sensors

| Menu / item | Manual ref. | Value | Notes |
|-------------|-------------|-------|-------|
| **Wide** — sensitivity | Global → Wide Sensor | **Field tune** (start **10–12**) | Range 1–16. Higher = more sensitive. Adjust with far sensor disabled; use indicator light and flaps. |
| **Wide** — function | Hold **Up** or **Down** >2 s on Wide screen | **Wake** | Cycles Normal → Off → **Wake**. Sends **half-press only** for early approach. |
| **Far** — sensitivity | Global → Far Sensor | **Field tune** (start **8–10**) | Far lens is hotter than wide; start lower than wide if false triggers occur. |
| **Far** — function | Hold **Up** or **Down** >2 s on Far screen | **Normal** | Full trigger when subject is in the narrow zone. **Not** Wake. |

**Wide Wake caveat (manual):** Wake signals from a sensor in Wake mode are only sent if the PIR has **not** been active in the last **30 s**. Continuous activity can suppress extra wake pulses — acceptable; MCU holds HP during activity (R13, R15).

### Global — gap, wake, wireless

| Menu / item | Manual ref. | Value | Notes |
|-------------|-------------|-------|-------|
| **Gap** (Gap Time) | Global → Gap Time | **Minimum (0.5 s)** | Manual range 0.5 s–1 min; default 1 s. Use **lowest** value. Retrigger spacing during a burst is handled by MCU `fullPressIgnoreGap`, not PIR gap. |
| **Wake** (Wake Time) | Global → Wake Time | **OFF** | Hold Up/Down >2 s until OFF. Global wake delay is for slow cameras **without** our MCU; wide **Wake** mode + MCU HP latch replace this. |
| **Wireless Channel** | Global → Wireless Channel | **Same number as camera trap system** | Set to the trap’s **system number** (1–15). Must match the wireless receiver (and any other gear) on that trap — e.g. system **7** → channel **7**. Hold Up/Down >2 s → **OFF** only if wireless is not used on that unit. |
| **EXT WAKE** (Periodic External Wake) | Global → External Wake | **OFF** | Hold Up/Down >2 s until OFF. MCU + wide wake keep the camera responsive; periodic wake wastes power. |

### Global — clock & time windows

| Menu / item | Manual ref. | Value | Notes |
|-------------|-------------|-------|-------|
| **Set Time** (clock) | Global → Set Clock Time | **Set to local time** | Required if time windows are used. Retained when powered off. |
| **Time Mode** (Time Windows) | Global → Enable Time Windows | **OFF** (default) or **per deploy** | When ON, home screen shows clock + ON/OFF. |
| **Set On Time** | Window On Time | **—** | Only if Time Mode = ON (e.g. night-only: 18:00). |
| **Set Off Time** | Window Off Time | **—** | Only if Time Mode = ON (e.g. 06:00). |

### Video-only menus (not used)

| Menu / item | Value | Notes |
|-------------|-------|-------|
| **TIME**, **EXT TIME**, **MODE** (video trigger) | — | **N/A** — stay in **Still** mode. |

### Advanced — Custom Variables (C Vars)

Access: Home → hold **Left + Right** >2 s → C VAR menu. Leave at factory defaults unless field testing requires a change.

| C Var | Name (manual) | Value | Notes |
|-------|----------------|-------|-------|
| **0** | Sampling frequency / adaptive sensitivity | **8** | Default. Hold Up/Down >2 s: **Adaptive OFF** unless wind/vegetation false triggers persist. |
| **1** | Half-press length (before full press) | **OFF** | Default. MCU extends HP on camera side; keep PIR HP stub short unless testing AF-on-PIR without MCU. |
| **2** | Half-press length (after full press) | **OFF** | Default. |
| **3** | Full-press duration (still) — override | **OFF** (disabled) | Do not enable; would fight MCU `shutterPulseDuration`. |
| **4** | Gap between full-press signals (still) | **OFF** (disabled) | Do not enable; would fight MCU `StartFrameSpacingMin`. |
| **5** | Full-press duration (video) | — | N/A (Still mode). |
| **6** | Max video extension limit | — | N/A (Still mode). |
| **7** | Flash wake / fire signals | **0** | No flash signals from PIR unless you deliberately run flashes on a separate wireless channel. |
| **8** | Flash signal channel | **—** | Only if C Var 7 ≠ 0; must differ from camera channel. |
| **9** | Wireless power boost | **0** | Default. Set **1** only for wireless troubleshooting. |

---

## Field checklist

1. **Set Mode** = **S**.
2. **NUM** = **1**; confirm **FPS** does not matter.
3. **Wide** = **Wake** + tune sensitivity.
4. **Far** = **Normal** + tune sensitivity.
5. **Gap** = **0.5 s** (minimum).
6. **Wake** (global) = **OFF**.
7. **Wireless Channel** = **camera trap system number** (receiver on same channel).
8. **EXT WAKE** = **OFF**.
9. C Vars **3** and **4** = **OFF**; **7** = **0**.
10. Clock / time windows per deployment.
11. Verify firmware **1.19+**; run indicator-light aim test before sealing housing.

---

## Related docs

- [architecture.md](architecture.md) — signal roles (wide HP, narrow FP)
- [parameters.md](parameters.md) — MCU timing registry
- [scenarios.md](scenarios.md) — acceptance cases (SC-01, SC-02, SC-12, …)
