# TickleBoard Suite Report

- Cases: 43 passed, 0 failed, 0 skipped, 43 total
- Timing assertions: 115/115
- Functional assertions: 202/202

| Case | Scenario | Status | Failure Class | Timing | Functional | Skip Reason | Notes |
|------|----------|--------|---------------|--------|------------|-------------|-------|
| SC-01-NOMINAL | SC-01 | passed | pass | 6/6 | 7/7 |  |  |
| SC-02-FP-DURING-SEQUENCE | SC-02 | passed | pass | 2/2 | 5/5 |  | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-03-FP-FLOOD | SC-03 | passed | pass | 2/2 | 5/5 |  | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-04-WAKE-TIMEOUT | SC-04 | passed | pass | 2/2 | 3/3 |  |  |
| SC-04B-REPEATED-HP | SC-04b | passed | pass | 2/2 | 3/3 |  |  |
| SC-05-BACK-TO-BACK | SC-05 | passed | pass | 5/5 | 7/7 |  |  |
| SC-05B-FP-DURING-POST-HOLD | SC-05b | passed | pass | 5/5 | 7/7 |  |  |
| SC-06-COLD-FP | SC-06 | passed | pass | 2/2 | 3/3 |  |  |
| SC-07-HP-DURING-BURST | SC-07 | passed | pass | 2/2 | 3/3 |  |  |
| SC-07B-HP-DURING-POST-HOLD | SC-07b | passed | pass | 2/2 | 3/3 |  |  |
| SC-08-FP-BEFORE-HP | SC-08 | passed | pass | 3/3 | 3/3 |  |  |
| SC-09-SEQUENCE-CAP | SC-09 | passed | pass | 2/2 | 4/4 |  |  |
| SC-10-RECOVERY-AFTER-CAP | SC-10 | passed | pass | 3/3 | 7/7 |  |  |
| SC-11-SPACING-VS-T | SC-11 | passed | pass | 3/3 | 2/2 |  |  |
| SC-12-HP-ONLY-MIN-GAP | SC-12 | passed | pass | 2/2 | 3/3 |  |  |
| SC-13-BOUNCE-DEBOUNCE | SC-13 | passed | pass | 2/2 | 3/3 |  | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| SC-14-HELD-VS-PULSED-FP | SC-14 | passed | pass | 3/3 | 2/2 |  |  |
| SC-15-POWER-SAVE-BUDGET | SC-15 | passed | pass | 2/2 | 3/3 |  |  |
| SC-16-HP-RELEASE-AFTER-FP | SC-16 | passed | pass | 6/6 | 2/2 |  |  |
| SC-17-SHORT-HP-LEAD | SC-17 | passed | pass | 3/3 | 2/2 |  |  |
| SC-18-HP-CHATTER-BURST | SC-18 | passed | pass | 2/2 | 3/3 |  |  |
| SC-19-NEW-EVENT-AFTER-RELEASE | SC-19 | passed | pass | 4/4 | 7/7 |  |  |
| SC-20-T-GREATER-THAN-Y | SC-20 | passed | pass | 3/3 | 2/2 |  |  |
| AO-TELEMETRY-FIELD-COVERAGE | ADDON-TELEMETRY | passed | pass | 2/2 | 32/32 |  |  |
| AO-BLE-CONNECTED-SC04 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |  |
| AO-BLE-CONNECTED-SC01 | ADDON-BLE-CONNECTED | passed | pass | 2/2 | 3/3 |  |  |
| AO-LATENCY-HP-IN-TO-HP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 1/1 |  |  |
| AO-LATENCY-FP-IN-TO-FP-OUT | ADDON-LATENCY | passed | pass | 2/2 | 3/3 |  |  |
| AO-GAP-BOUNDARY-TRIAD | ADDON-GAP-BOUNDARY | passed | pass | 3/3 | 7/7 |  | dual_classification_expected: gap_and_burst_ignored_fp_incremented |
| AO-GAP-CADENCE | ADDON-GAP-CADENCE | passed | pass | 3/3 | 7/7 |  |  |
| AO-MAX-SEQUENCE-RANGE-64 | ADDON-MAX-SEQUENCE-RANGE | passed | pass | 2/2 | 7/7 |  |  |
| AO-DEFERRED-CONFIG-WRITES | ADDON-DEFERRED-CONFIG | passed | pass | 2/2 | 3/3 |  |  |
| AO-FACTORY-RESET-AND-COERCION | ADDON-FACTORY-RESET | passed | pass | 2/2 | 2/2 |  |  |
| AO-BOUNDS-COERCE-MIN | ADDON-BOUNDS-COERCE | passed | pass | 3/3 | 4/4 |  |  |
| AO-BOUNDS-COERCE-MAX | ADDON-BOUNDS-COERCE | passed | pass | 1/1 | 3/3 |  | telemetry_after_idle_timeout_state=3_flags=0x03 |
| AO-CAMCFG-NACK-BAD-VERSION | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |  |
| AO-CAMCFG-NACK-BAD-LENGTH | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |  |
| AO-CAMCFG-NACK-OUT-OF-RANGE | ADDON-CAMCFG-PROTOCOL | passed | pass | 0/0 | 1/1 |  |  |
| AO-HPRELAX-SIMULTANEOUS-INTENT | ADDON-HP-RELAX | passed | pass | 5/5 | 7/7 |  |  |
| AO-HPRELAX-ALLOWED-Z0 | ADDON-HP-RELAX | passed | pass | 4/4 | 7/7 |  |  |
| AO-HPRELAX-BLOCKED-Z500-T700 | ADDON-HP-RELAX | passed | pass | 4/4 | 7/7 |  |  |
| AO-HPRELAX-BOUNDARY-Z300-T700 | ADDON-HP-RELAX | passed | pass | 4/4 | 7/7 |  |  |
| SC-01-HPRELAX | SC-01 | passed | pass | 4/4 | 7/7 |  |  |
