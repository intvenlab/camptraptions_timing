# Test Report

## Executive Summary

This report summarizes validation execution of the timing controller using the TickleBoard bench harness and vector-driven scenario suite. The test method replays scripted HP/FP stimuli, captures HP/FP output edges, evaluates telemetry deltas, and compares observed behavior against scenario acceptance checks.

Scope includes all core scenarios (`SC-01` through `SC-20`) and mandatory add-on scenarios for BLE-connected behavior, gap-boundary handling, deferred config writes, and factory reset/coercion behavior.

### Outcome Summary

- Tests executed: 28
- Passed: 10
- Failed: 18
- Timing assertion compliance: 36/59 (61.0%)
- Functional assertion compliance: 88/97 (90.7%)
- Timing coverage KPI target for future runs: >=80% of SC/AO cases with at least one explicit timing assertion

### Status Update

- Current full-suite run is regressed versus the prior baseline: 18 failing cases are present and require triage.

### Metric Interpretation Update (HP Timing)

- `hpInToHpOutLatencyMs` is treated as a causal latency metric and is never reported as negative.
- If `HP_OUT` is already asserted before first `HP_IN`, `hpInToHpOutLatencyMs` is classified as **unmeasurable** (null + reason), not a signed latency.
- `hpHoldAfterLastFrameMs` is interpreted as the delay from final `FP_OUT` release to the first `HP_OUT` release at/after that point.
- Any `HP_OUT` release before final `FP_OUT` release is an anomaly for scenarios requiring latched HP and must be called out as validation failure evidence, not accepted as normal arithmetic.

## Master Results Table

