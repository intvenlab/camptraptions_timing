# Camptraptions Timing Validation Test Report

## Executive Summary

This report captures results from the latest **BLE-enabled authoritative full-suite run**.

Timeout-on-cap and telemetry rename (`MaxSequenceExceededCount`) were introduced after this archived run. Detailed assertion lines below may still show the previous counter label (`rejectedFpAtSequenceCapCount`) until the suite is rerun on the updated firmware.

- Suite: `TickleBoard/vectors/suites/full_validation_suite.yaml`
- Artifacts root: `TickleBoard/artifacts/full_run_ble_20260521_1719`
- Cases executed: 31
- Outcome: **31 passed, 0 failed, 0 skipped**
- Timing assertions: **86/86 passed**
- Functional assertions: **109/109 passed**

Reference non-BLE execution (for harness behavior checks when BLE is unavailable):

- Artifacts root: `TickleBoard/artifacts/full_run_no_ble_20260521_1731`
- Outcome: 19 passed, 2 failed, 10 skipped (non-authoritative; BLE-required checks skipped)

## Master Results Table

| Case | Scenario | Status | Failure Class | Timing | Functional | Notes |
|------|----------|--------|---------------|--------|------------|-------|
| SC-01-NOMINAL | SC-01 | passed | pass | 6/6 | 5/5 |  |
| SC-02-FP-DURING-SEQUENCE | SC-02 | passed | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-03-FP-FLOOD | SC-03 | passed | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-04-WAKE-TIMEOUT | SC-04 | passed | pass | 2/2 | 3/3 |  |
| SC-04B-REPEATED-HP | SC-04b | passed | pass | 2/2 | 3/3 |  |
| SC-05-BACK-TO-BACK | SC-05 | passed | pass | 5/5 | 5/5 |  |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | passed | pass | 5/5 | 5/5 |  |
| SC-06-COLD-FP | SC-06 | passed | pass | 2/2 | 3/3 |  |
| SC-07-HP-DURING-BURST | SC-07 | passed | pass | 2/2 | 3/3 |  |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | passed | pass | 2/2 | 3/3 |  |
| SC-08-FP-BEFORE-HP | SC-08 | passed | pass | 3/3 | 3/3 |  |
| SC-09-SEQUENCE-CAP | SC-09 | passed | pass | 2/2 | 4/4 |  |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | passed | pass | 3/3 | 5/5 |  |
| SC-11-SPACING-VS-T | SC-11 | passed | pass | 3/3 | 2/2 |  |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | passed | pass | 2/2 | 3/3 |  |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | passed | pass | 2/2 | 3/3 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-14-HELD-VS-PULSED-FP | SC-14 | passed | pass | 3/3 | 2/2 |  |
| SC-15-POWER-SAVE-BUDGET | SC-15 | passed | pass | 2/2 | 3/3 |  |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | passed | pass | 6/6 | 2/2 |  |
| SC-17-SHORT-HP-LEAD | SC-17 | passed | pass | 3/3 | 2/2 |  |
| SC-18-HP-CHATTER-BURST | SC-18 | passed | pass | 2/2 | 3/3 |  |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | passed | pass | 4/4 | 5/5 |  |
| SC-20-T-GREATER-THAN-Y | SC-20 | passed | pass | 3/3 | 2/2 |  |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |
| AO-LATENCY-HP-IN-TO-HP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 1/1 |  |
| AO-LATENCY-FP-IN-TO-FP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 3/3 |  |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | passed | pass | 3/3 | 7/7 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| AO-GAP-CADENCE | ADDON-GAP-CADENCE | passed | pass | 3/3 | 7/7 |  |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | passed | pass | 2/2 | 3/3 |  |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | passed | pass | 2/2 | 3/3 |  |

## Results Statistics

- Total cases: 31
- Passed: 31
- Failed: 0
- Skipped: 0
- Timing assertions passed: 86/86
- Functional assertions passed: 109/109

