# Camptraptions Timing Validation Test Report

## Executive Summary

This report captures results from the latest **BLE-enabled authoritative full-suite run**.

- Suite: `TickleBoard/vectors/suites/full_validation_suite.yaml`
- Artifacts root: `TickleBoard/artifacts`
- Run batch: `20260523_124553`
- Cases executed: 38
- Outcome: **38 passed, 0 failed, 0 skipped**
- Timing assertions: **94/94 passed**
- Functional assertions: **167/167 passed**
- Protocol assertions: **24/24 passed**

## Master Results Table

| Case | Scenario | Status | Failure Class | Timing | Functional | Notes |
|------|----------|--------|---------------|--------|------------|-------|
| SC-01-NOMINAL | SC-01 | passed | pass | 6/6 | 7/7 |  |
| SC-02-FP-DURING-SEQUENCE | SC-02 | passed | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-03-FP-FLOOD | SC-03 | passed | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-04-WAKE-TIMEOUT | SC-04 | passed | pass | 2/2 | 3/3 |  |
| SC-04B-REPEATED-HP | SC-04b | passed | pass | 2/2 | 3/3 |  |
| SC-05-BACK-TO-BACK | SC-05 | passed | pass | 5/5 | 7/7 |  |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | passed | pass | 5/5 | 7/7 |  |
| SC-06-COLD-FP | SC-06 | passed | pass | 2/2 | 3/3 |  |
| SC-07-HP-DURING-BURST | SC-07 | passed | pass | 2/2 | 3/3 |  |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | passed | pass | 2/2 | 3/3 |  |
| SC-08-FP-BEFORE-HP | SC-08 | passed | pass | 3/3 | 3/3 |  |
| SC-09-SEQUENCE-CAP | SC-09 | passed | pass | 2/2 | 4/4 |  |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | passed | pass | 3/3 | 7/7 |  |
| SC-11-SPACING-VS-T | SC-11 | passed | pass | 3/3 | 2/2 |  |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | passed | pass | 2/2 | 3/3 |  |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | passed | pass | 2/2 | 3/3 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-14-HELD-VS-PULSED-FP | SC-14 | passed | pass | 3/3 | 2/2 |  |
| SC-15-POWER-SAVE-BUDGET | SC-15 | passed | pass | 2/2 | 3/3 |  |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | passed | pass | 6/6 | 2/2 |  |
| SC-17-SHORT-HP-LEAD | SC-17 | passed | pass | 3/3 | 2/2 |  |
| SC-18-HP-CHATTER-BURST | SC-18 | passed | pass | 2/2 | 3/3 |  |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | passed | pass | 4/4 | 7/7 |  |
| SC-20-T-GREATER-THAN-Y | SC-20 | passed | pass | 3/3 | 2/2 |  |
| AO-TELEMETRY-FIELD-COVERAGE | ADDON-TELEMETRY | passed | pass | 2/2 | 32/32 |  |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |
| AO-LATENCY-HP-IN-TO-HP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 1/1 |  |
| AO-LATENCY-FP-IN-TO-FP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 3/3 |  |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | passed | pass | 3/3 | 7/7 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| AO-GAP-CADENCE | ADDON-GAP-CADENCE | passed | pass | 3/3 | 7/7 |  |
| AO-MAX-SEQUENCE-RANGE-64 | ADDON-MAX-SEQUENCE-RANGE | passed | pass | 2/2 | 7/7 |  |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | passed | pass | 2/2 | 3/3 |  |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | passed | pass | 2/2 | 2/2 |  |
| AO-BOUNDS-COERCE-MIN | ADDON-BOUNDS-COERCE | passed | pass | 3/3 | 4/4 |  |
| AO-BOUNDS-COERCE-MAX | ADDON-BOUNDS-COERCE | passed | pass | 1/1 | 3/3 | telemetry_after_idle_timeout_state=3_flags=0x03 |
| AO-CAMCFG-NACK-BAD-VERSION | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |
| AO-CAMCFG-NACK-BAD-LENGTH | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |
| AO-CAMCFG-NACK-OUT-OF-RANGE | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |

## Results Statistics