| Test Description | Reference | Outcome | Failure Class | Timing Assertions |
|---|---|---|---|---:|
| Normal wake then shoot | `SC-01` (`SC-01-NOMINAL`) | Fail | timing_tolerance | 4/5 |
| Extra FP during sequence should be ignored | `SC-02` (`SC-02-FP-DURING-SEQUENCE`) | Pass | pass | 1/1 |
| FP flood during burst still yields one sequence | `SC-03` (`SC-03-FP-FLOOD`) | Pass | pass | 1/1 |
| HP only path should timeout with no FP | `SC-04` (`SC-04-WAKE-TIMEOUT`) | Fail | timing_tolerance | 0/1 |
| Repeated HP pulses do not extend wake hold | `SC-04b` (`SC-04B-REPEATED-HP`) | Pass | pass | 2/2 |
| FP after burst starts second sequence under cap | `SC-05` (`SC-05-BACK-TO-BACK`) | Fail | timing_tolerance | 1/5 |
| FP during post-shutter hold can start next sequence | `SC-05b` (`SC-05B-FP-DURING-POST-HOLD`) | Fail | timing_tolerance | 1/5 |
| FP before HP triggers cold path with AF lead | `SC-06` (`SC-06-COLD-FP`) | Fail | timing_tolerance | 1/2 |
| HP activity during burst does not alter frame schedule | `SC-07` (`SC-07-HP-DURING-BURST`) | Pass | pass | 1/1 |
| HP during post-hold does not move wake hold and does not fire FP | `SC-07b` (`SC-07B-HP-DURING-POST-HOLD`) | Pass | pass | 2/2 |
| FP before HP then later HP should keep cold path stable | `SC-08` (`SC-08-FP-BEFORE-HP`) | Fail | timing_tolerance | 1/2 |
| Additional FP rejected at MaxSequenceCount cap | `SC-09` (`SC-09-SEQUENCE-CAP`) | Fail | logic_mismatch | 1/1 |
| New event after cap end starts fresh activity | `SC-10` (`SC-10-RECOVERY-AFTER-CAP`) | Fail | timing_tolerance | 2/3 |
| Validate T gating frame 1 and Y spacing for later frames | `SC-11` (`SC-11-SPACING-VS-T`) | Fail | timing_tolerance | 1/2 |
| HP-only no-sequence behavior at field-like stimulus | `SC-12` (`SC-12-HP-ONLY-MIN-GAP`) | Fail | timing_tolerance | 0/1 |
| Synthetic bounce should be rejected by debounce windows | `SC-13` (`SC-13-BOUNCE-DEBOUNCE`) | Fail | logic_mismatch | 1/1 |
| Held FP should count once, not continuous output | `SC-14` (`SC-14-HELD-VS-PULSED-FP`) | Pass | pass | 2/2 |
| Compare latency paths with powerSaveIdleMode enabled vs disabled | `SC-15` (`SC-15-POWER-SAVE-BUDGET`) | Pass | pass | 1/1 |
| HP release after FP should not drop HP_OUT continuity | `SC-16` (`SC-16-HP-RELEASE-AFTER-FP`) | Fail | timing_tolerance | 3/5 |
| First frame follows max(FP accept, HP_OUT assert + T) | `SC-17` (`SC-17-SHORT-HP-LEAD`) | Fail | timing_tolerance | 1/2 |
| HP chatter/release during burst does not alter scheduling | `SC-18` (`SC-18-HP-CHATTER-BURST`) | Pass | pass | 1/1 |
| New FP after HP release should be cold/short-lead behavior | `SC-19` (`SC-19-NEW-EVENT-AFTER-RELEASE`) | Fail | timing_tolerance | 3/4 |
| T may delay frame 1 but not add delay to frames 2..N | `SC-20` (`SC-20-T-GREATER-THAN-Y`) | Pass | pass | 3/3 |
| Re-run SC-04 while BLE remains connected | `ADDON-BLE-CONNECTED` (`AO-BLE-CONNECTED-SC04`) | Fail | timing_tolerance | 0/1 |
| Re-run SC-01 while BLE remains connected | `ADDON-BLE-CONNECTED` (`AO-BLE-CONNECTED-SC01`) | Fail | timing_tolerance | 0/1 |
| fullPressIgnoreGap just-before/exact/just-after checks | `ADDON-GAP-BOUNDARY` (`AO-GAP-BOUNDARY-TRIAD`) | Fail | timing_tolerance | 1/2 |
| Write config during active flow and verify defer/reject behavior | `ADDON-DEFERRED-CONFIG` (`AO-DEFERRED-CONFIG-WRITES`) | Fail | timing_tolerance | 0/1 |
| Verify factory reset behavior and reserved field coercion | `ADDON-FACTORY-RESET` (`AO-FACTORY-RESET-AND-COERCION`) | Pass | pass | 1/1 |

## Results Statistics

### Assertion Coverage And Compliance

| Assertion Class | Passed | Total | Compliance |
|---|---:|---:|---:|
| Timing assertions | 36 | 59 | 61.0% |
| Functional assertions | 88 | 97 | 90.7% |

### Failure Class Definitions

| Failure class | Meaning |
|---|---|
| `pass` | No failed assertions |
| `timing_tolerance` | One or more timing assertions failed tolerance/range checks |
| `logic_mismatch` | Functional assertion mismatch with no timing assertion failures |

### Timing By Commanded Condition

Mixed-condition global means are intentionally removed here. Timing values are broken out by commanded scenario regime so each line compares like-with-like.

