# TickleBoard Stabilization Notes

This note captures the Phase 6 bench-validation guidance.

## Golden subset

Run first:

- `SC-01`
- `SC-04`
- `SC-06`
- `SC-11`
- `SC-15`

Use:

`python TickleBoard/scripts/tickle_cli.py run-suite TickleBoard/vectors/suites/golden_subset_suite.yaml --port COM7 --ble <address>`

## Uno timing/jitter handling

- Collect at least 20 repeated samples for each SC-15 path.
- Compare enabled/disabled distributions (min/mean/max/p95/p99).
- If SC-15 delta is within 0.2 ms of the 1.0 ms limit, mark as `needs_logic_analyzer_confirmation`.

## Result tagging convention

Store these tags in each run result where relevant:

- `fixture_edge_overflow`
- `fixture_malformed_lines`
- `telemetry_dual_classification_expected`
- `needs_logic_analyzer_confirmation`