## Detailed Test Results

### SC-01-NOMINAL (SC-01)

- Status: passed
- Failure class: pass
- Timing assertions: 6/6
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172101_SC-01-NOMINAL`
- Description: Normal wake then shoot
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=54.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=98.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=946.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5948.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `fpPulseWidthMs` (timing) - expected 100.0 +/- 5.0, got 98.8
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `sequenceCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 54.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=5948.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 4041.0+2000.0) = 10000.0 +/- 100.0, got 9989.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-02-FP-DURING-SEQUENCE (SC-02)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172120_SC-02-FP-DURING-SEQUENCE`
- Description: Extra FP during sequence should be ignored
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=48.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=952.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 48.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-03-FP-FLOOD (SC-03)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172135_SC-03-FP-FLOOD`
- Description: FP flood during burst still yields one sequence
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=39.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=961.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 3, got 3
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 3, got 3
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 39.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 3, 'ignoredFpDuringBurstCount': 3, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-04-WAKE-TIMEOUT (SC-04)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172153_SC-04-WAKE-TIMEOUT`
- Description: HP only path should timeout with no FP
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=55.0; hpOutContinuityMs=9989.0; wakeOnlyHoldMs=9989.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9989.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 55.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1}

### SC-04B-REPEATED-HP (SC-04b)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172211_SC-04B-REPEATED-HP`
- Description: Repeated HP pulses do not extend wake hold
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9988.0; wakeOnlyHoldMs=9988.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.hpRefreshCount` (functional) - expected 2, got 2
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9988.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 2, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1}

### SC-05-BACK-TO-BACK (SC-05)

- Status: passed
- Failure class: pass
- Timing assertions: 5/5
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172236_SC-05-BACK-TO-BACK`
- Description: FP after burst starts second sequence under cap
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=46.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.25; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=954.0; hpOutContinuityMs=12047.0; hpHoldAfterLastFrameMs=1998.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [2500.0, 3500.0], got 2904.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 46.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=1998.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 10049.0+2000.0) = 12049.0 +/- 120.5, got 12047.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1}

### SC-05B-FP-DURING-POST-HOLD (SC-05b)

- Status: passed
- Failure class: pass
- Timing assertions: 5/5
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172259_SC-05B-FP-DURING-POST-HOLD`
- Description: FP during post-shutter hold can start next sequence
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=49.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.375; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=951.0; hpOutContinuityMs=11244.0; hpHoldAfterLastFrameMs=1998.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [1800.0, 2600.0], got 2104.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 49.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=1998.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 9246.0+2000.0) = 11246.0 +/- 112.5, got 11244.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1}

### SC-06-COLD-FP (SC-06)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172320_SC-06-COLD-FP`
- Description: FP before HP triggers cold path with AF lead
- Key metrics: frameCount=4; sequenceCount=1; fpInToFpOutLatencyMs=554.0; fpPulseWidthMs=107.5; frameStartSpacingMs=999.0; firstFrameGateDelayMs=554.0; firstFrameAfLeadMs=499.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [450.0, 650.0], got 554.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-07-HP-DURING-BURST (SC-07)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172336_SC-07-HP-DURING-BURST`
- Description: HP activity during burst does not alter frame schedule
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=46.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=954.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.hpIgnoredDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 46.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 1, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-07B-HP-DURING-POST-HOLD (SC-07b)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172353_SC-07B-HP-DURING-POST-HOLD`
- Description: HP during post-hold does not move wake hold and does not fire FP
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=46.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.25; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=954.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5939.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.hpRefreshCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 46.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-08-FP-BEFORE-HP (SC-08)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172413_SC-08-FP-BEFORE-HP`
- Description: FP before HP then later HP should keep cold path stable
- Key metrics: frameCount=4; sequenceCount=1; fpInToFpOutLatencyMs=551.0; fpPulseWidthMs=107.25; frameStartSpacingMs=999.0; firstFrameGateDelayMs=551.0; firstFrameAfLeadMs=499.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [450.0, 650.0], got 551.0
  - [PASS] `hpInToHpOutLatencyMs_preasserted_handling` (timing) - expected unmeasurable latency when HP_OUT pre-asserted (latency=None, reason=hp_out_already_asserted_before_hp_in)
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-09-SEQUENCE-CAP (SC-09)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 4/4
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172429_SC-09-SEQUENCE-CAP`
- Description: Additional FP rejected at MaxSequenceCount cap
- Key metrics: frameCount=2; sequenceCount=1; hpInToHpOutLatencyMs=43.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.5; frameStartSpacingMs=998.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=957.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=7934.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 2, got 2
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.rejectedFpAtSequenceCapCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 43.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 1, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-10-RECOVERY-AFTER-CAP (SC-10)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172448_SC-10-RECOVERY-AFTER-CAP`
- Description: New event after cap end starts fresh activity
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=50.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.5; frameStartSpacingMs=998.5; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=950.0; hpOutContinuityMs=9988.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.5
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.rejectedFpAtSequenceCapCount` (functional) - expected 0, got 0
  - [PASS] `interSequenceGapMs` (timing) - expected range [5000.0, inf], got 10400.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 50.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1}