| Timing regime | Sample count | Min | Mean | Max | Cases |
|---|---:|---:|---:|---:|---|
| `frameStartSpacingMs` around 0.2 s (rapid cadence) | 4 | 200 ms | 200 ms | 200 ms | `SC-20-T-GREATER-THAN-Y`, `AO-BLE-CONNECTED-SC01`, `AO-GAP-BOUNDARY-TRIAD`, `AO-DEFERRED-CONFIG-WRITES` |
| `frameStartSpacingMs` around 1.0 s (nominal cadence) | 16 | 998.67 ms | 998.93 ms | 999.57 ms | `SC-01-NOMINAL`, `SC-02-FP-DURING-SEQUENCE`, `SC-03-FP-FLOOD`, `SC-06-COLD-FP`, `SC-07-HP-DURING-BURST`, `SC-07B-HP-DURING-POST-HOLD`, `SC-08-FP-BEFORE-HP`, `SC-09-SEQUENCE-CAP`, `SC-11-SPACING-VS-T`, `SC-13-BOUNCE-DEBOUNCE`, `SC-14-HELD-VS-PULSED-FP`, `SC-15-POWER-SAVE-BUDGET`, `SC-16-HP-RELEASE-AFTER-FP`, `SC-17-SHORT-HP-LEAD`, `SC-18-HP-CHATTER-BURST`, `AO-FACTORY-RESET-AND-COERCION` |
| `frameStartSpacingMs` around 1.2-2.0 s (extended/cap windows) | 3 | 1.412 s | 1.604 s | 1.929 s | `SC-05-BACK-TO-BACK`, `SC-05B-FP-DURING-POST-HOLD`, `SC-19-NEW-EVENT-AFTER-RELEASE` |
| `frameStartSpacingMs` around 4.2 s (recovery cadence) | 1 | 4.171 s | 4.171 s | 4.171 s | `SC-10-RECOVERY-AFTER-CAP` |

All other timing metrics are reported per-test in the detailed sections to preserve command/context-specific meaning.

## Detailed Test Results

### Normal wake then shoot (`SC-01-NOMINAL`)

| Field | Value |
|---|---|
| Requirement descriptor | Normal wake then shoot |
| Reference | `SC-01` |
| Coverage tags | core, nominal |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `fpPulseWidthMs` | Pass | expected 100.0 +/- 5.0, got 100.0 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `sequenceCount` | Pass | expected 1, got 1 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetryDelta.sequenceCompletedCount` | Pass | expected 1, got 1 |
| `hold.noHpReleaseBeforeFinalFrame` | Pass | expected False, got False |
| `hold.requirePostFinalFrameHpRelease` | Pass | hpHoldAfterLastFrameMs=1999.0 reason=None |
| `hold.dropTimeRule` | Fail | missing input for drop-time rule (wakeHoldMs=10000, postFinalFrameHoldMs=2000, hpOutContinuityMs=None, hpAssertToFinalFrameReleaseMs=None) |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Extra FP during sequence should be ignored (`SC-02-FP-DURING-SEQUENCE`)

| Field | Value |
|---|---|
| Requirement descriptor | Extra FP during sequence should be ignored |
| Reference | `SC-02` |
| Coverage tags | core, ignore-gap |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.25 |
| `frameStartSpacingMs` | 998.67 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 1 |
| `ignoredFpDuringBurstCount` | 1 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 998.7 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetryDelta.ignoredFpDuringGapCount` | Pass | expected 1, got 1 |
| `telemetryDelta.ignoredFpDuringBurstCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### FP flood during burst still yields one sequence (`SC-03-FP-FLOOD`)

| Field | Value |
|---|---|
| Requirement descriptor | FP flood during burst still yields one sequence |
| Reference | `SC-03` |
| Coverage tags | core, ignore-gap |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.25 |
| `frameStartSpacingMs` | 998.67 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 3 |
| `ignoredFpDuringBurstCount` | 3 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 998.7 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetryDelta.ignoredFpDuringGapCount` | Pass | expected 3, got 3 |
| `telemetryDelta.ignoredFpDuringBurstCount` | Pass | expected 3, got 3 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 3, 'ignoredFpDuringBurstCount': 3, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP only path should timeout with no FP (`SC-04-WAKE-TIMEOUT`)

| Field | Value |
|---|---|
| Requirement descriptor | HP only path should timeout with no FP |
| Reference | `SC-04` |
| Coverage tags | core, wake-only |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 0 |
| `sequenceCount` | 0 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 0 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 0, got 0 |
| `telemetryDelta.wakeTimeoutCount` | Pass | expected 1, got 1 |
| `wakeOnlyHoldMs` | Fail | missing metric wakeOnlyHoldMs |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Repeated HP pulses do not extend wake hold (`SC-04B-REPEATED-HP`)

