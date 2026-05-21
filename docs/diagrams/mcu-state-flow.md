# MCU state flow

State-oriented view. Parameters: [parameters.md](../parameters.md). Rules: [behavior-spec.md](../behavior-spec.md). Scenarios: [scenarios.md](../scenarios.md).

**Nominal path (defaults):** [timing-sequences § Nominal](timing-sequences.md#nominal-path-defaults) — camera **HP OUT stays ON** from activity start through burst(s) and `PostShutterHalfPressHoldTimeExtension` until activity ends (R7, R13).

## Activity state diagram

```mermaid
stateDiagram-v2
    [*] --> Idle

    Idle --> WakeAfActive: HP wake SC04
    WakeAfActive --> WakeAfActive: HP pulse observed deadline unchanged SC04b
    WakeAfActive --> Idle: wake timeout SC04

    Idle --> ColdFpWait: FP cold SC06
    WakeAfActive --> BurstActive: FP accepted SC01
    ColdFpWait --> BurstActive: minHalfPress elapsed SC06

    state BurstActive {
        [*] --> WaitSchedule
        WaitSchedule --> WaitMinHalfPress: HP lead less than T
        WaitMinHalfPress --> WaitSchedule: T satisfied
        WaitSchedule --> ShutterPulse: frame due
        ShutterPulse --> InterFrameWait: pulse done
        InterFrameWait --> WaitSchedule: more frames Y spacing only
        InterFrameWait --> [*]: burst complete
    }

    BurstActive --> BurstActive: FP during sequence ignored SC02 SC03
    BurstActive --> PostShutterHpExtension: last frame done R11
    PostShutterHpExtension --> BurstActive: FP new sequence SC05 SC05b
    PostShutterHpExtension --> Idle: activity end release HP

    note right of BurstActive
        Nominal: HP OUT ON
        whole activity R7 R13
        MaxSequenceCount SC09
    end note
```

**Burst substates (nominal):** After frame 1, `WaitSchedule` waits **StartFrameSpacingMin** only — not another full `minHalfPressBeforeShutter` unless HP was released (`WaitMinHalfPress`).

## Decision flow (nominal path highlighted)

```mermaid
flowchart TD
    A[HP wake] --> B[Latch HP OUT ON]
    B --> C{FP within wakeHalfPressHoldTime?}
    C -->|No timeout SC04| D[Release HP idle]
    C -->|Yes| E{HP lead >= T?}
    E -->|No cold SC06| F[Wait T then first frame]
    E -->|Yes SC01| G[Start sequence R10b]
    F --> H[Burst loop]
    G --> H
    H --> I{more frames?}
    I -->|Yes| J[wait StartFrameSpacingMin]
    J --> K[FP OUT pulse]
    K --> H
    I -->|No| L[PostShutter extension HP stays ON]
    L --> M{activity end?}
    M -->|Yes| D
    L --> P{sequences less than MaxSequenceCount?}
    P -->|Yes FP SC05| H
    P -->|No cap SC09| Q[ignore FP until activity end]
    N[FP during sequence SC02] --> O[discard R10]
    O --> H
```

## States (customer language)

| State | Scenario | Experience |
|-------|----------|------------|
| **Nominal activity (SC-01)** | SC-01 | HP stays on; burst after FP; extension; sleep |
| Idle | — | Camera ready |
| Wake / AF active | SC-04, SC-01 wake phase | Focusing after distant motion |
| Burst active | SC-01–SC-03 | Exposures firing; extra FP inputs ignored (R10) |
| Post-shutter HP extension | SC-01, SC-05b | HP still on after last frame (`PostShutterHalfPressHoldTimeExtension`) |
| Second sequence same activity | SC-05, SC-05b | HP usually still latched; new burst |
| At max sequences | SC-09 | Further FP cannot start sequences until activity ends |

## Exception entry points

| From | Event | To |
|------|-------|-----|
| Idle / wake | No FP before X expires | Idle (SC-04) |
| Idle | FP without prior HP | ColdFpWait → BurstActive (SC-06) |
| BurstActive | HP OUT released mid-burst | WaitMinHalfPress before next frame (behavior-spec exception) |