- Total cases: 38
- Passed: 38
- Failed: 0
- Skipped: 0
- Timing assertions passed: 94/94
- Functional assertions passed: 167/167
- Protocol assertions passed: 24/24

## Detailed Test Results

### SC-01-NOMINAL (SC-01)

- Status: passed
- Failure class: pass
- Timing assertions: 6/6
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_124553_SC-01-NOMINAL`
- Description: Normal wake then shoot
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5592.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `fpPulseWidthMs` (timing) - expected 100.0 +/- 5.0, got 99.8
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 4, got 4
  - [PASS] `sequenceCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=5592.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 4396.0+2000.0) = 10000.0 +/- 100.0, got 9988.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-02-FP-DURING-SEQUENCE (SC-02)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\20260523_124615_SC-02-FP-DURING-SEQUENCE`
- Description: Extra FP during sequence should be ignored
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; interSequenceGapMs=999.0; secondSequenceStartDelayMs=698.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-03-FP-FLOOD (SC-03)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\20260523_124636_SC-03-FP-FLOOD`
- Description: FP flood during burst still yields one sequence
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5592.0; interSequenceGapMs=998.0; secondSequenceStartDelayMs=898.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 3, got 3
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 3, got 3
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 3, 'ignoredFpDuringBurstCount': 3, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-04-WAKE-TIMEOUT (SC-04)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124657_SC-04-WAKE-TIMEOUT`
- Description: HP only path should timeout with no FP
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9989.0; wakeOnlyHoldMs=9989.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9989.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-04B-REPEATED-HP (SC-04b)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124719_SC-04B-REPEATED-HP`
- Description: Repeated HP pulses do not extend wake hold
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9989.0; wakeOnlyHoldMs=9989.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.hpRefreshCount` (functional) - expected 2, got 2
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9989.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-05-BACK-TO-BACK (SC-05)

- Status: passed
- Failure class: pass
- Timing assertions: 5/5
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_124748_SC-05-BACK-TO-BACK`
- Description: FP after burst starts second sequence under cap
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.9; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1098.5; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=12394.0; hpHoldAfterLastFrameMs=1998.0; interSequenceGapMs=2605.0; secondSequenceStartDelayMs=0.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=10396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [2500.0, 3500.0], got 2605.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=1998.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 10396.0+2000.0) = 12396.0 +/- 124.0, got 12394.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-05B-FP-DURING-POST-HOLD (SC-05b)