### SC-11-SPACING-VS-T (SC-11)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172515_SC-11-SPACING-VS-T`
- Description: Validate T gating frame 1 and Y spacing for later frames
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=52.0; fpInToFpOutLatencyMs=372.0; fpPulseWidthMs=99.0; frameStartSpacingMs=999.0; firstFrameGateDelayMs=372.0; firstFrameAfLeadMs=420.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 999.0
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [350.0, 650.0], got 372.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 52.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-12-HP-ONLY-MIN-GAP (SC-12)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172532_SC-12-HP-ONLY-MIN-GAP`
- Description: HP-only no-sequence behavior at field-like stimulus
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=54.0; hpOutContinuityMs=9989.0; wakeOnlyHoldMs=9989.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9989.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 54.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1}

### SC-13-BOUNCE-DEBOUNCE (SC-13)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172551_SC-13-BOUNCE-DEBOUNCE`
- Description: Synthetic bounce should be rejected by debounce windows
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=36.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1964.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.fpDebounceRejectCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 36.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 1, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 1, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 1, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-14-HELD-VS-PULSED-FP (SC-14)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172605_SC-14-HELD-VS-PULSED-FP`
- Description: Held FP should count once, not continuous output
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=50.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.25; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=950.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5944.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `fpPulseWidthMs` (timing) - expected 100.0 +/- 5.0, got 99.2
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 50.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-15-POWER-SAVE-BUDGET (SC-15)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172623_SC-15-POWER-SAVE-BUDGET`
- Description: Compare latency paths with powerSaveIdleMode enabled vs disabled
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=54.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=946.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 54.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-16-HP-RELEASE-AFTER-FP (SC-16)

- Status: passed
- Failure class: pass
- Timing assertions: 6/6
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172636_SC-16-HP-RELEASE-AFTER-FP`
- Description: HP release after FP should not drop HP_OUT continuity
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=399.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=399.0; firstFrameAfLeadMs=499.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=6393.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `firstFrameAfLeadMs` (timing) - expected range [450.0, 650.0], got 499.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `hold.noHpReleaseBeforeFinalFrame` (timing) - expected False, got False
  - [PASS] `hold.requirePostFinalFrameHpRelease` (timing) - hpHoldAfterLastFrameMs=6393.0 reason=None
  - [PASS] `hold.dropTimeRule` (timing) - expected HP_OUT continuity by rule max(10000.0, 3595.0+2000.0) = 10000.0 +/- 100.0, got 9988.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### SC-17-SHORT-HP-LEAD (SC-17)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172655_SC-17-SHORT-HP-LEAD`