| Field | Value |
|---|---|
| Requirement descriptor | Repeated HP pulses do not extend wake hold |
| Reference | `SC-04b` |
| Coverage tags | variant, wake-only |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | 6.7e+03 |
| `frameCount` | 0 |
| `sequenceCount` | 0 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 0 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 2 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 0, got 0 |
| `telemetryDelta.hpRefreshCount` | Pass | expected 2, got 2 |
| `wakeOnlyHoldMs` | Pass | expected 10000.0 +/- 100.0, got 9989.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 6698.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 2, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 2} |

### FP after burst starts second sequence under cap (`SC-05-BACK-TO-BACK`)

| Field | Value |
|---|---|
| Requirement descriptor | FP after burst starts second sequence under cap |
| Reference | `SC-05` |
| Coverage tags | core, multi-sequence |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.88 |
| `frameStartSpacingMs` | 1.41e+03 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | 7.39e+03 |
| `frameCount` | 8 |
| `sequenceCount` | 2 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 8, got 8 |
| `sequenceCount` | Pass | expected 2, got 2 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 2, got 2 |
| `telemetryDelta.sequenceCompletedCount` | Pass | expected 2, got 2 |
| `interSequenceGapMs` | Fail | expected range [2500.0, 3500.0], got 3790.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 7387.0 |
| `hold.noHpReleaseBeforeFinalFrame` | Fail | expected False, got True |
| `hold.requirePostFinalFrameHpRelease` | Fail | hpHoldAfterLastFrameMs=None reason=no_hp_release_after_final_fp_release |
| `hold.dropTimeRule` | Fail | expected HP_OUT continuity by rule max(10000.0, 3596.0+2000.0) = 10000.0 +/- 100.0, got -1292.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

### FP during post-shutter hold can start next sequence (`SC-05B-FP-DURING-POST-HOLD`)

| Field | Value |
|---|---|
| Requirement descriptor | FP during post-shutter hold can start next sequence |
| Reference | `SC-05b` |
| Coverage tags | variant, multi-sequence |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 1.47e+03 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | 7.81e+03 |
| `frameCount` | 8 |
| `sequenceCount` | 2 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 8, got 8 |
| `sequenceCount` | Pass | expected 2, got 2 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 2, got 2 |
| `telemetryDelta.sequenceCompletedCount` | Pass | expected 2, got 2 |
| `interSequenceGapMs` | Fail | expected range [1800.0, 2600.0], got 4209.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 7806.0 |
| `hold.noHpReleaseBeforeFinalFrame` | Fail | expected False, got True |
| `hold.requirePostFinalFrameHpRelease` | Fail | hpHoldAfterLastFrameMs=None reason=no_hp_release_after_final_fp_release |
| `hold.dropTimeRule` | Fail | expected HP_OUT continuity by rule max(10000.0, 3597.0+2000.0) = 10000.0 +/- 100.0, got -1711.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

### FP before HP triggers cold path with AF lead (`SC-06-COLD-FP`)

