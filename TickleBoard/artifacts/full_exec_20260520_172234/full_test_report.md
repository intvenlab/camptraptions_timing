# Full Validation Test Report

## Executive Summary

- Total tests: 28
- Passed: 26
- Failed: 2
- Framework errors: 0
- Average wall time per test: 37932.9 ms

| Case ID | Scenario | Status | Failure Class | Wall Time (ms) |
|---|---|---|---|---:|
| SC-01-NOMINAL | SC-01 | pass | pass | 32628 |
| SC-02-FP-DURING-SEQUENCE | SC-02 | pass | pass | 33641 |
| SC-03-FP-FLOOD | SC-03 | pass | pass | 36812 |
| SC-04-WAKE-TIMEOUT | SC-04 | pass | pass | 39609 |
| SC-04B-REPEATED-HP | SC-04b | pass | pass | 39643 |
| SC-05-BACK-TO-BACK | SC-05 | pass | pass | 43627 |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | fail | logic_mismatch | 38689 |
| SC-06-COLD-FP | SC-06 | pass | pass | 31849 |
| SC-07-HP-DURING-BURST | SC-07 | pass | pass | 36623 |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | pass | pass | 41679 |
| SC-08-FP-BEFORE-HP | SC-08 | pass | pass | 38641 |
| SC-09-SEQUENCE-CAP | SC-09 | fail | logic_mismatch | 41867 |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | pass | pass | 49492 |
| SC-11-SPACING-VS-T | SC-11 | pass | pass | 34454 |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | pass | pass | 36455 |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | pass | pass | 33933 |
| SC-14-HELD-VS-PULSED-FP | SC-14 | pass | pass | 40816 |
| SC-15-POWER-SAVE-BUDGET | SC-15 | pass | pass | 33541 |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | pass | pass | 35708 |
| SC-17-SHORT-HP-LEAD | SC-17 | pass | pass | 36733 |
| SC-18-HP-CHATTER-BURST | SC-18 | pass | pass | 39536 |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | pass | pass | 42620 |
| SC-20-T-GREATER-THAN-Y | SC-20 | pass | pass | 38503 |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | pass | pass | 41593 |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | pass | pass | 32670 |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | pass | pass | 40711 |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | pass | pass | 37637 |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | pass | pass | 32412 |

## Timing And Functional Metrics By Test

| Case ID | fpPulseWidthMs | frameStartSpacingMs | fpInToFpOutLatencyMs | hpInToHpOutLatencyMs | frameCount | sequenceCount | ignoredFpCount | Notes |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| SC-01-NOMINAL | 101.0 | 999.0 | 0.0 | 53.0 | 4 | 1 | 0 |  |
| SC-02-FP-DURING-SEQUENCE | 99.25 | 999.0 | 0.0 | 5.0 | 4 | 1 | 0 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-03-FP-FLOOD | 99.0 | 999.0 | 0.0 | 42.0 | 4 | 1 | 0 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-04-WAKE-TIMEOUT | None | None | None | 56.0 | 0 | 0 | 0 |  |
| SC-04B-REPEATED-HP | None | None | None | 50.0 | 0 | 0 | 0 |  |
| SC-05-BACK-TO-BACK | 99.875 | 1285.142857142857 | 1.0 | 49.0 | 8 | 1 | 0 |  |
| SC-05B-FP-DURING-POST-HOLD | 100.5 | 1171.0 | 0.0 | 48.0 | 8 | 1 | 0 |  |
| SC-06-COLD-FP | 98.75 | 998.6666666666666 | 496.0 | None | 4 | 1 | 0 |  |
| SC-07-HP-DURING-BURST | 99.0 | 999.0 | 0.0 | 50.0 | 4 | 1 | 0 |  |
| SC-07B-HP-DURING-POST-HOLD | 99.75 | 998.6666666666666 | 0.0 | 49.0 | 4 | 1 | 0 |  |
| SC-08-FP-BEFORE-HP | 100.0 | 999.0 | 491.0 | -150.0 | 4 | 1 | 0 |  |
| SC-09-SEQUENCE-CAP | 100.25 | 1833.0 | 0.0 | 49.0 | 4 | 1 | 0 |  |
| SC-10-RECOVERY-AFTER-CAP | 100.0 | 4166.666666666667 | 0.0 | 46.0 | 4 | 1 | 0 |  |
| SC-11-SPACING-VS-T | 106.75 | 999.0 | 450.0 | 51.0 | 4 | 1 | 0 |  |
| SC-12-HP-ONLY-MIN-GAP | None | None | None | 56.0 | 0 | 0 | 0 |  |
| SC-13-BOUNCE-DEBOUNCE | 101.0 | 999.0 | 0.0 | 12.0 | 4 | 1 | 0 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-14-HELD-VS-PULSED-FP | 101.0 | 999.0 | 0.0 | 10.0 | 4 | 1 | 0 |  |
| SC-15-POWER-SAVE-BUDGET | 100.25 | 998.6666666666666 | 0.0 | 54.0 | 4 | 1 | 0 |  |
| SC-16-HP-RELEASE-AFTER-FP | 89.75 | 998.6666666666666 | 462.0 | 11.0 | 4 | 1 | 0 |  |
| SC-17-SHORT-HP-LEAD | 100.0 | 999.0 | 1879.0 | 50.0 | 4 | 1 | 0 |  |
| SC-18-HP-CHATTER-BURST | 99.25 | 999.0 | 970.0 | 43.0 | 4 | 1 | 0 |  |
| SC-19-NEW-EVENT-AFTER-RELEASE | 100.0 | 1930.0 | 0.0 | 5.0 | 8 | 1 | 0 |  |
| SC-20-T-GREATER-THAN-Y | 99.25 | 199.66666666666666 | 1880.0 | 52.0 | 4 | 1 | 0 |  |
| AO-BLE-CONNECTED-SC04 | None | None | None | 57.0 | 0 | 0 | 0 |  |
| AO-BLE-CONNECTED-SC01 | 99.75 | 199.66666666666666 | 978.0 | 51.0 | 4 | 1 | 0 |  |
| AO-GAP-BOUNDARY-TRIAD | 99.0 | 200.0 | 972.0 | 40.0 | 2 | 1 | 2 |  |
| AO-DEFERRED-CONFIG-WRITES | 101.0 | 999.0 | 972.0 | 52.0 | 4 | 1 | 0 |  |
| AO-FACTORY-RESET-AND-COERCION | 99.25 | 999.0 | 984.0 | 53.0 | 4 | 1 | 0 |  |

## Failure Analysis

- timing_tolerance: 0
- logic_mismatch: 2
- framework_error: 0