- Status: passed
- Failure class: pass
- Timing assertions: 5/5
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_124814_SC-05B-FP-DURING-POST-HOLD`
- Description: FP during post-shutter hold can start next sequence
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.6; frameEndToStartSpacingMs=1114.0; frameStartSpacingMs=1213.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=11593.0; hpHoldAfterLastFrameMs=1998.0; interSequenceGapMs=1804.0; secondSequenceStartDelayMs=0.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=9595.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [1800.0, 2600.0], got 1804.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=1998.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 9595.0+2000.0) = 11595.0 +/- 116.0, got 11593.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-06-COLD-FP (SC-06)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124838_SC-06-COLD-FP`
- Description: FP before HP triggers cold path with AF lead
- Key metrics: frameCount=4; sequenceCount=1; fpInToFpOutLatencyMs=499.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=499.0; firstFrameAfLeadMs=499.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=3896.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [450.0, 650.0], got 499.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-07-HP-DURING-BURST (SC-07)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124859_SC-07-HP-DURING-BURST`
- Description: HP activity during burst does not alter frame schedule
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5592.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.hpIgnoredDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 1, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-07B-HP-DURING-POST-HOLD (SC-07b)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124920_SC-07B-HP-DURING-POST-HOLD`
- Description: HP during post-hold does not move wake hold and does not fire FP
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5594.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4395.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.hpRefreshCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-08-FP-BEFORE-HP (SC-08)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_124943_SC-08-FP-BEFORE-HP`
- Description: FP before HP then later HP should keep cold path stable
- Key metrics: frameCount=4; sequenceCount=1; fpInToFpOutLatencyMs=499.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=499.0; firstFrameAfLeadMs=499.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=3896.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [450.0, 650.0], got 499.0
  - [PASS] `hpInToHpOutLatencyMs_preasserted_handling` (timing) - expected unmeasurable latency when HP_OUT pre-asserted (latency=None, reason=hp_out_already_asserted_before_hp_in)
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-09-SEQUENCE-CAP (SC-09)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 4/4
- Artifact path: `TickleBoard\artifacts\20260523_125004_SC-09-SEQUENCE-CAP`
- Description: Cap ends current activity; later HP+FP can start a new sequence activity
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.5; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=5000.0; interSequenceGapMs=4501.0; secondSequenceStartDelayMs=1699.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=7898.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.MaxSequenceExceededCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 1, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2, 'bootCount': 0}

### SC-10-RECOVERY-AFTER-CAP (SC-10)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_125032_SC-10-RECOVERY-AFTER-CAP`
- Description: Recovery after cap timeout resumes sequence starts
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.5; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=17289.0; hpHoldAfterLastFrameMs=8292.0; interSequenceGapMs=5601.0; secondSequenceStartDelayMs=2799.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=8997.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 4, got 4
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.MaxSequenceExceededCount` (functional) - expected 1, got 1
  - [PASS] `interSequenceGapMs` (timing) - expected range [2000.0, inf], got 5601.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 1, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2, 'bootCount': 0}

### SC-11-SPACING-VS-T (SC-11)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125102_SC-11-SPACING-VS-T`
- Description: Validate T gating frame 1 and Y spacing for later frames
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=399.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1099.0; firstFrameGateDelayMs=399.0; firstFrameAfLeadMs=499.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=6093.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=3896.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [350.0, 650.0], got 399.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-12-HP-ONLY-MIN-GAP (SC-12)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125123_SC-12-HP-ONLY-MIN-GAP`
- Description: HP-only no-sequence behavior at field-like stimulus
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9989.0; wakeOnlyHoldMs=9989.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9989.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-13-BOUNCE-DEBOUNCE (SC-13)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125145_SC-13-BOUNCE-DEBOUNCE`
- Description: Synthetic bounce should be rejected by debounce windows
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=2000.0; interSequenceGapMs=999.0; secondSequenceStartDelayMs=1088.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=5395.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.fpDebounceRejectCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 1, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-14-HELD-VS-PULSED-FP (SC-14)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125206_SC-14-HELD-VS-PULSED-FP`
- Description: Held FP should count once, not continuous output
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5592.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `fpPulseWidthMs` (timing) - expected 100.0 +/- 5.0, got 99.8
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-15-POWER-SAVE-BUDGET (SC-15)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125227_SC-15-POWER-SAVE-BUDGET`
- Description: Compare latency paths with powerSaveIdleMode enabled vs disabled
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-16-HP-RELEASE-AFTER-FP (SC-16)

- Status: passed
- Failure class: pass
- Timing assertions: 6/6
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125248_SC-16-HP-RELEASE-AFTER-FP`
- Description: HP release after FP should not drop HP_OUT continuity
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=399.0; fpPulseWidthMs=100.0; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=399.0; firstFrameAfLeadMs=499.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=6093.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=3896.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `firstFrameAfLeadMs` (timing) - expected range [450.0, 650.0], got 499.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=6093.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 3896.0+2000.0) = 10000.0 +/- 100.0, got 9989.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-17-SHORT-HP-LEAD (SC-17)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125310_SC-17-SHORT-HP-LEAD`
- Description: First frame follows max(FP accept, HP_OUT assert + T)
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=1897.0; fpPulseWidthMs=100.2; frameEndToStartSpacingMs=998.7; frameStartSpacingMs=1099.0; firstFrameGateDelayMs=1897.0; firstFrameAfLeadMs=1997.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=5394.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [1750.0, 2150.0], got 1897.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-18-HP-CHATTER-BURST (SC-18)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125332_SC-18-HP-CHATTER-BURST`
- Description: HP chatter/release during burst does not alter scheduling
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5593.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4395.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.hpIgnoredDuringBurstCount` (functional) - expected 2, got 2
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 2, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### SC-19-NEW-EVENT-AFTER-RELEASE (SC-19)

