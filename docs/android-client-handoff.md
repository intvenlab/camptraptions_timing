# Android client BLE handoff

This document is the implementation handoff for Android client updates that consume the current firmware contract in `firmware/Camtraptions_Firmware.ino`.

## Scope

- No Android source changes are made in this repository.
- This document defines the expected BLE payload/field handling for Android-side implementation.

## GATT contract

Service UUID:

- `ca500000-0000-0000-0000-000000000000`

Characteristic UUIDs:

| Purpose | UUID |
|--------|------|
| Device name | `ca500001-0000-0000-0000-000000000000` |
| Group ID | `ca500002-0000-0000-0000-000000000000` |
| Group name | `ca500003-0000-0000-0000-000000000000` |
| Device type | `ca500004-0000-0000-0000-000000000000` |
| Chemistry | `ca500005-0000-0000-0000-000000000000` |
| Cell count | `ca500006-0000-0000-0000-000000000000` |
| Shutter count (read/notify) | `ca500007-0000-0000-0000-000000000000` |
| Reset shutter count (write `0x01`) | `ca500008-0000-0000-0000-000000000000` |
| Factory reset (write `0x01`) | `ca500009-0000-0000-0000-000000000000` |
| Camera config (read/write, 22 bytes) | `ca50000a-0000-0000-0000-000000000000` |
| Camera telemetry (read/notify) | `ca50000b-0000-0000-0000-000000000000` |
| Camera config write status (read/notify, 1 byte) | `ca50000e-0000-0000-0000-000000000000` |

## Camera config wire layout (22 bytes)

Firmware constants:

- `CAMERA_SETTINGS_VERSION = 3`
- Camera config payload length = 22 bytes

| Byte | Field | Encoding |
|------|-------|----------|
| 0 | `version` | Must be `3` |
| 1 | `enabled` | `0` pass-through, `1` state-machine |
| 2 | `wakeHalfPressHoldSec` | seconds |
| 3 | `minHalfPressBeforeShutter` | x100 ms |
| 4..5 | `shutterPulseDuration` | little-endian uint16, x10 ms (range 10..30000ms) |
| 6..7 | `startFrameSpacingTicks` | little-endian uint16, x10 ms (range 10..30000ms), pulse-end-to-next-start |
| 8 | `postShutterHpHoldTenths` | x100 ms |
| 9 | `hpDebounceMs` | ms |
| 10 | `fpDebounceMs` | ms |
| 11 | `frameCount` | count |
| 12 | `maxSequenceCount` | count |
| 13 | `wakeHoldRefreshPolicy` | Legacy encoding (`0/1/2`) retained for compatibility; current firmware behavior does not move wake deadline from HP refresh pulses |
| 14 | `halfPressDuringBurstPolicy` | currently only `0` supported |
| 15 | `fullPressWithoutHpPolicy` | `0 assertHpThenWait`, `1 ignoreFP` |
| 16 | `activityHalfPressHoldPolicy` | currently coerced to `0` |
| 17 | `fpAfterMaxSeqCountPolicy` | Legacy compatibility byte; cap handling is timeout-based in current firmware |
| 18 | `inputActivePolarity` | reserved, currently coerced to `0` |
| 19 | `outputDriveMode` | reserved, currently coerced to `0` |
| 20 | `powerSaveIdleMode` | `0` disabled, `1` enabled |
| 21 | `fullPressIgnoreGapTenths` | x100 ms (default `34` = 3.4 s) |

### Android write/readback rules

1. Always write the full 22-byte payload for camera config.
2. Read camera-config write status (`00e`) immediately after the write completes.
3. On `0x00` (applied), **wait at least 100 ms** before reading camera config (`00a`). Immediate reads can return stale pre-write bytes from the host BLE stack even though firmware has already updated the characteristic. If the first readback still does not match the written payload, wait another 100 ms and read `00a` again before treating the write as failed.
4. Read back camera config and update UI from readback values, not from requested values.
5. Treat `0xE1` / `0xE2` status codes as write rejection; do not treat the config as updated.
6. Validate the returned `version` and payload length before parsing policy bytes.

Camera-config write status codes (`00e`):

- `0x00` = ACK, applied immediately
- `0xE1` = NACK, payload format problem (length/version)
- `0xE2` = NACK, bad value range / unsupported reserved-field value
- `0xE3` = NACK, camera activity in progress (retry when idle)

## Telemetry characteristic (`00b`)

Telemetry payload is fixed-length binary with little-endian fields and `TELEMETRY_VERSION = 2`.

Client parsing requirements:

- Parse by `version` plus expected length.
- Reject unknown versions safely (do not reinterpret fields).
- Subscribe to notify and refresh live state from notifications.
- Keep before/after snapshots for automated validation workflows.

Live fields include:

- `cameraState`, flags (`activityActive`, `hpOutAsserted`)
- sequence/frame progress
- `lastEventCode`, `lastScenarioHint`
- `msUntilWakeDeadline`, `msUntilFpIgnoreClear`, `msUntilNextFrame`, `msUntilPostHoldEnd`
- trailing persisted counters block (including lifetime `bootCount`)

## Advertising manufacturer data

Company ID bytes are `0xFF, 0xFF` (placeholder).

Payload offsets after company ID:

| Offset | Field |
|--------|-------|
| 0 | Internal battery percent |
| 1..2 | Internal battery mV (LE) |
| 3 | External battery percent (`0xFF` when unavailable) |
| 4..5 | External battery mV (LE, `0xFFFF` when unavailable) |
| 6 | Device flags |
| 7 | Group ID |
| 8 | Cell count |
| 9..10 | Shutter count (LE uint16) |
| 11 | Beacon layout version (`BEACON_LAYOUT_VERSION`, current `1`) |
| 12 | `cameraState` snapshot |
| 13 | Runtime flags (`bit0 activityActive`, `bit1 hpOutAsserted`) |

### Backward compatibility policy

- Offsets `0..10` are legacy-compatible and remain stable.
- Offsets `11+` are extension bytes gated by beacon layout version.
- Android parser should:
  - parse legacy fields whenever at least 11 bytes are present,
  - parse extension fields only when length is sufficient,
  - ignore unknown trailing bytes.

## Factory reset expectations

Writing `0x01` to `ca500009...` now resets:

- device settings
- camera config
- telemetry counters/file
- active activity state (safe teardown)
- boot count is preserved across factory reset

Android UX should treat this as a full device reset and refresh all cached settings after reconnect.

## Known implementation caveats

- Reserved policy/mode fields exist in the wire format and are currently rejected with `0xE2` when set to unsupported values.
- UI should label reserved fields as unavailable or read-only until firmware enables runtime behavior.
- Telemetry counters may show intentional dual classification for a single rejected FP during burst (`ignoredFpDuringBurstCount` and `ignoredFpDuringGapCount` both increment).
