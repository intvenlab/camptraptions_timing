# TickleBoard Suite Report

- Timing assertions: 74/79
- Functional assertions: 95/97

| Case | Scenario | Passed | Failure Class | Timing | Functional | Notes |
|------|----------|--------|---------------|--------|------------|-------|
| SC-01-NOMINAL | SC-01 | True | pass | 6/6 | 5/5 |  |
| SC-02-FP-DURING-SEQUENCE | SC-02 | True | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-03-FP-FLOOD | SC-03 | True | pass | 2/2 | 5/5 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-04-WAKE-TIMEOUT | SC-04 | True | pass | 2/2 | 3/3 |  |
| SC-04B-REPEATED-HP | SC-04b | True | pass | 2/2 | 3/3 |  |
| SC-05-BACK-TO-BACK | SC-05 | True | pass | 5/5 | 5/5 |  |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | False | timing_tolerance | 4/5 | 4/5 |  |
| SC-06-COLD-FP | SC-06 | True | pass | 2/2 | 3/3 |  |
| SC-07-HP-DURING-BURST | SC-07 | True | pass | 2/2 | 3/3 |  |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | True | pass | 2/2 | 3/3 |  |
| SC-08-FP-BEFORE-HP | SC-08 | True | pass | 3/3 | 3/3 |  |
| SC-09-SEQUENCE-CAP | SC-09 | True | pass | 2/2 | 4/4 |  |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | False | timing_tolerance | 2/3 | 5/5 |  |
| SC-11-SPACING-VS-T | SC-11 | True | pass | 3/3 | 2/2 |  |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | True | pass | 2/2 | 3/3 |  |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | False | logic_mismatch | 2/2 | 3/4 | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-14-HELD-VS-PULSED-FP | SC-14 | True | pass | 3/3 | 2/2 |  |
| SC-15-POWER-SAVE-BUDGET | SC-15 | True | pass | 2/2 | 3/3 |  |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | False | timing_tolerance | 5/6 | 2/2 |  |
| SC-17-SHORT-HP-LEAD | SC-17 | True | pass | 3/3 | 2/2 |  |
| SC-18-HP-CHATTER-BURST | SC-18 | True | pass | 2/2 | 3/3 |  |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | False | timing_tolerance | 3/4 | 5/5 |  |
| SC-20-T-GREATER-THAN-Y | SC-20 | True | pass | 3/3 | 2/2 |  |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | True | pass | 2/2 | 3/3 |  |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | False | timing_tolerance | 1/2 | 3/3 |  |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | True | pass | 3/3 | 5/5 |  |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | True | pass | 2/2 | 3/3 |  |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | True | pass | 2/2 | 3/3 |  |
