# Telemetry

Telemetry records what the timing MCU actually did in the field. It is separate from configuration: `CameraConfig` remains the user-facing timing setup, while telemetry is runtime state plus counters for diagnostics, validation, and app display.

## Storage policy

The firmware keeps live telemetry in RAM. Values are marked dirty only when a persisted counter actually changes. Repeated BLE publication, live-only state changes, and unchanged values do not mark telemetry dirty.

Interrupt handlers must never write flash. They may update in-memory counters or set dirty/notify flags, and normal loop/state-machine code flushes snapshots later. Persisted telemetry is stored separately from `DeviceConfig` and `CameraConfig`, so high-churn diagnostics do not rewrite user settings.

Flash snapshots are rate-limited. The default policy is:

1. Keep current state, deadlines, current sequence counts, and last event in RAM only.
2. Persist coarse lifetime counters to a telemetry file in `InternalFS` / LittleFS.
3. Flush at controlled boundaries such as activity end, BLE disconnect, a periodic dirty check, or an explicit app/test-client request.
4. Design for fewer than roughly 100,000 telemetry flash snapshots over board life.

LittleFS provides wear leveling, but the firmware still treats flash as a snapshot store rather than the primary telemetry engine.

## Published BLE payload

The camera firmware publishes a read/notify telemetry characteristic:

| Characteristic | UUID suffix | Access | Purpose |
|----------------|-------------|--------|---------|
| Camera telemetry | `00b` | Read + Notify | Live state, timing windows, last event, and persisted counters |

The payload is a fixed little-endian binary struct with a schema version byte. Android or validation clients should parse by version, not by inferred length alone.

### Parser compatibility contract

- Parse telemetry by `(version, expectedLength)`; reject or ignore unsupported versions.
- Current telemetry payload is `TELEMETRY_VERSION = 1` and fixed to `sizeof(CameraTelemetryPayload)`.
- Treat added trailing fields in future versions as opt-in via version bump, not as implicit extensions.
- Validation/client logs should capture payload version and observed payload length for troubleshooting.

### Live state fields

| Field | Meaning |
|-------|---------|
| `version` | Telemetry payload version |
| `cameraState` | Current firmware state: idle, wake/AF, cold-FP wait, burst active, or post-shutter hold |
| `flags` | Bit flags for active activity and HP OUT asserted |
| `framesFiredThisSequence` | Frames fired in the current sequence |
| `sequencesStartedThisActivity` | Accepted sequences in the current activity |
| `lastEventCode` | Last meaningful state/counter event |
| `lastScenarioHint` | Broad SC-xx family hint for UI/logging |
| `msUntilWakeDeadline` | Remaining wake timeout window, or zero |
| `msUntilFpIgnoreClear` | Remaining `fullPressIgnoreGap`, or zero |
| `msUntilNextFrame` | Remaining time until next scheduled FP OUT start, or zero |
| `msUntilPostHoldEnd` | Remaining post-shutter HP hold, or zero |

### Persisted counters

| Counter | Meaning | Primary scenarios |
|---------|---------|-------------------|
| `wakeTimeoutCount` | HP asserted but no accepted FP before timeout | SC-04, SC-04b, SC-12 |
| `acceptedFpCount` | FP inputs accepted as sequence starts | SC-01, SC-05, SC-05b, SC-06, SC-08 |
| `ignoredFpDuringGapCount` | FP inputs rejected by `fullPressIgnoreGap` / R10 | SC-02, SC-03, SC-14 |
| `ignoredFpDuringBurstCount` | FP input observed while burst scheduling is active | SC-02, SC-03 |
| `rejectedFpAtSequenceCapCount` | FP rejected because `MaxSequenceCount` is reached | SC-09, SC-10 |
| `coldFpSequenceCount` | FP-before-HP events that become accepted sequences | SC-06, SC-08 |
| `hpRefreshCount` | Repeated HP pulses while wake/activity logic is active | SC-04b, SC-07b, SC-12 |
| `hpIgnoredDuringBurstCount` | HP pulses during burst that do not alter scheduling | SC-07, SC-18 |
| `fpDebounceRejectCount` | FP edges rejected by debounce | SC-13 |
| `hpDebounceRejectCount` | Legacy HP debounce reject counter (no longer used as a validation gate) | Informational only |
| `sequenceCompletedCount` | Completed burst sequences | SC-01, SC-05, SC-05b |
| `activityCompletedCount` | Completed MCU activities, including wake-only timeouts | SC-01, SC-04, SC-05 |

