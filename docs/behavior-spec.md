# Behavior specification

Normative description of what the timing MCU must do. **This document is the source of truth for requirements and behavior precedence.** Parameter names refer to [parameters.md](parameters.md). **Acceptance scenarios:** [scenarios.md](scenarios.md). Diagrams: [mcu-state-flow.md](diagrams/mcu-state-flow.md), [timing-sequences.md](diagrams/timing-sequences.md).

## Customer baseline requirements (source of truth)

The customer-provided requirements define the baseline behavior:

- A brief HP input pulse latches camera HP for `wakeHalfPressHoldTime` (X).
- If FP arrives and HP lead is less than `minHalfPressBeforeShutter` (T), delay only until T is satisfied, then fire.
- One accepted FP starts one sequence of `FrameCount` (N) shutter pulses with `StartFrameSpacingMin` (Y) spacing and `shutterPulseDuration` pulse width.
- After sequence completion, keep HP for `PostShutterHalfPressHoldTimeExtension` (Z), while wake/hold rules still apply.
- If another FP arrives while HP is still latched, accept it as soon as normal gates clear (R4, R10, R12). Do not require a new wide-sensor wake cycle.

Unless explicitly stated as an optional extension requirement, behavior must align with this baseline.

## Scenario index

| ID | Summary |
|----|---------|
| [SC-01](scenarios.md#sc-01--normal-wake-then-shoot) | Normal wake → FP → full burst → sleep |
| [SC-02](scenarios.md#sc-02--fp-during-sequence-ignored) | FP during `fullPressIgnoreGap` discarded |
| [SC-03](scenarios.md#sc-03--fp-flood-during-burst-ignored) | Repeated FP during burst; schedule unchanged |
| [SC-04](scenarios.md#sc-04--wake-timeout-no-fp) | HP only, timeout release |
| [SC-04b](scenarios.md#sc-04b--repeated-wake-pulses) | Repeated HP wake pulses do not refresh wake hold |
| [SC-05](scenarios.md#sc-05--back-to-back-sequence) | New sequence same activity; AF + timing gates |
| [SC-05b](scenarios.md#sc-05b--fp-during-post-shutter-hp-hold-extension) | FP accepted during post-burst hold starts next sequence |
| [SC-06](scenarios.md#sc-06--cold-fp-no-prior-wake) | FP without prior HP |
| [SC-07](scenarios.md#sc-07--hp-during-active-burst) | HP ignored during burst |
| [SC-07b](scenarios.md#sc-07b--hp-during-post-burst-hold) | HP input during post-burst hold does not refresh wake hold |
| [SC-08](scenarios.md#sc-08--fp-before-hp) | FP before HP |
| [SC-09](scenarios.md#sc-09--fp-when-maxsequencecount-reached) | FP at sequence cap |
| [SC-10](scenarios.md#sc-10--recovery-after-maxsequencecount-cap) | Recovery after cap |
| [SC-11](scenarios.md#sc-11--startframespacingmin-vs-minhalfpressbeforeshutter) | StartFrameSpacingMin vs T |
| [SC-12](scenarios.md#sc-12--hp-only-pir-gap-minimum) | HP only, PIR Gap minimum |
| [SC-13](scenarios.md#sc-13--input-line-bounce-debounce) | Input line bounce (debounce) |
| [SC-14](scenarios.md#sc-14--held-vs-pulsed-fp-input) | Held vs pulsed FP |
| [SC-15](scenarios.md#sc-15--power-save-performance-budget) | Power-save performance (&lt; 1 ms) |
| [SC-16](scenarios.md#sc-16--hp-input-released-immediately-after-fp) | HP input release after FP does not drop HP OUT |
| [SC-17](scenarios.md#sc-17--first-frame-gated-by-short-hp-lead) | First frame gated by short HP lead |
| [SC-18](scenarios.md#sc-18--hp-chatterrelease-during-burst-does-not-affect-fps) | HP chatter/release during burst does not affect FPS |
| [SC-19](scenarios.md#sc-19--new-event-after-hp-release) | New event after HP OUT release |
| [SC-20](scenarios.md#sc-20--t-greater-than-y-interaction) | T greater than Y interaction |

## Terminology

| Term | Meaning |
|------|---------|
| **Activity** | One continuous “session” from first camera HP assert (wake or cold FP) until HP is released and the trap returns to idle. May include **multiple sequences**. |
| **Sequence** | One **full-press activation cycle**: a debounced FP **input** is accepted → MCU runs `FrameCount` shutter pulses → sequence completes with `PostShutterHalfPressHoldTimeExtension`. Retriggers during a shot are **not** a new sequence (R10). |
| **Frame** | A single FP **output** pulse to the camera within a sequence. |

`MaxSequenceCount` limits how many sequences may run **per activity**, not how many frames per sequence (`FrameCount` does that).

Accepted sequences do not refresh `wakeHalfPressHoldTime`. Final HP release follows: `max(initial HP assert + wakeHalfPressHoldTime, final frame release + PostShutterHalfPressHoldTimeExtension)`.

## Activity (wake-to-sleep)

An **activity** begins when the MCU first asserts camera HP for a wake or cold FP path, and ends when:

- HP is released after the last sequence’s post-burst hold (and no further sequence is started), **and**
- `wakeHalfPressHoldTime` expires from the initial HP assert point (not refreshed by accepted FP), combined with post-frame hold rule.

`MaxSequenceCount` applies **per activity**. Extra FP inputs within `fullPressIgnoreGap` after each sequence start are ignored (R10). PIR **Gap** is set to **minimum** ([pir-sensor-settings.md](pir-sensor-settings.md)); MCU `fullPressIgnoreGap` handles burst retrigger.

HP **input release** is not mirrored to camera HP OUT in state-machine mode. HP input is an edge-triggered wake/refresh signal; once camera HP OUT is latched, only the MCU activity rules release it. A trigger may release HP input immediately after FP without dropping camera HP OUT or changing burst cadence (SC-16, SC-18).

## Core rules

| # | Rule |
|----|--------------------------------------------------------|
| R1 | HP **input** is wake/prepare only; MCU **latches camera HP ON** on wake. Repeated HP pulses while already latched do not refresh wake deadline. |
| R2 | If no FP arrives before `wakeHalfPressHoldTime` expires, MCU **releases camera HP**. |
| R3 | FP may arrive **without** prior HP input. |
| R4 | Before any FP **output** pulse, camera HP must have been active for at least `minHalfPressBeforeShutter`. In the common latched-HP path, this typically gates frame 1 only; later frames follow `StartFrameSpacingMin` unless HP had been dropped. |
| R5 | Each FP pulse to the camera lasts `shutterPulseDuration` (no overlapping FP outputs). |
| R6 | In a sequence, schedule successive FP **output** pulse starts so each next start occurs at least `StartFrameSpacingMin` after the prior pulse end (falling edge to next rising edge). Spacing is **pulse-end-to-next-start**; start-to-start interval therefore includes `shutterPulseDuration` and may be longer if R4 or HP release delays the next start. |
| R7 | During a sequence burst, **keep camera HP OUT asserted** between frames; do not release HP because of trigger activations (original §4b). |
| R9 | During burst, **do not drop HP** merely because another FP input arrives (HP latch is independent of ignored FP). |
| R10 | From **sequence start** (accepted FP that schedules the burst), ignore all FP **inputs** for `fullPressIgnoreGap` — no second sequence and no extra frames in that window. Telemetry reject counters may still increment for those ignored inputs. Covers PIR retrigger flood when PIR **Gap = minimum**. Set **≥** typical burst length; default estimate `(FrameCount - 1) × (StartFrameSpacingMin + shutterPulseDuration) + shutterPulseDuration`. |
| R10b | An FP **input** is accepted and **starts a new sequence** only when the burst schedule is not in progress, `fullPressIgnoreGap` has elapsed since that sequence’s start, `sequencesStartedThisActivity < MaxSequenceCount`, and R4/R12 gates pass. A retrigger while HP is still latched is taken as soon as these gates clear. |
| R11 | After the last frame of a sequence’s burst schedule, hold HP for `PostShutterHalfPressHoldTimeExtension`. If another sequence may still start (under `MaxSequenceCount`), HP may remain latched; release HP only when activity ends. |
| R12 | A new **sequence** begins when an FP is accepted after the prior sequence’s burst **schedule** is complete and normal gates pass. R4 applies before the first FP output of that sequence. |
| R13 | While activity is active (any sequence in progress, between sequences, or post-burst with sequences remaining), camera HP must not be released solely because `wakeHalfPressHoldTime` expired (`activityHalfPressHoldPolicy = holdUntilActivityEnd`). |
| R14 | HP **input** during an active burst does not change `remainingFrames`, does not emit FP, and does not release camera HP (`halfPressDuringBurstPolicy = independent`). |
| R15 | Final HP release time is `max(initial HP assert + wakeHalfPressHoldTime, final frame release + PostShutterHalfPressHoldTimeExtension)`. Accepted FP/sequence starts do not refresh wake deadline. |
| TimeOut | Optional extension only (not part of customer baseline unless explicitly requested): when `sequencesStartedThisActivity >= MaxSequenceCount` and another FP arrives, enter timeout for one full burst budget `((FrameCount - 1) * (StartFrameSpacingMin + shutterPulseDuration)) + shutterPulseDuration`. During timeout, ignore FP and HP inputs for sequence triggering. After timeout expires, normal acceptance resumes. |

## Half-press without full-press (timeout path)

```mermaid
sequenceDiagram
    participant PIR as Wide PIR / HP in
    participant MCU as Timing MCU
    participant CAM as Camera

    PIR->>MCU: HP wake pulse(s)
    MCU->>CAM: Assert HP (latched)
    Note over MCU: Start wakeHalfPressHoldTime
    alt FP within hold window
        PIR->>MCU: FP (narrow PIR)
        Note over MCU,CAM: Nominal: HP OUT stays latched through burst R7
        MCU->>CAM: FP pulse(s) per burst rules
    else Timeout
        MCU->>CAM: Release HP
    end
```

## Full-press without adequate half-press lead

| Condition | Behavior |
|-----------|----------|
| Before each shutter, HP active ≥ `minHalfPressBeforeShutter` | Fire that pulse after debounce |
| Before a shutter, HP inactive or lead too short | Assert HP, wait remainder of `minHalfPressBeforeShutter`, then fire that pulse |
| `fullPressWithoutPriorHpPolicy` | `assertHpThenWait` (see parameters) |

## Burst scheduling and FP ignore gap

**Start sequence:** A debounced FP **input** accepted while idle (first sequence) or after the prior sequence’s burst is complete sets `remainingFrames = FrameCount` for **that sequence only**, if `sequencesStartedThisActivity < MaxSequenceCount`. That schedule runs to completion; frame count is not capped by `MaxSequenceCount` (only sequence **count** is).

**FP ignore during sequence (R10):** Set PIR **Gap** to **minimum** (0.5 s). The MCU ignores FP **inputs** for `fullPressIgnoreGap` after each sequence start — not a per-pulse gap after each shutter.

**Default estimate** for `fullPressIgnoreGap` (tunable — set longer in the field if needed):

```text
(FrameCount - 1) × (StartFrameSpacingMin + shutterPulseDuration) + shutterPulseDuration
```

Example: `FrameCount = 4`, `StartFrameSpacingMin = 1.0 s`, `shutterPulseDuration = 0.1 s` → default **3.4 s**. With `shutterPulseDuration = 0.2 s` → **3.8 s**. Increase if `minHalfPressBeforeShutter` or other gates stretch real burst spacing beyond this estimate.

**Example** (`FrameCount = 4`):

| Step | Event | Result |
|--|----------|----------|
| 1 | First FP accepted | Sequence starts; ignore all further FP (R10) |
| 2 | More FP during frames 1–4 | **Ignored** (`fullPressIgnoreGap`) |
| 3 | Frames 1–4 | Fire on `StartFrameSpacingMin` schedule |
| 4 | Burst schedule complete | Post-burst hold (R11); R10 ignore ends when `fullPressIgnoreGap` elapses |
| 5 | New FP after sequence done | May start next sequence (R12) if `< MaxSequenceCount` |

**Between sequences:** Only after burst schedule completes may an FP start the next sequence (R12).

## Burst frame scheduling (within one sequence)

Normative timing for firmware — avoids reading `minHalfPressBeforeShutter` as an extra delay on top of `StartFrameSpacingMin`.

### Normal case (HP latched for whole sequence)

Half press stays up; frames fire with a minimum idle gap **Y** after each pulse release.

1. Sequence starts → camera HP OUT **on** (wake path or cold-FP path).
2. **Sequence start:** Begin R10 FP ignore for `fullPressIgnoreGap` (default ≈ `(FrameCount - 1) × (StartFrameSpacingMin + shutterPulseDuration) + shutterPulseDuration`).
3. **Frame 1:** Wait until `minHalfPressBeforeShutter` satisfied → FP OUT pulse (`shutterPulseDuration`).
4. **Frames 2…N:** At each scheduled time `t_prev_release + StartFrameSpacingMin`, fire the next FP OUT pulse (R6). **Do not** wait another full `minHalfPressBeforeShutter` if HP never dropped — R4 is already satisfied.
5. **Sequence end:** After frame **N** → `PostShutterHalfPressHoldTimeExtension` (R11). R10 ignore ends when `fullPressIgnoreGap` elapses (burst may finish earlier).

Example (`minHalfPressBeforeShutter` = 0.5 s, `StartFrameSpacingMin` = 1.0 s, `shutterPulseDuration` = 0.1 s, `FrameCount` = 4, HP held throughout):

| Event | Time (illustrative) |
|-------|---------------------|
| HP asserted | 0.0 s |
| Frame 1 OUT | 0.5 s (after T) |
| Frame 2 OUT | 1.6 s |
| Frame 3 OUT | 2.7 s |
| Frame 4 OUT | 3.8 s |

`StartFrameSpacingMin` sets the schedule. **`minHalfPressBeforeShutter` does not override or replace `StartFrameSpacingMin`** in this case.

### Exception path (HP OUT released mid-burst)

Not normal operation. This means camera HP **output** was released by the MCU, not that HP **input** from the trigger released. If camera HP OUT was released between scheduled frames, **before** the next FP OUT pulse:

1. Assert HP OUT.
2. Wait until `minHalfPressBeforeShutter` has elapsed since that assert.
3. Then fire the pulse.

The **actual** gap since the previous frame may be **longer than** `StartFrameSpacingMin`; that is a late gate, not a change to **Y**.

### What does *not* happen

- No separate focus-acquisition interval between frames.
- `minHalfPressBeforeShutter` is **not** added to every inter-frame interval when HP stays latched.
- `StartFrameSpacingMin` is **not** shortened or replaced by **T** when HP lead is already satisfied.
- HP **input release** is not a command to release camera HP OUT in state-machine mode.

## Policies (enums)

| Parameter | Intended value | Meaning |
|-----------|----------------|---------|
| `halfPressDuringBurstPolicy` | `independent` | HP input during burst does not cancel latched HP |
| `fullPressWithoutPriorHpPolicy` | `assertHpThenWait` | Cold FP path waits for `minHalfPressBeforeShutter` before each pulse |

## Customer-facing summary

1. **Motion detected (wide area):** camera wakes and autofocuses; if nothing worth a photo appears within the configured hold time, the camera goes back to sleep.
2. **Subject in frame (narrow area):** camera takes a configured number of photos per trigger event, spaced by `StartFrameSpacingMin`.
3. **Retriggers during a sequence:** extra full-press signals for `fullPressIgnoreGap` after sequence start are **ignored** (PIR Gap minimum; MCU R10) so the burst is not reset or extended.
4. **Another pass after a sequence:** a new full-press after the prior sequence’s burst completes can start another sequence in the same activity (subject to `fullPressIgnoreGap`, AF timing, and `MaxSequenceCount`).
5. **Short HP trigger pulses:** once the MCU latches camera HP, releasing the trigger's HP input does not release camera HP during a shoot.

## Sequence boundaries (SC-05, SC-05b)

**Sequence ends when:** that sequence’s burst schedule is complete and its `PostShutterHalfPressHoldTimeExtension` has run (unless cut short by accepting the next sequence).

**Activity ends when:** no further sequence will start, wake hold expired, and HP released.

**New sequence on FP:** After the prior sequence’s burst schedule is complete, a debounced FP may start the next sequence (R12), incrementing `sequencesStartedThisActivity` while under cap.

**Gates before first FP output of a new sequence:**

1. **Prior sequence complete (R12):** Burst schedule finished and `fullPressIgnoreGap` elapsed since that sequence’s start.
2. **AF lead (R4):** `minHalfPressBeforeShutter` (often already satisfied if HP stayed latched).
3. **Spacing:** `PostShutterHalfPressHoldTimeExtension` from prior sequence may still run before next sequence’s first shot.

Flash/strobe readiness is **not** an MCU parameter — spacing is enforced via `StartFrameSpacingMin` and operator flash setup.

**FP during post-shutter HP hold extension:** See [SC-05b](scenarios.md#sc-05b--fp-during-post-shutter-hp-hold-extension).

## Developer notes

- Treat the MCU lane as a **state machine**, not a linear script — HP latch, per-sequence burst scheduler, and `sequencesStartedThisActivity` interact.
- Debounce HP and FP inputs separately (`halfPressInputDebounce`, `fullPressInputDebounce`).
- `triggerDuringBurstPolicy` **removed** — sequence-level FP ignore (R10) when PIR Gap = minimum.
- `minStrobeRecycleTime` **removed** — flash timing is not an MCU parameter; use `StartFrameSpacingMin` and external setup.
- `triggerRetriggerPolicy` removed — covered by SC-01, SC-05, SC-06 and R12.
- Align firmware constant names with [parameters.md](parameters.md); validate against [scenarios.md](scenarios.md).