- Description: First frame follows max(FP accept, HP_OUT assert + T)
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=52.0; fpInToFpOutLatencyMs=1866.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=1866.0; firstFrameAfLeadMs=1914.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [1750.0, 2150.0], got 1866.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 52.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-18-HP-CHATTER-BURST (SC-18)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172711_SC-18-HP-CHATTER-BURST`
- Description: HP chatter/release during burst does not alter scheduling
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=45.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=98.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=955.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.hpIgnoredDuringBurstCount` (functional) - expected 2, got 2
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 45.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 2, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### SC-19-NEW-EVENT-AFTER-RELEASE (SC-19)

- Status: passed
- Failure class: pass
- Timing assertions: 4/4
- Functional assertions: 5/5
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172728_SC-19-NEW-EVENT-AFTER-RELEASE`
- Description: New FP after HP release should be cold/short-lead behavior
- Key metrics: frameCount=8; sequenceCount=2; hpInToHpOutLatencyMs=49.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.875; frameStartSpacingMs=998.8333333333334; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=951.0; hpOutContinuityMs=9990.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 8, got 8
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.8
  - [PASS] `sequenceCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.coldFpSequenceCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `interSequenceGapMs` (timing) - expected range [6500.0, 7500.0], got 7403.0
  - [PASS] `secondSequenceStartDelayMs` (timing) - expected range [450.0, 700.0], got 498.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 49.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 1, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1}

### SC-20-T-GREATER-THAN-Y (SC-20)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 2/2
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172751_SC-20-T-GREATER-THAN-Y`
- Description: T may delay frame 1 but not add delay to frames 2..N
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=51.0; fpInToFpOutLatencyMs=1865.0; fpPulseWidthMs=100.25; frameStartSpacingMs=199.66666666666666; firstFrameGateDelayMs=1865.0; firstFrameAfLeadMs=1914.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 200.0 +/- 5.0, got 199.7
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [1750.0, 2150.0], got 1865.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 51.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### AO-BLE-CONNECTED-SC04 (ADDON-BLE-CONNECTED)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172807_AO-BLE-CONNECTED-SC04`
- Description: Re-run SC-04 while BLE remains connected
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=0.0; hpOutContinuityMs=9988.0; wakeOnlyHoldMs=9988.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.wakeTimeoutCount` (functional) - expected 1, got 1
  - [PASS] `wakeOnlyHoldMs` (timing) - expected 10000.0 +/- 100.0, got 9988.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 1}

### AO-BLE-CONNECTED-SC01 (ADDON-BLE-CONNECTED)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172826_AO-BLE-CONNECTED-SC01`
- Description: Re-run SC-01 while BLE remains connected
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=52.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=98.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=948.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 52.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### AO-LATENCY-HP-IN-TO-HP-OUT (ADDON-LATENCY)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 1/1
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172842_AO-LATENCY-HP-IN-TO-HP-OUT`
- Description: Measure HP_IN to HP_OUT assertion latency under suite runtime conditions
- Key metrics: frameCount=0; sequenceCount=0; hpInToHpOutLatencyMs=55.0
- Assertion details:
  - [PASS] `hpInToHpOutLatencyMs` (timing) - expected range [0.0, 200.0], got 55.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 55.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 0, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 0, 'activityCompletedCount': 0}

### AO-LATENCY-FP-IN-TO-FP-OUT (ADDON-LATENCY)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172850_AO-LATENCY-FP-IN-TO-FP-OUT`
- Description: Measure FP_IN to FP_OUT latency with HP lead pre-satisfied
- Key metrics: frameCount=1; sequenceCount=1; hpInToHpOutLatencyMs=53.0; fpInToFpOutLatencyMs=83.0; fpPulseWidthMs=99.0; firstFrameGateDelayMs=83.0; firstFrameAfLeadMs=630.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `fpInToFpOutLatencyMs` (timing) - expected range [0.0, 200.0], got 83.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 53.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### AO-GAP-BOUNDARY-TRIAD (ADDON-GAP-BOUNDARY)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172900_AO-GAP-BOUNDARY-TRIAD`
- Description: Hermetic fullPressIgnoreGap boundary triad with explicit runtime parameters
- Notes: dual_classification_expected: gap_and_burst_ignored_fp_incremented
- Key metrics: frameCount=4; sequenceCount=2; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.0; frameStartSpacingMs=998.0; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0; hpOutContinuityMs=9988.0; hpHoldAfterLastFrameMs=4790.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.0
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 2, got 2
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.rejectedFpAtSequenceCapCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 2, got 2
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [0.0, 50.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 2, 'ignoredFpDuringGapCount': 2, 'ignoredFpDuringBurstCount': 1, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 2, 'activityCompletedCount': 1}

