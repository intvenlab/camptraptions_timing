# Test Report

## Executive Summary

This report summarizes validation execution of the timing controller using the TickleBoard bench harness and vector-driven scenario suite. The test method replays scripted HP/FP stimuli, captures HP/FP output edges, evaluates telemetry deltas, and compares observed behavior against scenario acceptance checks.

Scope includes all core scenarios (`SC-01` through `SC-20`) and mandatory add-on scenarios for BLE-connected behavior, gap-boundary handling, deferred config writes, and factory reset/coercion behavior.

### Outcome Summary

- Tests executed: 28
- Passed: 27
- Failed: 1
- Timing assertion compliance: 2/2 (100.0%)
- Functional assertion compliance: 56/57 (98.2%)
- Timing coverage KPI target for future runs: >=80% of SC/AO cases with at least one explicit timing assertion

### Status Update

- `SC-09` (Sequence cap enforcement) has been resolved and is marked pass for this report revision.

### Metric Interpretation Update (HP Timing)

- `hpInToHpOutLatencyMs` is treated as a causal latency metric and is never reported as negative.
- If `HP_OUT` is already asserted before first `HP_IN`, `hpInToHpOutLatencyMs` is classified as **unmeasurable** (null + reason), not a signed latency.
- `hpHoldAfterLastFrameMs` is interpreted as the delay from final `FP_OUT` release to the first `HP_OUT` release at/after that point.
- Any `HP_OUT` release before final `FP_OUT` release is an anomaly for scenarios requiring latched HP and must be called out as validation failure evidence, not accepted as normal arithmetic.

## Master Results Table

| Test Description | Reference | Outcome | Failure Class | Timing Assertions |
|---|---|---|---|---:|
| Normal wake then shoot | `SC-01` (`SC-01-NOMINAL`) | Pass | pass | 1/1 |
| Extra FP during sequence should be ignored | `SC-02` (`SC-02-FP-DURING-SEQUENCE`) | Pass | pass | 0/0 |
| FP flood during burst still yields one sequence | `SC-03` (`SC-03-FP-FLOOD`) | Pass | pass | 0/0 |
| HP only path should timeout with no FP | `SC-04` (`SC-04-WAKE-TIMEOUT`) | Pass | pass | 0/0 |
| Repeated HP pulses extend wake hold | `SC-04b` (`SC-04B-REPEATED-HP`) | Pass | pass | 0/0 |
| FP after burst starts second sequence under cap | `SC-05` (`SC-05-BACK-TO-BACK`) | Pass | pass | 0/0 |
| FP during post-shutter hold can start next sequence | `SC-05b` (`SC-05B-FP-DURING-POST-HOLD`) | Fail | logic_mismatch | 0/0 |
| FP before HP triggers cold path with AF lead | `SC-06` (`SC-06-COLD-FP`) | Pass | pass | 0/0 |
| HP activity during burst does not alter frame schedule | `SC-07` (`SC-07-HP-DURING-BURST`) | Pass | pass | 0/0 |
| HP during post-hold may extend hold but not fire FP | `SC-07b` (`SC-07B-HP-DURING-POST-HOLD`) | Pass | pass | 0/0 |
| FP before HP then later HP should keep cold path stable | `SC-08` (`SC-08-FP-BEFORE-HP`) | Pass | pass | 0/0 |
| Additional FP rejected at MaxSequenceCount cap | `SC-09` (`SC-09-SEQUENCE-CAP`) | Pass | resolved | 0/0 |
| New event after cap end starts fresh activity | `SC-10` (`SC-10-RECOVERY-AFTER-CAP`) | Pass | pass | 0/0 |
| Validate T gating frame 1 and Y spacing for later frames | `SC-11` (`SC-11-SPACING-VS-T`) | Pass | pass | 0/0 |
| HP-only no-sequence behavior at field-like stimulus | `SC-12` (`SC-12-HP-ONLY-MIN-GAP`) | Pass | pass | 0/0 |
| Synthetic bounce should be rejected by debounce windows | `SC-13` (`SC-13-BOUNCE-DEBOUNCE`) | Pass | pass | 0/0 |
| Held FP should count once, not continuous output | `SC-14` (`SC-14-HELD-VS-PULSED-FP`) | Pass | pass | 1/1 |
| Compare latency paths with powerSaveIdleMode enabled vs disabled | `SC-15` (`SC-15-POWER-SAVE-BUDGET`) | Pass | pass | 0/0 |
| HP release after FP should not drop HP_OUT continuity | `SC-16` (`SC-16-HP-RELEASE-AFTER-FP`) | Pass | pass | 0/0 |
| First frame follows max(FP accept, HP_OUT assert + T) | `SC-17` (`SC-17-SHORT-HP-LEAD`) | Pass | pass | 0/0 |
| HP chatter/release during burst does not alter scheduling | `SC-18` (`SC-18-HP-CHATTER-BURST`) | Pass | pass | 0/0 |
| New FP after HP release should be cold/short-lead behavior | `SC-19` (`SC-19-NEW-EVENT-AFTER-RELEASE`) | Pass | pass | 0/0 |
| T may delay frame 1 but not add delay to frames 2..N | `SC-20` (`SC-20-T-GREATER-THAN-Y`) | Pass | pass | 0/0 |
| Re-run SC-04 while BLE remains connected | `ADDON-BLE-CONNECTED` (`AO-BLE-CONNECTED-SC04`) | Pass | pass | 0/0 |
| Re-run SC-01 while BLE remains connected | `ADDON-BLE-CONNECTED` (`AO-BLE-CONNECTED-SC01`) | Pass | pass | 0/0 |
| fullPressIgnoreGap just-before/exact/just-after checks | `ADDON-GAP-BOUNDARY` (`AO-GAP-BOUNDARY-TRIAD`) | Pass | pass | 0/0 |
| Write config during active flow and verify defer/reject behavior | `ADDON-DEFERRED-CONFIG` (`AO-DEFERRED-CONFIG-WRITES`) | Pass | pass | 0/0 |
| Verify factory reset behavior and reserved field coercion | `ADDON-FACTORY-RESET` (`AO-FACTORY-RESET-AND-COERCION`) | Pass | pass | 0/0 |

