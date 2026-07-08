# Parameter registry

## MCU behavior & timing

| Parameter | AKA | Default | Units |
|-----------|-----|---------|-------|
| `wakeHalfPressHoldTime` | X | 10 | s |
| `wakeHoldRefreshPolicy` | — | `legacy-no-op` | enum |
| `minHalfPressBeforeShutter` | T | 0.5 | s |
| `fullPressIgnoreGap` | — | 5.0 | s |
| `FrameCount` | N | 4 | frames |
| `MaxSequenceCount` | — | 4 | sequences |
| `StartFrameSpacingMin` | Y | 1.0 | s |
| `PostShutterHalfPressHoldTimeExtension` | Z | 2.0 | s |
| `shutterPulseDuration` | — | 200 | ms |
| `halfPressInputDebounce` | — | 20–50 | ms |
| `fullPressInputDebounce` | — | 10–30 | ms |

Half press is not dropped because of trigger activations during a burst (original §4b) → `halfPressDuringBurstPolicy = independent`.

## Parameter definitions

#### `wakeHalfPressHoldTime`

On a brief wide-PIR half-press (wake) with no full-press yet, the MCU asserts camera HP for this duration (SC-04). Trigger activations do not refresh or extend this wake timer. HP release follows the rule: `max(initial HP assert + wakeHalfPressHoldTime, final frame release + PostShutterHalfPressHoldTimeExtension)`.

#### `wakeHoldRefreshPolicy`

Legacy field retained for backward compatibility. In current requirement-aligned behavior, debounced HP input does not move wake hold timing while HP is already asserted.

**Default:** `legacy-no-op`

| Value | Behavior |
|-------|----------|
| `extend` | Reserved legacy encoding. Current firmware behavior does not move wake hold deadline from HP refresh pulses. |
| `restart` | Reserved legacy encoding. Current firmware behavior does not move wake hold deadline from HP refresh pulses. |
| `ignoreWhileActive` | Reserved legacy encoding. Current firmware behavior does not move wake hold deadline from HP refresh pulses. |

#### `minHalfPressBeforeShutter`