### AO-GAP-CADENCE (ADDON-GAP-CADENCE)

- Status: passed
- Failure class: pass
- Timing assertions: 3/3
- Functional assertions: 7/7
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172924_AO-GAP-CADENCE`
- Description: Hermetic 1 second inter-frame cadence check (1000/2000/3000/4000)
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=50.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=100.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=950.0; hpOutContinuityMs=9989.0; hpHoldAfterLastFrameMs=5942.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `telemetryDelta.ignoredFpDuringGapCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.ignoredFpDuringBurstCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.rejectedFpAtSequenceCapCount` (functional) - expected 0, got 0
  - [PASS] `telemetryDelta.sequenceCompletedCount` (functional) - expected 1, got 1
  - [PASS] `firstFrameGateDelayMs` (timing) - expected range [0.0, 50.0], got 0.0
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 50.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 1, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 1}

### AO-DEFERRED-CONFIG-WRITES (ADDON-DEFERRED-CONFIG)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_172946_AO-DEFERRED-CONFIG-WRITES`
- Description: Write config during active flow and verify defer/reject behavior
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=53.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.25; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=947.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 53.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

### AO-FACTORY-RESET-AND-COERCION (ADDON-FACTORY-RESET)

- Status: passed
- Failure class: pass
- Timing assertions: 2/2
- Functional assertions: 3/3
- Artifact path: `TickleBoard\artifacts\full_run_ble_20260521_1719\20260521_173002_AO-FACTORY-RESET-AND-COERCION`
- Description: Verify factory reset behavior and reserved field coercion
- Key metrics: frameCount=4; sequenceCount=1; hpInToHpOutLatencyMs=0.0; fpInToFpOutLatencyMs=0.0; fpPulseWidthMs=99.75; frameStartSpacingMs=998.6666666666666; firstFrameGateDelayMs=0.0; firstFrameAfLeadMs=1000.0
- Assertion details:
  - [PASS] `frameCount` (functional) - expected 4, got 4
  - [PASS] `frameStartSpacingMs` (timing) - expected 1000.0 +/- 10.0, got 998.7
  - [PASS] `telemetryDelta.acceptedFpCount` (functional) - expected 1, got 1
  - [PASS] `hpInToHpOutLatencyMs_non_negative` (timing) - expected >= 0 when measurable, got 0.0
  - [PASS] `telemetry_delta_sanity` (functional) - deltas={'wakeTimeoutCount': 0, 'acceptedFpCount': 1, 'ignoredFpDuringGapCount': 0, 'ignoredFpDuringBurstCount': 0, 'rejectedFpAtSequenceCapCount': 0, 'coldFpSequenceCount': 0, 'hpRefreshCount': 0, 'hpIgnoredDuringBurstCount': 0, 'fpDebounceRejectCount': 0, 'hpDebounceRejectCount': 0, 'sequenceCompletedCount': 1, 'activityCompletedCount': 0}

## Conclusion

All BLE-enabled full-suite cases passed with complete timing and functional assertion coverage.