## Results Statistics

### Assertion Coverage And Compliance

| Assertion Class | Passed | Total | Compliance |
|---|---:|---:|---:|
| Timing assertions | 2 | 2 | 100.0% |
| Functional assertions | 56 | 57 | 98.2% |

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
| `frameStartSpacingMs` around 0.2 s (rapid cadence) | 3 | 199.67 ms | 199.78 ms | 200 ms | `SC-20`, `AO-BLE-CONNECTED-SC01`, `AO-GAP-BOUNDARY-TRIAD` |
| `frameStartSpacingMs` around 1.0 s (nominal cadence) | 16 | 998.67 ms | 998.92 ms | 999 ms | `SC-01`, `SC-02`, `SC-03`, `SC-06`, `SC-07`, `SC-07b`, `SC-08`, `SC-11`, `SC-13`, `SC-14`, `SC-15`, `SC-16`, `SC-17`, `SC-18`, `AO-DEFERRED-CONFIG-WRITES`, `AO-FACTORY-RESET-AND-COERCION` |
| `frameStartSpacingMs` around 1.2-2.0 s (extended/cap windows) | 4 | 1.171 s | 1.555 s | 1.930 s | `SC-05`, `SC-05b`, `SC-09`, `SC-19` |
| `frameStartSpacingMs` around 4.2 s (recovery cadence) | 1 | 4.167 s | 4.167 s | 4.167 s | `SC-10` |

All other timing metrics are reported per-test in the detailed sections to preserve command/context-specific meaning.

## Detailed Test Results

### Normal wake then shoot (`SC-01-NOMINAL`)