When an FP arrives during an active burst and still inside `fullPressIgnoreGap`, firmware intentionally increments both `ignoredFpDuringBurstCount` and `ignoredFpDuringGapCount`. Treat that pair as one rejected input viewed from two dimensions (burst-state + gap-rule), not as duplicate acceptance.

## Advertising payload telemetry

Manufacturer data now carries a versioned beacon suffix so clients can read basic runtime status without opening a full connection.

| Byte offset (after company ID) | Field |
|--------------------------------|-------|
| `0..10` | Legacy battery/config/shutter fields (unchanged layout) |
| `11` | Beacon layout version (`BEACON_LAYOUT_VERSION`, current `1`) |
| `12` | `cameraState` snapshot |
| `13` | Runtime flags (`bit0=activityActive`, `bit1=hpOutAsserted`) |

Compatibility rule: parsers must keep legacy offsets stable and treat bytes beyond `10` as optional extension fields gated by beacon layout version.

## Event and scenario hints

Firmware counters are named by rule/reason. Scenario hints are deliberately broad because the device cannot always know which validation case the operator intended.

| Event code | Typical meaning |
|------------|-----------------|
| `HP_WAKE` | HP input woke the camera path |
| `HP_REFRESH` | HP input refreshed or touched the wake/activity path |
| `FP_ACCEPTED` | FP accepted and a sequence started |
| `WAKE_TIMEOUT` | HP wake expired without accepted FP |
| `FP_REJECT_GAP` | FP rejected by ignore-gap/burst rules |
| `FP_REJECT_CAP` | FP rejected at sequence cap |
| `BURST_COMPLETE` | Current sequence finished all frames |
| `ACTIVITY_END` | Activity ended and HP/FP outputs returned idle |
| `COLD_FP` | FP arrived before prior HP wake |
| `HP_IGNORED_BURST` | HP input during burst did not alter scheduling |
| `FP_DEBOUNCE_REJECT` | FP edge rejected by debounce |
| `HP_DEBOUNCE_REJECT` | HP edge rejected by debounce |
| `FP_ACCEPTED_AT_GAP_BOUNDARY` | FP accepted exactly when `fullPressIgnoreGap` cleared (`now == fullPressIgnoreUntilMs`) |

| Scenario hint | Maps to |
|---------------|---------|
| `WAKE_TIMEOUT` | SC-04, SC-04b, SC-12 |
| `FP_GAP_IGNORE` | SC-02, SC-03, SC-14 |
| `COLD_FP` | SC-06, SC-08 |
| `SEQUENCE_CAP` | SC-09, SC-10 |
| `HP_DURING_BURST` | SC-07, SC-18 |
| `DEBOUNCE` | SC-13 |

## Validation use

The fixture remains the timing source of truth for edge latency and pulse width metrics. Telemetry complements fixture logs by showing internal decisions: accepted vs ignored FP, wake-only timeouts, sequence caps, debounce rejects, and current state transitions.

For each validation case, capture a telemetry snapshot before and after the stimulus. Compare counter deltas with fixture-derived metrics such as `sequenceCount`, `frameCount`, and `ignoredFpCount`.

For repeatable validation, capture snapshots at consistent boundaries:

1. Pre-case baseline (before first stimulus edge)
2. Post-case steady point (after outputs settle)
3. Optional forced-flush point if persistence needs verification
4. Post-disconnect snapshot when BLE disconnect-triggered flush behavior is under test
