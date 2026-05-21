# Parameter registry

## MCU behavior & timing

| Parameter | AKA | Default | Units |
|-----------|-----|---------|-------|
| `wakeHalfPressHoldTime` | X | 10 | s |
| `wakeHoldRefreshPolicy` | — | `extend` | enum |
| `minHalfPressBeforeShutter` | T | 0.5 | s |
| `fullPressIgnoreGap` | — | 3.1 | s |
| `FrameCount` | N | 4 | frames |
| `MaxSequenceCount` | — | 4 | sequences |
| `StartFrameSpacingMin` | Y | 1.0 | s |
| `PostShutterHalfPressHoldTimeExtension` | Z | 2.0 | s |
| `shutterPulseDuration` | — | 100 | ms |
| `halfPressInputDebounce` | — | 20–50 | ms |
| `fullPressInputDebounce` | — | 10–30 | ms |

Half press is not dropped because of trigger activations during a burst (original §4b) → `halfPressDuringBurstPolicy = independent`.

## Parameter definitions

#### `wakeHalfPressHoldTime`

On a brief wide-PIR half-press (wake) with no full-press yet, the MCU asserts camera HP for this duration (SC-04). Each accepted sequence **extends** the remaining `wakeHalfPressHoldTime` from FP accept (R15). This is **not** a cap on total activity length — activity length is bounded by `MaxSequenceCount`, `FrameCount`, `StartFrameSpacingMin`, `PostShutterHalfPressHoldTimeExtension`, and sequence logic.

#### `wakeHoldRefreshPolicy`

Applies when debounced HP **input** arrives while wake/hold logic is active (SC-04b, SC-07b, SC-12), including post-burst hold. It does not change burst frame scheduling (R14) or sequence-accept gating.

**Default:** `extend`

| Value | Behavior |
|-------|----------|
| `extend` | Each debounced HP pulse **extends** the remaining `wakeHalfPressHoldTime` from that edge. Camera HP OUT stays on; the idle timeout moves later with each wake pulse. |
| `restart` | Each debounced HP pulse **restarts** a full `wakeHalfPressHoldTime` from zero. Camera HP OUT stays on; the timeout is always the full X seconds from the latest HP edge. |
| `ignoreWhileActive` | HP pulses refresh wake hold only before an activity starts; once activity is active, additional HP pulses do not move the deadline. |

#### `minHalfPressBeforeShutter`

**Gate before each shutter** — not extra spacing between frames. At each scheduled fire time, HP must have been on for at least this long (assert HP and wait only if the lead is short). Does **not** add on top of `StartFrameSpacingMin` when HP stays latched through the burst. See [behavior-spec § Burst frame scheduling](behavior-spec.md#burst-frame-scheduling-within-one-sequence).

#### `fullPressIgnoreGap`

After **sequence start**, FP **inputs** are ignored for this duration (R10). The PIR **Gap** menu value is a **minimum** — see [pir-sensor-settings.md](pir-sensor-settings.md). Default **3.1 s** is an estimate (recalculate when burst params change): `(FrameCount - 1) × StartFrameSpacingMin + shutterPulseDuration` (e.g. 3×1.0 + 0.1 s with registry defaults). Set longer if retriggers persist after the burst or gates stretch spacing.

#### `FrameCount`

Number of shutter (full-press output) activations per accepted trigger — one **sequence** (SC-01, SC-02, SC-03).

#### `MaxSequenceCount`

Maximum number of full-press **sequences** per activity. Limits by count, not wall-clock (SC-05, SC-09).

#### `StartFrameSpacingMin`

**Minimum** time from **start** of one FP OUT pulse to **start** of the next (R6), measured **rising-edge to rising-edge** on FP OUT pulse starts. Does **not** include `shutterPulseDuration`. Actual spacing may exceed this if `minHalfPressBeforeShutter` or HP release delays the next start — e.g. when `PostShutterHalfPressHoldTimeExtension` is low/zero and T > Y between sequences.

#### `PostShutterHalfPressHoldTimeExtension`

After the last shutter in a sequence, **extend** camera HP hold by this duration (R11). HP may stay latched for another sequence; accepting the next sequence may shorten the remainder (R15).

#### `shutterPulseDuration`

How long each FP output pulse is held (camera single-shot; the MCU generates pulses).

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
| `holdUntilActivityEnd` | Camera HP OUT must **not** be released solely because `wakeHalfPressHoldTime` expired while the activity is still active (any sequence in progress, between sequences, or waiting for another FP under `MaxSequenceCount`). HP is released when the activity ends (cap reached, no further sequences, and idle/timeout logic completes). |

Only this value is currently implemented. Other policy encodings are reserved and coerced to `holdUntilActivityEnd` on write/load.

#### `fpAfterMaxSequenceCountPolicy`

Applies when `sequencesStartedThisActivity >= MaxSequenceCount` and another debounced FP **input** arrives (SC-09). Separate from R10 (intra-sequence FP ignore during `fullPressIgnoreGap`).

**Default:** `ignoreUntilActivityEnd`

| Value | Behavior |
|-------|----------|
| `ignoreUntilActivityEnd` | Further FP inputs **do not** start a new sequence and **do not** add frames. Activity continues until normal end conditions (HP released, idle). SC-10: after that activity ends, the next FP starts a **new** activity with `sequencesStartedThisActivity = 0`. |
| `endActivityImmediately` | On FP after the cap, **end the activity now**: stop accepting new sequences, run activity teardown, and release camera HP OUT without waiting for further idle timeout. **Not** the registry default. |

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
| `wakeHalfPressHoldTime` | SC-04, SC-12; extended per sequence (R15) |
| `wakeHoldRefreshPolicy` | SC-04b, SC-07b, SC-12 |
| `minHalfPressBeforeShutter` | SC-01, SC-05, SC-06, SC-08 |
| `fullPressIgnoreGap` | SC-02, SC-03 |
| `FrameCount` | SC-01, SC-02, SC-03 (frames per sequence) |
| `MaxSequenceCount` | SC-05, SC-09 |
| `StartFrameSpacingMin` | SC-01, SC-11 |
| `activityHalfPressHoldPolicy` | SC-03, SC-07 |
| `fpAfterMaxSequenceCountPolicy` | SC-09 |
| `halfPressDuringBurstPolicy` | SC-07 |
| `fullPressWithoutPriorHpPolicy` | SC-06, SC-08 |
| `PostShutterHalfPressHoldTimeExtension` | SC-01, SC-05b (per sequence) |
| `halfPressInputDebounce` | SC-13 |
| `fullPressInputDebounce` | SC-13, SC-14 |
| `powerSaveIdleMode` | SC-15 |

When revising, update [scenarios.md](scenarios.md) and [diagrams/](diagrams/) together.