- Status: passed
- Failure class: pass
- Timing assertions: 4/4
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_125353_SC-19-NEW-EVENT-AFTER-RELEASE`
- Description: New FP after HP release should be cold/short-lead behavior
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.6; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; interSequenceGapMs=7103.0; secondSequenceStartDelayMs=498.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=14894.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [6500.0, 7500.0], got 7103.0
  - [PASS] `secondSequenceStartDelayMs` (timing) - expected range [450.0, 700.0], got 498.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 2, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2, 'bootCount': 0}

### SC-20-T-GREATER-THAN-Y (SC-20)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125424_SC-20-T-GREATER-THAN-Y`
- Description: T may delay frame 1 but not add delay to frames 2..N
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=1897.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=200.0; frameStartSpacingMs=300.0; firstFrameGateDelayMs=1897.0; firstFrameAfLeadMs=1997.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=2996.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 200.0 +/- 5.0, got 200.0
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [1750.0, 2150.0], got 1897.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-TELEMETRY-FIELD-COVERAGE (ADDON-TELEMETRY)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 32/32
- Artifact path: `TickleBoard\artifacts\20260523_125445_AO-TELEMETRY-FIELD-COVERAGE`
- Description: Validate all telemetry fields decode and report their values
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4395.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}
  - [PASS] `telemetryFields.after.requireAllPresent` (functional) - all parsed telemetry fields present
  - [PASS] `telemetryFields.after.equals.version` (functional) - expected 3, got 3
  - [PASS] `telemetryFields.after.equals.counters.version` (functional) - expected 3, got 3
  - [PASS] `telemetryFields.after.range.camera_state` (functional) - expected [0, 4], got 0
  - [PASS] `telemetryFields.after.range.flags` (functional) - expected [0, 3], got 0
  - [PASS] `telemetryFields.after.range.frames_fired_this_sequence` (functional) - expected [0, 8], got 0
  - [PASS] `telemetryFields.after.range.sequences_started_this_activity` (functional) - expected [0, 64], got 0
  - [PASS] `telemetryFields.after.range.last_event_code` (functional) - expected [0, 255], got 4
  - [PASS] `telemetryFields.after.range.last_scenario_hint` (functional) - expected [0, 255], got 1
  - [PASS] `telemetryFields.after.range.ms_until_wake_deadline` (functional) - expected [0, 120000], got 0
  - [PASS] `telemetryFields.after.range.ms_until_fp_ignore_clear` (functional) - expected [0, 120000], got 0
  - [PASS] `telemetryFields.after.range.ms_until_next_frame` (functional) - expected [0, 120000], got 0
  - [PASS] `telemetryFields.after.range.ms_until_post_hold_end` (functional) - expected [0, 120000], got 0
  - [PASS] `telemetryFields.after.range.boot_reset_raw` (functional) - expected [0, 65535], got 0
  - [PASS] `telemetryFields.after.range.boot_temp_c_x100` (functional) - expected [-4000, 12500], got 3825
  - [PASS] `telemetryFields.after.range.counters.wakeTimeoutCount` (functional) - expected [0, 4294967295], got 29
  - [PASS] `telemetryFields.after.range.counters.acceptedFpCount` (functional) - expected [0, 4294967295], got 30
  - [PASS] `telemetryFields.after.range.counters.ignoredFpDuringGapCount` (functional) - expected [0, 4294967295], got 5
  - [PASS] `telemetryFields.after.range.counters.ignoredFpDuringBurstCount` (functional) - expected [0, 4294967295], got 5
  - [PASS] `telemetryFields.after.range.counters.MaxSequenceExceededCount` (functional) - expected [0, 4294967295], got 2
  - [PASS] `telemetryFields.after.range.counters.coldFpSequenceCount` (functional) - expected [0, 4294967295], got 5
  - [PASS] `telemetryFields.after.range.counters.hpRefreshCount` (functional) - expected [0, 4294967295], got 6
  - [PASS] `telemetryFields.after.range.counters.hpIgnoredDuringBurstCount` (functional) - expected [0, 4294967295], got 3
  - [PASS] `telemetryFields.after.range.counters.fpDebounceRejectCount` (functional) - expected [0, 4294967295], got 1
  - [PASS] `telemetryFields.after.range.counters.hpDebounceRejectCount` (functional) - expected [0, 4294967295], got 0
  - [PASS] `telemetryFields.after.range.counters.sequenceCompletedCount` (functional) - expected [0, 4294967295], got 30
  - [PASS] `telemetryFields.after.range.counters.activityCompletedCount` (functional) - expected [0, 4294967295], got 31
  - [PASS] `telemetryFields.after.range.counters.bootCount` (functional) - expected [1, 4294967295], got 5
  - [PASS] `telemetryFields.after.report` (functional) - boot_reset_raw=0; boot_temp_c_x100=3825; camera_state=0; counters.MaxSequenceExceededCount=2; counters.acceptedFpCount=30; counters.activityCompletedCount=31; counters.bootCount=5; counters.coldFpSequenceCount=5; counters.fpDebounceRejectCount=1; counters.hpDebounceRejectCount=0; counters.hpIgnoredDuringBurstCount=3; counters.hpRefreshCount=6; counters.ignoredFpDuringBurstCount=5; counters.ignoredFpDuringGapCount=5; counters.sequenceCompletedCount=30; counters.version=3; counters.wakeTimeoutCount=29; flags=0; frames_fired_this_sequence=0; last_event_code=4; last_scenario_hint=1; ms_until_fp_ignore_clear=0; ms_until_next_frame=0; ms_until_post_hold_end=0; ms_until_wake_deadline=0; sequences_started_this_activity=0; version=3; boot_reset_reason=power_on_or_unknown; boot_temp_c=38.25; boot_temp_f=100.85