| Field | Value |
|---|---|
| Requirement descriptor | FP before HP triggers cold path with AF lead |
| Reference | `SC-06` |
| Coverage tags | core, cold-fp |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.coldFpSequenceCount` | Fail | expected 1, got 0 |
| `firstFrameGateDelayMs` | Fail | expected range [450.0, 650.0], got 0.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP activity during burst does not alter frame schedule (`SC-07-HP-DURING-BURST`)

| Field | Value |
|---|---|
| Requirement descriptor | HP activity during burst does not alter frame schedule |
| Reference | `SC-07` |
| Coverage tags | core, hp-during-burst |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.hpIgnoredDuringBurstCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 1, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP during post-hold does not move wake hold and does not fire FP (`SC-07B-HP-DURING-POST-HOLD`)

| Field | Value |
|---|---|
| Requirement descriptor | HP during post-hold does not move wake hold and does not fire FP |
| Reference | `SC-07b` |
| Coverage tags | variant, hp-during-burst |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.75 |
| `frameStartSpacingMs` | 998.67 |
| `fpInToFpOutLatencyMs` | 1 |
| `hpInToHpOutLatencyMs` | 7.81e+03 |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 998.7 |
| `telemetryDelta.hpRefreshCount` | Pass | expected 1, got 1 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 7807.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### FP before HP then later HP should keep cold path stable (`SC-08-FP-BEFORE-HP`)

| Field | Value |
|---|---|
| Requirement descriptor | FP before HP then later HP should keep cold path stable |
| Reference | `SC-08` |
| Coverage tags | core, cold-fp |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.5 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.coldFpSequenceCount` | Fail | expected 1, got 0 |
| `firstFrameGateDelayMs` | Fail | expected range [450.0, 650.0], got 0.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 1, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Additional FP rejected at MaxSequenceCount cap (`SC-09-SEQUENCE-CAP`)

| Field | Value |
|---|---|
| Requirement descriptor | Additional FP rejected at MaxSequenceCount cap |
| Reference | `SC-09` |
| Coverage tags | core, cap |
| Outcome | Fail |
| Failure class | logic_mismatch |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.5 |
| `frameStartSpacingMs` | 999.57 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 8 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Fail | expected 2, got 8 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.6 |
| `telemetryDelta.acceptedFpCount` | Fail | expected 1, got 2 |
| `telemetryDelta.rejectedFpAtSequenceCapCount` | Fail | expected 1, got 0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

### New event after cap end starts fresh activity (`SC-10-RECOVERY-AFTER-CAP`)

| Field | Value |
|---|---|
| Requirement descriptor | New event after cap end starts fresh activity |
| Reference | `SC-10` |
| Coverage tags | core, cap |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.5 |
| `frameStartSpacingMs` | 4.17e+03 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | 1.2e+04 |
| `frameCount` | 4 |
| `sequenceCount` | 2 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Fail | expected 1000.0 +/- 10.0, got 4171.3 |
| `sequenceCount` | Pass | expected 2, got 2 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 2, got 2 |
| `telemetryDelta.rejectedFpAtSequenceCapCount` | Pass | expected 0, got 0 |
| `interSequenceGapMs` | Pass | expected range [5000.0, inf], got 10416.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 12015.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

### Validate T gating frame 1 and Y spacing for later frames (`SC-11-SPACING-VS-T`)

| Field | Value |
|---|---|
| Requirement descriptor | Validate T gating frame 1 and Y spacing for later frames |
| Reference | `SC-11` |
| Coverage tags | core, timing |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 2 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Fail | expected 4, got 2 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `firstFrameGateDelayMs` | Fail | expected range [350.0, 650.0], got 0.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP-only no-sequence behavior at field-like stimulus (`SC-12-HP-ONLY-MIN-GAP`)

| Field | Value |
|---|---|
| Requirement descriptor | HP-only no-sequence behavior at field-like stimulus |
| Reference | `SC-12` |
| Coverage tags | core, wake-only |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 0 |
| `sequenceCount` | 0 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 0 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 0, got 0 |
| `telemetryDelta.wakeTimeoutCount` | Pass | expected 1, got 1 |
| `wakeOnlyHoldMs` | Fail | missing metric wakeOnlyHoldMs |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Synthetic bounce should be rejected by debounce windows (`SC-13-BOUNCE-DEBOUNCE`)

