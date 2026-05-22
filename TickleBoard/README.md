# TickleBoard

Validation fixture workspace for the timing-board test system.

This folder contains:

- `TickleBoardBits/` - Arduino Uno fixture firmware
- `scripts/` - Windows-side orchestration scripts (serial + BLE/GATT workflow)
- `vectors/` - Test vectors used by the orchestrator

The design follows `docs/validation-test-plan.md`:

- line-oriented fixture protocol (`ID?`, `MAP`, `ARM`, `PULSE`, `LEVEL`, `RUN`, `DUMP`, `RESET`)
- fixture-driven edge timestamp collection
- Windows client control for BLE/GATT parameter read/write and test execution

## Quick Start

1. Upload `TickleBoard/TickleBoardBits/TickleBoardBits.ino` to Arduino Uno.
2. Install Python deps from `TickleBoard/scripts/requirements.txt`.
3. Run preflight:
   - `python TickleBoard/scripts/tickle_cli.py preflight --port COM7`
4. Run golden subset:
   - `python TickleBoard/scripts/tickle_cli.py run-suite TickleBoard/vectors/suites/golden_subset_suite.yaml --port COM7 --ble <dut_ble_address>`
5. Run full suite:
   - `python TickleBoard/scripts/tickle_cli.py run-suite TickleBoard/vectors/suites/full_validation_suite.yaml --port COM7 --ble <dut_ble_address>`

## Coverage Implemented

- scenario vectors for SC-01 through SC-20
- mandatory add-on vectors:
  - BLE-connected runtime checks
  - `fullPressIgnoreGap` boundary triad
  - `fullPressIgnoreGap` cadence sanity (`1000/2000/3000/4000 ms` with `StartFrameSpacingMin=1.0s`)
  - deferred config write case
  - factory reset and reserved-field coercion case
- suite files:
  - `full_validation_suite.yaml`
  - `golden_subset_suite.yaml`
  - `mandatory_addons_suite.yaml`
  - `parameter_interaction_suite.yaml`

## Artifacts

Run outputs are written to `TickleBoard/artifacts/`:

- per-case run folder with `result.json` and raw edge log
- suite rollup files: JSON, markdown, and CSV
- preflight and discovery outputs via CLI JSON/text