### AO-BLE-CONNECTED-SC04 (ADDON-BLE-CONNECTED)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125506_AO-BLE-CONNECTED-SC04`
- Description: Re-run SC-04 while BLE remains connected
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9988.0; wakeOnlyHoldMs=9988.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9988.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-BLE-CONNECTED-SC01 (ADDON-BLE-CONNECTED)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125529_AO-BLE-CONNECTED-SC01`
- Description: Re-run SC-01 while BLE remains connected
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4395.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-LATENCY-HP-IN-TO-HP-OUT (ADDON-LATENCY)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 1/1
- Artifact path: `TickleBoard\artifacts\20260523_125550_AO-LATENCY-HP-IN-TO-HP-OUT`
- Description: Measure HP_IN to HP_OUT assertion latency under suite runtime conditions
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `hpInToHpOutLatencyMs` (timing) - expected range [0.0, 200.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-LATENCY-FP-IN-TO-FP-OUT (ADDON-LATENCY)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125604_AO-LATENCY-FP-IN-TO-FP-OUT`
- Description: Measure FP_IN to FP_OUT latency with HP lead pre-satisfied
- Key metrics: frameCount=1; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=98.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=600.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=698.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `fpInToFpOutLatencyMs` (timing) - expected range [0.0, 200.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-GAP-BOUNDARY-TRIAD (ADDON-GAP-BOUNDARY)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_125625_AO-GAP-BOUNDARY-TRIAD`
- Description: Hermetic fullPressIgnoreGap boundary triad with explicit runtime parameters
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=3; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=4691.0; interSequenceGapMs=1902.0; secondSequenceStartDelayMs=100.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=5298.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.MaxSequenceExceededCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [0.0, 50.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 2, 'ignoredFpDuringBurstCount': 1, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-GAP-CADENCE (ADDON-GAP-CADENCE)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_125650_AO-GAP-CADENCE`
- Description: Hermetic 1 second inter-frame cadence check (1000/2000/3000/4000)
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.3; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=5592.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.MaxSequenceExceededCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [0.0, 50.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-MAX-SEQUENCE-RANGE-64 (ADDON-MAX-SEQUENCE-RANGE)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\20260523_125715_AO-MAX-SEQUENCE-RANGE-64`
- Description: MaxSequenceCount accepts values above 8 and allows nine sequences when set to 64
- Key metrics: frameCount=9; sequenceCount=9; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.0; frameEndToStartSpacingMs=601.0; frameStartSpacingMs=700.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=3290.0; interSequenceGapMs=601.0; secondSequenceStartDelayMs=0.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=6699.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 9, got 9
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected range [500.0, inf], got 601.0
  - [PASS] `sequenceCount.telemetryAcceptedFpCount` (functional) - expected 9, got 9
  - [PASS] `sequenceCount.fpOutPulseCount` (functional) - expected 9, got 9
  - [PASS] `sequenceCount` (functional) - expected 9, got 9
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 9, got 9
  - [PASS] `telemetryDelta.MaxSequenceExceededCount` (functional) - expected 0, got 0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 9, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 9, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-DEFERRED-CONFIG-WRITES (ADDON-DEFERRED-CONFIG)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Protocol assertions: 5/5
- Artifact path: `TickleBoard\artifacts\20260523_125738_AO-DEFERRED-CONFIG-WRITES`
- Description: Config write during active burst is rejected with NACK_BUSY and leaves config unchanged
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5594.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4395.0
- Assertion details:
  - [PASS] `protocol[0].writeStatus` (protocol) - expected ACK_APPLIED (0x00), got ACK_APPLIED (0x00)
  - [PASS] `protocol[1].activityActiveAtWrite` (protocol) - camera_state=3 flags=0x03
  - [PASS] `protocol[1].writeStatus` (protocol) - expected NACK_BUSY (0xE3), got NACK_BUSY (0xE3)
  - [PASS] `protocol[1].configUnchanged` (protocol) - readback unchanged
  - [PASS] `protocol[1].readback.FrameCount` (protocol) - expected 4, got 4
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 2, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 2, 'bootCount': 0}

### AO-FACTORY-RESET-AND-COERCION (ADDON-FACTORY-RESET)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 2/2
- Protocol assertions: 9/9
- Artifact path: `TickleBoard\artifacts\20260523_125814_AO-FACTORY-RESET-AND-COERCION`
- Description: Factory reset restores factory defaults; subsequent baseline write applies test params
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.8; frameEndToStartSpacingMs=999.0; frameStartSpacingMs=1098.7; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=4396.0
- Assertion details:
  - [PASS] `protocol[0].writeStatus` (protocol) - expected ACK_APPLIED (0x00), got ACK_APPLIED (0x00)
  - [PASS] `protocol[2].readback.enabled` (protocol) - expected 1, got 1
  - [PASS] `protocol[2].readback.StartFrameSpacingMin` (protocol) - expected 100, got 100
  - [PASS] `protocol[2].readback.PostShutterHalfPressHoldTimeExtension` (protocol) - expected 20, got 20
  - [PASS] `protocol[2].readback.FrameCount` (protocol) - expected 4, got 4
  - [PASS] `protocol[2].readback.MaxSequenceCount` (protocol) - expected 4, got 4
  - [PASS] `protocol[2].readback.inputActivePolarity` (protocol) - expected 0, got 0
  - [PASS] `protocol[2].readback.outputDriveMode` (protocol) - expected 0, got 0
  - [PASS] `protocol[2].readback.activityHalfPressHoldPolicy` (protocol) - expected 0, got 0
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': -37, 'acceptedFpCount': -45, 'ignoredFpDuringGapCount': -7, 'ignoredFpDuringBurstCount': -6, 'MaxSequenceExceededCount': -2, 'coldFpSequenceCount': -5, 'hpRefreshCount': -6, 'hpIgnoredDuringBurstCount': -3, 'fpDebounceRejectCount': -1, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': -45, 'activityCompletedCount': -39, 'bootCount': 0}

### AO-BOUNDS-COERCE-MIN (ADDON-BOUNDS-COERCE)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 4/4
- Artifact path: `TickleBoard\artifacts\20260523_125839_AO-BOUNDS-COERCE-MIN`
- Description: Verify 10ms lower-bound coercion for shutterPulseDuration and StartFrameSpacingMin
- Key metrics: frameCount=2; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=10.0; frameEndToStartSpacingMs=9.0; frameStartSpacingMs=19.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0; hpAssertToFinalFrameReleaseMs=1029.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 2, got 2
  - [PASS] `fpPulseWidthMs` (timing) - expected 10.0 +/- 5.0, got 10.0
  - [PASS] `frameEndToStartSpacingMs` (timing) - expected 10.0 +/- 5.0, got 9.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1, 'bootCount': 0}

### AO-BOUNDS-COERCE-MAX (ADDON-BOUNDS-COERCE)

- Status: passed
- Failure class: pass
- Timing assertions: 1/1
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\20260523_125900_AO-BOUNDS-COERCE-MAX`
- Description: Verify 30000ms upper-bound coercion for shutterPulseDuration and StartFrameSpacingMin
- Notes: telemetry_after_idle_timeout_state=3_flags=0x03
- Key metrics: frameCount=1; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; ignoredFpCount=0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0, 'bootCount': 0}

### AO-CAMCFG-NACK-BAD-VERSION (ADDON-CAMCFG-PROTOCOL)

- Status: passed
- Failure class: pass
- Timing assertions: 0/0
- Functional assertions: 1/1
- Protocol assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125930_AO-CAMCFG-NACK-BAD-VERSION`
- Description: Camera config write with wrong version byte returns NACK_BAD_FORMAT
- Key metrics: frameCount=0; sequenceCount=0; ignoredFpCount=0
- Assertion details:
  - [PASS] `protocol[0].writeStatus` (protocol) - expected NACK_BAD_FORMAT (0xE1), got NACK_BAD_FORMAT (0xE1)
  - [PASS] `protocol[0].configUnchanged` (protocol) - readback unchanged
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0, 'bootCount': 0}

### AO-CAMCFG-NACK-BAD-LENGTH (ADDON-CAMCFG-PROTOCOL)

- Status: passed
- Failure class: pass
- Timing assertions: 0/0
- Functional assertions: 1/1
- Protocol assertions: 2/2
- Artifact path: `TickleBoard\artifacts\20260523_125948_AO-CAMCFG-NACK-BAD-LENGTH`
- Description: Camera config write with wrong payload length returns NACK_BAD_FORMAT or is rejected at ATT
- Key metrics: frameCount=0; sequenceCount=0; ignoredFpCount=0
- Assertion details:
  - [PASS] `protocol[0].writeStatus` (protocol) - expected NACK_BAD_FORMAT (0xE1), got NACK_BAD_FORMAT (0xE1)
  - [PASS] `protocol[0].configUnchanged` (protocol) - readback unchanged
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0, 'bootCount': 0}

### AO-CAMCFG-NACK-OUT-OF-RANGE (ADDON-CAMCFG-PROTOCOL)

- Status: passed
- Failure class: pass
- Timing assertions: 0/0
- Functional assertions: 1/1
- Protocol assertions: 6/6
- Artifact path: `TickleBoard\artifacts\20260523_125959_AO-CAMCFG-NACK-OUT-OF-RANGE`
- Description: Camera config write with invalid reserved/out-of-range fields returns NACK_OUT_OF_RANGE
- Key metrics: frameCount=0; sequenceCount=0; ignoredFpCount=0
- Assertion details:
  - [PASS] `protocol[0].writeStatus` (protocol) - expected NACK_OUT_OF_RANGE (0xE2), got NACK_OUT_OF_RANGE (0xE2)
  - [PASS] `protocol[0].configUnchanged` (protocol) - readback unchanged
  - [PASS] `protocol[1].writeStatus` (protocol) - expected NACK_OUT_OF_RANGE (0xE2), got NACK_OUT_OF_RANGE (0xE2)
  - [PASS] `protocol[1].configUnchanged` (protocol) - readback unchanged
  - [PASS] `protocol[2].writeStatus` (protocol) - expected NACK_OUT_OF_RANGE (0xE2), got NACK_OUT_OF_RANGE (0xE2)
  - [PASS] `protocol[2].configUnchanged` (protocol) - readback unchanged
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'MaxSequenceExceededCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0, 'bootCount': 0}

## Conclusion

All BLE-enabled full-suite cases passed with complete timing, functional, and protocol assertion coverage.