| Field | Value |
|---|---|
| Requirement descriptor | Synthetic bounce should be rejected by debounce windows |
| Reference | `SC-13` |
| Coverage tags | core, debounce |
| Outcome | Fail |
| Failure class | logic_mismatch |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 2 |
| `ignoredFpDuringBurstCount` | 2 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.fpDebounceRejectCount` | Fail | expected 1, got 0 |
| `telemetryDelta.hpDebounceRejectCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 2, 'ignoredFpDuringBurstCount': 2, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 1, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Held FP should count once, not continuous output (`SC-14-HELD-VS-PULSED-FP`)

| Field | Value |
|---|---|
| Requirement descriptor | Held FP should count once, not continuous output |
| Reference | `SC-14` |
| Coverage tags | core, fp-shape |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.25 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `fpPulseWidthMs` | Pass | expected 100.0 +/- 5.0, got 99.2 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Compare latency paths with powerSaveIdleMode enabled vs disabled (`SC-15-POWER-SAVE-BUDGET`)

| Field | Value |
|---|---|
| Requirement descriptor | Compare latency paths with powerSaveIdleMode enabled vs disabled |
| Reference | `SC-15` |
| Coverage tags | core, performance |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.5 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP release after FP should not drop HP_OUT continuity (`SC-16-HP-RELEASE-AFTER-FP`)

| Field | Value |
|---|---|
| Requirement descriptor | HP release after FP should not drop HP_OUT continuity |
| Reference | `SC-16` |
| Coverage tags | core, hp-continuity |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.75 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `firstFrameGateDelayMs` | Fail | expected range [450.0, 650.0], got 0.0 |
| `hold.noHpReleaseBeforeFinalFrame` | Pass | expected False, got False |
| `hold.requirePostFinalFrameHpRelease` | Pass | hpHoldAfterLastFrameMs=2193.0 reason=None |
| `hold.dropTimeRule` | Fail | missing input for drop-time rule (wakeHoldMs=10000, postFinalFrameHoldMs=2000, hpOutContinuityMs=None, hpAssertToFinalFrameReleaseMs=None) |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### First frame follows max(FP accept, HP_OUT assert + T) (`SC-17-SHORT-HP-LEAD`)

| Field | Value |
|---|---|
| Requirement descriptor | First frame follows max(FP accept, HP_OUT assert + T) |
| Reference | `SC-17` |
| Coverage tags | core, af-gate |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.75 |
| `frameStartSpacingMs` | 998.67 |
| `fpInToFpOutLatencyMs` | 1 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 998.7 |
| `firstFrameGateDelayMs` | Fail | expected range [1750.0, 2150.0], got 1.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP chatter/release during burst does not alter scheduling (`SC-18-HP-CHATTER-BURST`)

| Field | Value |
|---|---|
| Requirement descriptor | HP chatter/release during burst does not alter scheduling |
| Reference | `SC-18` |
| Coverage tags | core, hp-during-burst |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.25 |
| `frameStartSpacingMs` | 998.67 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 998.7 |
| `telemetryDelta.hpIgnoredDuringBurstCount` | Pass | expected 2, got 2 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 2, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### New FP after HP release should be cold/short-lead behavior (`SC-19-NEW-EVENT-AFTER-RELEASE`)

| Field | Value |
|---|---|
| Requirement descriptor | New FP after HP release should be cold/short-lead behavior |
| Reference | `SC-19` |
| Coverage tags | core, cold-fp |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.62 |
| `frameStartSpacingMs` | 1.93e+03 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | 1.1e+04 |
| `frameCount` | 8 |
| `sequenceCount` | 2 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 8, got 8 |
| `frameStartSpacingMs` | Fail | expected 1000.0 +/- 10.0, got 1929.0 |
| `sequenceCount` | Pass | expected 2, got 2 |
| `telemetryDelta.coldFpSequenceCount` | Pass | expected 1, got 1 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 2, got 2 |
| `interSequenceGapMs` | Pass | expected range [6500.0, 7500.0], got 7410.0 |
| `secondSequenceStartDelayMs` | Pass | expected range [450.0, 700.0], got 506.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 11006.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