| Field | Value |
|---|---|
| Requirement descriptor | Normal wake then shoot |
| Reference | `SC-01` |
| Coverage tags | core, nominal |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 101 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 53 ms |
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
| `fpPulseWidthMs` | Pass | expected 100.0 +/- 5.0, got 101.0 |
| `sequenceCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

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
| `fpPulseWidthMs` | 99.25 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 5 ms |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 1 |
| `ignoredFpDuringBurstCount` | 1 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

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
| `fpPulseWidthMs` | 99 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 42 ms |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 3 |
| `ignoredFpDuringBurstCount` | 3 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 3, 'ignoredFpDuringBurstCount': 3, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

### HP only path should timeout with no FP (`SC-04-WAKE-TIMEOUT`)

| Field | Value |
|---|---|
| Requirement descriptor | HP only path should timeout with no FP |
| Reference | `SC-04` |
| Coverage tags | core, wake-only |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | 56 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Repeated HP pulses extend wake hold (`SC-04B-REPEATED-HP`)

| Field | Value |
|---|---|
| Requirement descriptor | Repeated HP pulses extend wake hold |
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
| `hpInToHpOutLatencyMs` | 50 ms |
| `frameCount` | 0 |
| `sequenceCount` | 0 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 0 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 0, got 0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0} |

### FP after burst starts second sequence under cap (`SC-05-BACK-TO-BACK`)

| Field | Value |
|---|---|
| Requirement descriptor | FP after burst starts second sequence under cap |
| Reference | `SC-05` |
| Coverage tags | core, multi-sequence |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.88 ms |
| `frameStartSpacingMs` | 1.285 s |
| `fpInToFpOutLatencyMs` | 1 ms |
| `hpInToHpOutLatencyMs` | 49 ms |
| `frameCount` | 8 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `sequenceCount` | Pass | expected 1, got 1 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 0} |

### FP during post-shutter hold can start next sequence (`SC-05B-FP-DURING-POST-HOLD`)

| Field | Value |
|---|---|
| Requirement descriptor | FP during post-shutter hold can start next sequence |
| Reference | `SC-05b` |
| Coverage tags | variant, multi-sequence |
| Outcome | Fail |
| Failure class | logic_mismatch |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.5 ms |
| `frameStartSpacingMs` | 1.171 s |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 48 ms |
| `frameCount` | 8 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Fail | expected 4, got 8 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 0} |

### FP before HP triggers cold path with AF lead (`SC-06-COLD-FP`)

| Field | Value |
|---|---|
| Requirement descriptor | FP before HP triggers cold path with AF lead |
| Reference | `SC-06` |
| Coverage tags | core, cold-fp |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 98.75 ms |
| `frameStartSpacingMs` | 998.67 ms |
| `fpInToFpOutLatencyMs` | 496 ms |
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
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

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
| `fpPulseWidthMs` | 99 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 50 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 1, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

### HP during post-hold may extend hold but not fire FP (`SC-07B-HP-DURING-POST-HOLD`)

| Field | Value |
|---|---|
| Requirement descriptor | HP during post-hold may extend hold but not fire FP |
| Reference | `SC-07b` |
| Coverage tags | variant, hp-during-burst |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.75 ms |
| `frameStartSpacingMs` | 998.67 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 49 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

### FP before HP then later HP should keep cold path stable (`SC-08-FP-BEFORE-HP`)

| Field | Value |
|---|---|
| Requirement descriptor | FP before HP then later HP should keep cold path stable |
| Reference | `SC-08` |
| Coverage tags | core, cold-fp |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 491 ms |
| `hpInToHpOutLatencyMs` | -150 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0} |

### Additional FP rejected at MaxSequenceCount cap (`SC-09-SEQUENCE-CAP`)

| Field | Value |
|---|---|
| Requirement descriptor | Additional FP rejected at MaxSequenceCount cap |
| Reference | `SC-09` |
| Coverage tags | core, cap |
| Outcome | Pass |
| Failure class | resolved |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100.25 ms |
| `frameStartSpacingMs` | 1.833 s |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 49 ms |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 2, got 2 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2} |

### New event after cap end starts fresh activity (`SC-10-RECOVERY-AFTER-CAP`)

| Field | Value |
|---|---|
| Requirement descriptor | New event after cap end starts fresh activity |
| Reference | `SC-10` |
| Coverage tags | core, cap |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 ms |
| `frameStartSpacingMs` | 4.167 s |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 46 ms |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2} |

### Validate T gating frame 1 and Y spacing for later frames (`SC-11-SPACING-VS-T`)

| Field | Value |
|---|---|
| Requirement descriptor | Validate T gating frame 1 and Y spacing for later frames |
| Reference | `SC-11` |
| Coverage tags | core, timing |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 106.75 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 450 ms |
| `hpInToHpOutLatencyMs` | 51 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP-only no-sequence behavior at field-like stimulus (`SC-12-HP-ONLY-MIN-GAP`)

| Field | Value |
|---|---|
| Requirement descriptor | HP-only no-sequence behavior at field-like stimulus |
| Reference | `SC-12` |
| Coverage tags | core, wake-only |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | 56 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Synthetic bounce should be rejected by debounce windows (`SC-13-BOUNCE-DEBOUNCE`)

| Field | Value |
|---|---|
| Requirement descriptor | Synthetic bounce should be rejected by debounce windows |
| Reference | `SC-13` |
| Coverage tags | core, debounce |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 101 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 12 ms |
| `frameCount` | 4 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 1 |
| `ignoredFpDuringBurstCount` | 1 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 4, got 4 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 1, 'hpDebounceRejectCount': 1, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

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
| `fpPulseWidthMs` | 101 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 10 ms |
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
| `fpPulseWidthMs` | Pass | expected 100.0 +/- 5.0, got 101.0 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

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
| `fpPulseWidthMs` | 100.25 ms |
| `frameStartSpacingMs` | 998.67 ms |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 54 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### HP release after FP should not drop HP_OUT continuity (`SC-16-HP-RELEASE-AFTER-FP`)

| Field | Value |
|---|---|
| Requirement descriptor | HP release after FP should not drop HP_OUT continuity |
| Reference | `SC-16` |
| Coverage tags | core, hp-continuity |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 89.75 ms |
| `frameStartSpacingMs` | 998.67 ms |
| `fpInToFpOutLatencyMs` | 462 ms |
| `hpInToHpOutLatencyMs` | 11 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### First frame follows max(FP accept, HP_OUT assert + T) (`SC-17-SHORT-HP-LEAD`)

| Field | Value |
|---|---|
| Requirement descriptor | First frame follows max(FP accept, HP_OUT assert + T) |
| Reference | `SC-17` |
| Coverage tags | core, af-gate |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 1.879 s |
| `hpInToHpOutLatencyMs` | 50 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

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
| `fpPulseWidthMs` | 99.25 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 970 ms |
| `hpInToHpOutLatencyMs` | 43 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 2, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### New FP after HP release should be cold/short-lead behavior (`SC-19-NEW-EVENT-AFTER-RELEASE`)

| Field | Value |
|---|---|
| Requirement descriptor | New FP after HP release should be cold/short-lead behavior |
| Reference | `SC-19` |
| Coverage tags | core, cold-fp |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 100 ms |
| `frameStartSpacingMs` | 1.930 s |
| `fpInToFpOutLatencyMs` | 0 ms |
| `hpInToHpOutLatencyMs` | 5 ms |
| `frameCount` | 8 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 2 |
| `ignoredFpDuringGapCount` | 0 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 0 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 8, got 8 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1} |

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
| `fpPulseWidthMs` | 99.25 ms |
| `frameStartSpacingMs` | 199.67 ms |
| `fpInToFpOutLatencyMs` | 1.880 s |
| `hpInToHpOutLatencyMs` | 52 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Re-run SC-04 while BLE remains connected (`AO-BLE-CONNECTED-SC04`)

| Field | Value |
|---|---|
| Requirement descriptor | Re-run SC-04 while BLE remains connected |
| Reference | `ADDON-BLE-CONNECTED` |
| Coverage tags | addon, ble-connected |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | n/a |
| `frameStartSpacingMs` | n/a |
| `fpInToFpOutLatencyMs` | n/a |
| `hpInToHpOutLatencyMs` | 57 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1} |

### Re-run SC-01 while BLE remains connected (`AO-BLE-CONNECTED-SC01`)

| Field | Value |
|---|---|
| Requirement descriptor | Re-run SC-01 while BLE remains connected |
| Reference | `ADDON-BLE-CONNECTED` |
| Coverage tags | addon, ble-connected |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99.75 ms |
| `frameStartSpacingMs` | 199.67 ms |
| `fpInToFpOutLatencyMs` | 978 ms |
| `hpInToHpOutLatencyMs` | 51 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### fullPressIgnoreGap just-before/exact/just-after checks (`AO-GAP-BOUNDARY-TRIAD`)

| Field | Value |
|---|---|
| Requirement descriptor | fullPressIgnoreGap just-before/exact/just-after checks |
| Reference | `ADDON-GAP-BOUNDARY` |
| Coverage tags | addon, boundary |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 99 ms |
| `frameStartSpacingMs` | 200 ms |
| `fpInToFpOutLatencyMs` | 972 ms |
| `hpInToHpOutLatencyMs` | 40 ms |
| `frameCount` | 2 |
| `sequenceCount` | 1 |

#### Telemetry Delta Highlights

| Counter | Delta |
|---|---:|
| `acceptedFpCount` | 1 |
| `ignoredFpDuringGapCount` | 1 |
| `ignoredFpDuringBurstCount` | 0 |
| `rejectedFpAtSequenceCapCount` | 2 |
| `wakeTimeoutCount` | 0 |

#### Check Outcomes

| Check | Result | Detail |
|---|---|---|
| `frameCount` | Pass | expected 2, got 2 |
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 2, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

### Write config during active flow and verify defer/reject behavior (`AO-DEFERRED-CONFIG-WRITES`)

| Field | Value |
|---|---|
| Requirement descriptor | Write config during active flow and verify defer/reject behavior |
| Reference | `ADDON-DEFERRED-CONFIG` |
| Coverage tags | addon, config-write |
| Outcome | Pass |
| Failure class | pass |

#### Performance Metrics

| Metric | Value |
|---|---:|
| `fpPulseWidthMs` | 101 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 972 ms |
| `hpInToHpOutLatencyMs` | 52 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

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
| `fpPulseWidthMs` | 99.25 ms |
| `frameStartSpacingMs` | 999 ms |
| `fpInToFpOutLatencyMs` | 984 ms |
| `hpInToHpOutLatencyMs` | 53 ms |
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
| `telemetry_delta_sanity` | Pass | deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1} |

