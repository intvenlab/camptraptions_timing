# TickleBoard Scripts

`tickle_cli.py` is the orchestration entry point for fixture + DUT test execution.

## Commands

- `ports` - list serial ports
- `discover` - discover DUT BLE devices
- `preflight` - validate fixture and optional BLE readability
- `run-case <vector>` - run one case file
- `run-suite <suite>` - run a suite file and produce rollup outputs
- `run-sc15 <vector>` - repeated enabled/disabled latency budget check
- `ble-smoke` - discover + read/write/readback/restore BLE parameter smoke test
- `ble-telemetry-smoke` - read beacon + telemetry payload and print decoded runtime details
- `gen-parameter-sweep` - generate a comprehensive parameter-sweep vector pack + suite
- `report <rollup.json>` - render markdown/csv report from rollup JSON

## Python modules (`tickleboard/`)

- `fixture_client.py` - serial protocol handling with robust `DUMP` parsing
- `ble_adapter.py` - BLE/GATT discovery and camera config read/write/readback
- `camera_config.py` - doc-parameter to camera-byte mapping and clamping
- `telemetry.py` - telemetry payload parsing and delta checks
- `metrics.py` - edge-log metric extraction and SC-15 sample aggregation
- `evaluator.py` - tolerance and pass/fail checks
- `runner.py` - end-to-end case execution
- `reporting.py` - suite markdown/csv reporting
- `artifacts.py` - artifact path and JSON writing helpers

## Outputs

By default artifacts are written under `TickleBoard/artifacts/`:

- per-case `result.json`
- per-case `raw_edges.log`
- suite rollups in `.json`, `.md`, `.csv`
- generated sweep vectors under `TickleBoard/vectors/generated/parameter_sweep/`

### Parameter sweep generator

- Build sweep vectors/suite:
  - `python TickleBoard/scripts/tickle_cli.py gen-parameter-sweep`
- Generated suite path:
  - `TickleBoard/vectors/suites/parameter_sweep_suite.yaml`
- Coverage:
  - timing grid (`StartFrameSpacingMin` x `shutterPulseDuration` x `FrameCount`)
  - cap/gap interaction grid (`MaxSequenceCount` x `fullPressIgnoreGap`)

### No-BLE behavior

- If `--ble` is omitted, vectors that require BLE are marked as `skipped` with a skip reason.
- BLE-required classification uses:
  - vector-level `requiresBle: true` / `requires_ble: true`,
  - `ble-connected` tag,
  - non-default `parameters` that require runtime camera config writes.
- When telemetry is unavailable (no BLE), `expect.telemetryDeltas` assertions are skipped and noted in case output.