**Gate before each shutter** — not extra spacing between frames. At each scheduled fire time, HP must have been on for at least this long (assert HP and wait only if the lead is short). Does **not** add on top of `StartFrameSpacingMin` when HP stays latched through the burst. See [behavior-spec § Burst frame scheduling](behavior-spec.md#burst-frame-scheduling-within-one-sequence).

#### `fullPressIgnoreGap`

After **sequence start**, FP **inputs** are ignored for this duration (R10). The PIR **Gap** menu value is a **minimum** — see [pir-sensor-settings.md](pir-sensor-settings.md). Default is **5.0 s**. With registry defaults (`FrameCount=4`, `StartFrameSpacingMin=1.0 s`, `shutterPulseDuration=0.2 s`), nominal burst budget is **3.8 s** (`(FrameCount - 1) × (StartFrameSpacingMin + shutterPulseDuration) + shutterPulseDuration`), leaving about **1.2 s** post-burst retrigger margin.

#### `FrameCount`

Number of shutter (full-press output) activations per accepted trigger — one **sequence** (SC-01, SC-02, SC-03).

#### `MaxSequenceCount`

Maximum number of full-press **sequences** accepted before cap timeout behavior engages. Limits by count, not wall-clock (SC-05, SC-09).

- Valid range: `1..64`
- Default: `4`
- On cap hit (`sequencesStartedThisActivity >= MaxSequenceCount`), firmware enters **TimeOut** and ignores FP/HP inputs for one burst budget `((FrameCount - 1) * (StartFrameSpacingMin + shutterPulseDuration)) + shutterPulseDuration`, then resumes normal acceptance.

#### `StartFrameSpacingMin`

**Minimum** time from **end** of one FP OUT pulse to **start** of the next (R6), measured **falling-edge to next rising-edge** on FP OUT. Start-to-start spacing therefore includes `shutterPulseDuration`. Actual spacing may exceed this if `minHalfPressBeforeShutter` or HP release delays the next start — e.g. when `PostShutterHalfPressHoldTimeExtension` is low/zero and T > Y between sequences.

Range: **10ms..30000ms**.

#### `PostShutterHalfPressHoldTimeExtension`

After the last shutter in a sequence, hold camera HP for this duration (R11). Final HP release is the later of wake timeout or this post-final-frame extension window.

#### `shutterPulseDuration`

How long each FP output pulse is held (camera single-shot; the MCU generates pulses).

Range: **10ms..30000ms**.

#### `halfPressInputDebounce`

Reject short glitches on HP in (SC-13).

#### `fullPressInputDebounce`

Reject short glitches on FP in (SC-13, SC-14).

#### `halfPressDuringBurstPolicy`

Applies while a burst is active (`remainingFrames > 0` or a shutter pulse is in progress) (SC-07, R14).

**Default:** `independent`

| Value | Behavior |
|-------|----------|
| `independent` | HP **input** during the burst is **ignored** for scheduling: does not release camera HP OUT, does not change `remainingFrames`, does not emit extra FP OUT, does not reset the burst timer. Camera HP remains under activity rules (R13). Matches “half press not dropped because of trigger activations during a burst” (original §4b). |

#### `fullPressWithoutPriorHpPolicy`

Applies when a debounced FP **input** is accepted and camera HP was **not** already latched by the MCU (cold start — SC-06, SC-08).

**Default:** `assertHpThenWait`

| Value | Behavior |
|-------|----------|
| `assertHpThenWait` | Assert camera HP OUT immediately (R3). Wait until `minHalfPressBeforeShutter` has elapsed since that assert (R4). Then run the burst schedule (`FrameCount`, R5–R6). If HP input arrives later during cold-wait or burst, normal HP/burst rules apply (R14 during burst). |

#### `activityHalfPressHoldPolicy`

Applies for the whole **activity** — from first accepted FP through the last sequence, including gaps between sequences and post-burst hold while another sequence may still start (R13, SC-03).

**Default:** `holdUntilActivityEnd`

| Value | Behavior |
|-------|----------|
| `holdUntilActivityEnd` | Camera HP OUT must **not** be released solely because `wakeHalfPressHoldTime` expired while the activity is still active (any sequence in progress or between sequences while under cap). HP is released when the activity ends through normal idle/timeout teardown. |

Only this value is currently implemented. Other policy encodings are reserved and coerced to `holdUntilActivityEnd` on write/load.

#### `fpAfterMaxSequenceCountPolicy`

Legacy compatibility byte retained in camera config layout. Runtime cap handling is now timeout-based and does not branch behavior by this field.

**Current behavior:** when cap is reached, firmware enters TimeOut for one burst budget `((FrameCount - 1) * (StartFrameSpacingMin + shutterPulseDuration)) + shutterPulseDuration`, ignores FP/HP inputs during that window, then resumes normal sequence acceptance.

#### `inputActivePolarity`

Runtime polarity switching is currently reserved. Firmware runs active-low input semantics and coerces non-default values to active-low on write/load.

**Default:** `active-low`

| Value | Behavior |
|-------|----------|
| `active-low` | Implemented default. Input is active when the line is pulled to ground (switch closure to common). |
| `active-high` | Reserved/no-op in current firmware build (stored value is coerced back to active-low). |

#### `outputDriveMode`

Runtime drive-mode switching is currently reserved. Firmware uses open-drain output behavior in state-machine mode.

**Default:** `open-drain` / opto (field wiring dependent)

| Value | Behavior |
|-------|----------|
| `open-drain` | Implemented default. Outputs pull the camera line active (typically to ground) when ON; high-Z when OFF. |
| `opto` | Reserved/no-op in current firmware build (stored value is coerced back to open-drain). |

#### `powerSaveIdleMode`

**Default:** `enabled`

| Value | Behavior |
|-------|----------|
| `enabled` | MCU enters low-power idle when no activity. First HP or FP after idle must wake firmware within the SC-15 budget: extra input-to-output latency versus `disabled` **&lt; 1 ms**. R4 (`minHalfPressBeforeShutter`) and other gates still apply once awake. |
| `disabled` | MCU stays fully awake between events. Higher standby current; baseline for SC-15 latency comparison. |

#### `statusLedMode`

Optional status LED on the MCU board (bench / field diagnostics). **Allowed values and default:** not yet assigned in this registry — define per product SKU when LED behavior is fixed (each value must state on/off/blink vs idle, activity, and fault).

PIR v4 menu values (wide/far modes, gap, NUM, C Vars, etc.) live in **[pir-sensor-settings.md](pir-sensor-settings.md)** — not in this MCU registry.

## Scenario traceability

| Parameter | Primary scenarios |
|-----------|-------------------|
| `wakeHalfPressHoldTime` | SC-04, SC-12; fixed from initial HP assert |
| `wakeHoldRefreshPolicy` | SC-04b, SC-07b, SC-12 |
| `minHalfPressBeforeShutter` | SC-01, SC-05, SC-06, SC-08 |
| `fullPressIgnoreGap` | SC-02, SC-03 |
| `FrameCount` | SC-01, SC-02, SC-03 (frames per sequence) |
| `MaxSequenceCount` | SC-05, SC-09 |
| `StartFrameSpacingMin` | SC-01, SC-11 |
| `TimeOut on MaxSequenceCount` | SC-09, SC-10 |
| `activityHalfPressHoldPolicy` | SC-03, SC-07 |
| `halfPressDuringBurstPolicy` | SC-07 |
| `fullPressWithoutPriorHpPolicy` | SC-06, SC-08 |
| `PostShutterHalfPressHoldTimeExtension` | SC-01, SC-05b (per sequence) |
| `halfPressInputDebounce` | SC-13 |
| `fullPressInputDebounce` | SC-13, SC-14 |
| `powerSaveIdleMode` | SC-15 |

When revising, update [scenarios.md](scenarios.md) and [diagrams/](diagrams/) together.
