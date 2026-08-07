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
  - `fullPressIgnoreGap` cadence sanity (`1000/2100/3200/4300 ms` with `StartFrameSpacingMin=1.0s`, `shutterPulseDuration=100ms`)
  - deferred config write case
  - factory reset and reserved-field coercion case
  - generated parameter sweep pack for timing + cap/gap interactions
- suite files:
  - `full_validation_suite.yaml`
  - `golden_subset_suite.yaml`
  - `mandatory_addons_suite.yaml`
  - `parameter_interaction_suite.yaml`
  - `parameter_sweep_suite.yaml` (generated)

## Parameter Sweep

- Generate sweep vectors/suite:
  - `python TickleBoard/scripts/tickle_cli.py gen-parameter-sweep`
- Run generated sweep:
  - `python TickleBoard/scripts/tickle_cli.py run-suite TickleBoard/vectors/suites/parameter_sweep_suite.yaml --port COM7 --ble <dut_ble_address>`
- The generated suite currently produces 47 BLE-required cases covering:
  - `StartFrameSpacingMin` x `shutterPulseDuration` x `FrameCount`
  - `MaxSequenceCount` x `fullPressIgnoreGap`

## Artifacts

Run outputs are written to `TickleBoard/artifacts/`:

- per-case run folder with `result.json` and raw edge log
- suite rollup files: JSON, markdown, and CSV
- preflight and discovery outputs via CLI JSON/text

## Latest authoritative run

- BLE full-suite artifacts: per-case folders under `TickleBoard/artifacts/` with batch prefix `20260523_124553_*`
- Current top-level rollups (`TickleBoard/artifacts/suite_rollup.json`, `.md`, `.csv`) mirror the **2026-08-06** BLE full suite (HP-relax): **43/43 pass** (timing 115/115, functional 202/202, protocol 24/24)
- Customer-facing detailed report: `docs/validation-test-report.md` (generated via `tickle_cli.py report --detailed`)
- Archived prior run (31-case generation): `TickleBoard/artifacts/full_run_ble_20260521_1719`