### T may delay frame 1 but not add delay to frames 2..N (`SC-20-T-GREATER-THAN-Y`)

| Field | Value |
|---|---|
| Requirement descriptor | T may delay frame 1 but not add delay to frames 2..N |
| Reference | `SC-20` |
| Coverage tags | core, timing |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.5 |
| `frameStartSpacingMs` | 200 |
| `fpInToFpOutLatencyMs` | 1.91e+03 |
| `hpInToHpOutLatencyMs` | 13 |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 200.0 +/- 5.0, got 200.0 |
| `firstFrameGateDelayMs` | Pass | expected range [1750.0, 2150.0], got 1911.0 |
| `hpInToHpOutLatencyMs_non_negative` | Pass | expected >= 0 when measurable, got 13.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

### Re-run SC-04 while BLE remains connected (`AO-BLE-CONNECTED-SC04`)

| Field | Value |
|---|---|
| Requirement descriptor | Re-run SC-04 while BLE remains connected |
| Reference | `ADDON-BLE-CONNECTED` |
| Coverage tags | addon, ble-connected |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 0 |
| `sequenceCount` | 0 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 0 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 0, got 0 |
| `telemetryDelta.wakeTimeoutCount` | Pass | expected 1, got 1 |
| `wakeOnlyHoldMs` | Fail | missing metric wakeOnlyHoldMs |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Re-run SC-01 while BLE remains connected (`AO-BLE-CONNECTED-SC01`)

| Field | Value |
|---|---|
| Requirement descriptor | Re-run SC-01 while BLE remains connected |
| Reference | `ADDON-BLE-CONNECTED` |
| Coverage tags | addon, ble-connected |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.25 |
| `frameStartSpacingMs` | 200 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Fail | expected 1000.0 +/- 10.0, got 200.0 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### fullPressIgnoreGap just-before/exact/just-after checks (`AO-GAP-BOUNDARY-TRIAD`)

| Field | Value |
|---|---|
| Requirement descriptor | fullPressIgnoreGap just-before/exact/just-after checks |
| Reference | `ADDON-GAP-BOUNDARY` |
| Coverage tags | addon, boundary |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.5 |
| `frameStartSpacingMs` | 200 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 1 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 2 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Fail | expected 2, got 4 |
| `frameStartSpacingMs` | Pass | expected 200.0 +/- 5.0, got 200.0 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetryDelta.ignoredFpDuringGapCount` | Pass | expected 1, got 1 |
| `telemetryDelta.rejectedFpAtSequenceCapCount` | Pass | expected 2, got 2 |
| `firstFrameGateDelayMs` | Fail | expected range [850.0, 1200.0], got 0.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 2, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Write config during active flow and verify defer/reject behavior (`AO-DEFERRED-CONFIG-WRITES`)

| Field | Value |
|---|---|
| Requirement descriptor | Write config during active flow and verify defer/reject behavior |
| Reference | `ADDON-DEFERRED-CONFIG` |
| Coverage tags | addon, config-write |
| Outcome | Fail |
| Failure class | timing_tolerance |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 |
| `frameStartSpacingMs` | 200 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 2 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Fail | expected 4, got 2 |
| `frameStartSpacingMs` | Fail | expected 1000.0 +/- 10.0, got 200.0 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Verify factory reset behavior and reserved field coercion (`AO-FACTORY-RESET-AND-COERCION`)

| Field | Value |
|---|---|
| Requirement descriptor | Verify factory reset behavior and reserved field coercion |
| Reference | `ADDON-FACTORY-RESET` |
| Coverage tags | addon, reset, coercion |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99 |
| `frameStartSpacingMs` | 999 |
| `fpInToFpOutLatencyMs` | 0 |
| `hpInToHpOutLatencyMs` | n/a |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 1 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `frameStartSpacingMs` | Pass | expected 1000.0 +/- 10.0, got 999.0 |
| `telemetryDelta.acceptedFpCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |
