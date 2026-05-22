# TickleBoard Suite Report

- Cases: 19 passed, 2 failed, 10 skipped, 31 total
- Timing assertions: 58/62
- Functional assertions: 24/25

| Case | Scenario | Status | Failure Class | Timing | Functional | Skip Reason | Notes |
|------|----------|--------|---------------|--------|------------|-------------|-------|
| SC-01-NOMINAL | SC-01 | passed | pass | 6/6 | 2/2 |  | telemetry_assertions_skipped_no_ble |
| SC-02-FP-DURING-SEQUENCE | SC-02 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-03-FP-FLOOD | SC-03 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-04-WAKE-TIMEOUT | SC-04 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-04B-REPEATED-HP | SC-04b | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-05-BACK-TO-BACK | SC-05 | passed | pass | 5/5 | 2/2 |  | telemetry_assertions_skipped_no_ble |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | passed | pass | 5/5 | 2/2 |  | telemetry_assertions_skipped_no_ble |
| SC-06-COLD-FP | SC-06 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-07-HP-DURING-BURST | SC-07 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-08-FP-BEFORE-HP | SC-08 | passed | pass | 3/3 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-09-SEQUENCE-CAP | SC-09 | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: FrameCount=2 (factory=4), MaxSequenceCount=1 (factory=4) | skipped_no_ble; requires non-default camera params: FrameCount=2 (factory=4), MaxSequenceCount=1 (factory=4) |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: FrameCount=2 (factory=4), MaxSequenceCount=1 (factory=4) | skipped_no_ble; requires non-default camera params: FrameCount=2 (factory=4), MaxSequenceCount=1 (factory=4) |
| SC-11-SPACING-VS-T | SC-11 | failed | timing_tolerance | 1/3 | 0/1 |  |  |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-14-HELD-VS-PULSED-FP | SC-14 | passed | pass | 3/3 | 1/1 |  |  |
| SC-15-POWER-SAVE-BUDGET | SC-15 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | failed | timing_tolerance | 4/6 | 1/1 |  |  |
| SC-17-SHORT-HP-LEAD | SC-17 | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: minHalfPressBeforeShutter=2.0s (factory=0.5s) | skipped_no_ble; requires non-default camera params: minHalfPressBeforeShutter=2.0s (factory=0.5s) |
| SC-18-HP-CHATTER-BURST | SC-18 | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | passed | pass | 4/4 | 2/2 |  | telemetry_assertions_skipped_no_ble |
| SC-20-T-GREATER-THAN-Y | SC-20 | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: minHalfPressBeforeShutter=2.0s (factory=0.5s), StartFrameSpacingMin=0.2s (factory=1.0s) | skipped_no_ble; requires non-default camera params: minHalfPressBeforeShutter=2.0s (factory=0.5s), StartFrameSpacingMin=0.2s (factory=1.0s) |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | skipped | skipped | 0/0 | 0/0 | ble-connected scenario | skipped_no_ble; ble-connected scenario |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | skipped | skipped | 0/0 | 0/0 | ble-connected scenario | skipped_no_ble; ble-connected scenario |
| AO-LATENCY-HP-IN-TO-HP-OUT | ADDON-LATENCY | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: FrameCount=1 (factory=4), wakeHalfPressHoldTime=3s (factory=10s) | skipped_no_ble; requires non-default camera params: FrameCount=1 (factory=4), wakeHalfPressHoldTime=3s (factory=10s) |
| AO-LATENCY-FP-IN-TO-FP-OUT | ADDON-LATENCY | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: FrameCount=1 (factory=4), minHalfPressBeforeShutter=0.2s (factory=0.5s) | skipped_no_ble; requires non-default camera params: FrameCount=1 (factory=4), minHalfPressBeforeShutter=0.2s (factory=0.5s) |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: FrameCount=2 (factory=4) | skipped_no_ble; requires non-default camera params: FrameCount=2 (factory=4) |
| AO-GAP-CADENCE | ADDON-GAP-CADENCE | passed | pass | 3/3 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | passed | pass | 2/2 | 1/1 |  | telemetry_assertions_skipped_no_ble |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | skipped | skipped | 0/0 | 0/0 | requires non-default camera params: inputActivePolarity=1 (factory=0), outputDriveMode=1 (factory=0), activityHalfPressHoldPolicy=1 (factory=0) | skipped_no_ble; requires non-default camera params: inputActivePolarity=1 (factory=0), outputDriveMode=1 (factory=0), activityHalfPressHoldPolicy=1 (factory=0) |
