from __future__ import annotations

from dataclasses import dataclass
import struct

from .constants import TELEMETRY_PAYLOAD_LEN


COUNTER_NAMES = [
    "wakeTimeoutCount",
    "acceptedFpCount",
    "ignoredFpDuringGapCount",
    "ignoredFpDuringBurstCount",
    "MaxSequenceExceededCount",
    "coldFpSequenceCount",
    "hpRefreshCount",
    "hpIgnoredDuringBurstCount",
    "fpDebounceRejectCount",
    "hpDebounceRejectCount",
    "sequenceCompletedCount",
    "activityCompletedCount",
    "bootCount",
]

CAMERA_STATE_NAMES = {
    0: "CAM_IDLE",
    1: "CAM_WAKE_AF",
    2: "CAM_COLD_FP_WAIT",
    3: "CAM_BURST_ACTIVE",
    4: "CAM_POST_SHUTTER_EXT",
}

EVENT_CODE_NAMES = {
    0: "TEL_EVT_NONE",
    1: "TEL_EVT_HP_WAKE",
    2: "TEL_EVT_HP_REFRESH",
    3: "TEL_EVT_FP_ACCEPTED",
    4: "TEL_EVT_WAKE_TIMEOUT",
    5: "TEL_EVT_FP_REJECT_GAP",
    6: "TEL_EVT_FP_REJECT_CAP",
    7: "TEL_EVT_BURST_COMPLETE",
    8: "TEL_EVT_ACTIVITY_END",
    9: "TEL_EVT_COLD_FP",
    10: "TEL_EVT_HP_IGNORED_BURST",
    11: "TEL_EVT_FP_DEBOUNCE_REJECT",
    12: "TEL_EVT_HP_DEBOUNCE_REJECT",
    13: "TEL_EVT_FP_ACCEPTED_AT_GAP_BOUNDARY",
}

SCENARIO_HINT_NAMES = {
    0: "TEL_SC_NONE",
    1: "TEL_SC_WAKE_TIMEOUT",
    2: "TEL_SC_FP_GAP_IGNORE",
    3: "TEL_SC_COLD_FP",
    4: "TEL_SC_SEQUENCE_CAP",
    5: "TEL_SC_HP_DURING_BURST",
    6: "TEL_SC_DEBOUNCE",
}

BOOT_RESET_REASON_BITS = (
    (0x0001, "pin"),
    (0x0002, "watchdog"),
    (0x0004, "soft"),
    (0x0008, "lockup"),
    (0x0010, "off"),
    (0x0020, "lpcomp"),
    (0x0040, "debug"),
    (0x0080, "nfc"),
)


@dataclass
class TelemetrySnapshot:
    version: int
    camera_state: int
    flags: int
    frames_fired_this_sequence: int
    sequences_started_this_activity: int
    last_event_code: int
    last_scenario_hint: int
    ms_until_wake_deadline: int
    ms_until_fp_ignore_clear: int
    ms_until_next_frame: int
    ms_until_post_hold_end: int
    counters: dict[str, int]
    boot_reset_raw: int
    boot_temp_c_x100: int


def decode_boot_reset_raw(raw: int) -> list[str]:
    reasons = [name for bit, name in BOOT_RESET_REASON_BITS if (int(raw) & bit) != 0]
    if reasons:
        return reasons
    return ["power_on_or_unknown"]


def decode_boot_temp_c_x100(temp_c_x100: int) -> tuple[float, float]:
    temp_c = float(int(temp_c_x100)) / 100.0
    temp_f = (temp_c * 9.0 / 5.0) + 32.0
    return temp_c, temp_f


def parse_payload(payload: bytes) -> TelemetrySnapshot:
    if len(payload) < TELEMETRY_PAYLOAD_LEN:
        raise ValueError(f"telemetry payload too short: {len(payload)} bytes")
    version = payload[0]
    camera_state = payload[1]
    flags = payload[2]
    frames = payload[3]
    sequences = payload[4]
    evt = payload[5]
    hint = payload[6]
    ms_wake, ms_gap, ms_next, ms_hold = struct.unpack_from("<IIII", payload, 8)
    counters_struct_version = int(payload[24])
    counters_raw = struct.unpack_from("<13I", payload, 28)
    boot_reset_raw, boot_temp_c_x100 = struct.unpack_from("<Hh", payload, 80)
    counters = {name: int(v) for name, v in zip(COUNTER_NAMES, counters_raw)}
    counters["version"] = counters_struct_version
    return TelemetrySnapshot(
        version=version,
        camera_state=camera_state,
        flags=flags,
        frames_fired_this_sequence=frames,
        sequences_started_this_activity=sequences,
        last_event_code=evt,
        last_scenario_hint=hint,
        ms_until_wake_deadline=ms_wake,
        ms_until_fp_ignore_clear=ms_gap,
        ms_until_next_frame=ms_next,
        ms_until_post_hold_end=ms_hold,
        counters=counters,
        boot_reset_raw=int(boot_reset_raw),
        boot_temp_c_x100=int(boot_temp_c_x100),
    )


def diff_counters(before: TelemetrySnapshot | None, after: TelemetrySnapshot | None) -> dict[str, int]:
    if before is None or after is None:
        return {}
    out: dict[str, int] = {}
    for name in COUNTER_NAMES:
        out[name] = int(after.counters.get(name, 0)) - int(before.counters.get(name, 0))
    return out


def validate_delta_rules(deltas: dict[str, int]) -> list[str]:
    notes: list[str] = []
    gap = deltas.get("ignoredFpDuringGapCount", 0)
    burst = deltas.get("ignoredFpDuringBurstCount", 0)
    if gap > 0 and burst > 0:
        notes.append("dual_classification_expected: gap_and_burst_ignored_fp_incremented")
    return notes


def _state_name(code: int) -> str:
    return CAMERA_STATE_NAMES.get(int(code), f"UNKNOWN_STATE_{int(code)}")


def _event_name(code: int) -> str:
    return EVENT_CODE_NAMES.get(int(code), f"UNKNOWN_EVENT_{int(code)}")


def _hint_name(code: int) -> str:
    return SCENARIO_HINT_NAMES.get(int(code), f"UNKNOWN_HINT_{int(code)}")


def format_snapshot(snapshot: TelemetrySnapshot) -> str:
    lines: list[str] = []
    lines.append("Telemetry Characteristic")
    lines.append("----------------------")
    lines.append(f"Version: {snapshot.version}")
    lines.append(f"Camera State: {_state_name(snapshot.camera_state)} ({snapshot.camera_state})")
    lines.append(f"Last Event: {_event_name(snapshot.last_event_code)} ({snapshot.last_event_code})")
    lines.append(f"Scenario Hint: {_hint_name(snapshot.last_scenario_hint)} ({snapshot.last_scenario_hint})")
    lines.append("Flags:")
    lines.append(f"  activityActive: {bool(snapshot.flags & 0x01)}")
    lines.append(f"  hpOutAsserted:  {bool(snapshot.flags & 0x02)}")
    lines.append(f"  raw:            0x{snapshot.flags:02X}")
    lines.append("Live Fields:")
    lines.append(f"  framesFiredThisSequence:      {snapshot.frames_fired_this_sequence}")
    lines.append(f"  sequencesStartedThisActivity: {snapshot.sequences_started_this_activity}")
    lines.append(f"  msUntilWakeDeadline:          {snapshot.ms_until_wake_deadline}")
    lines.append(f"  msUntilFpIgnoreClear:         {snapshot.ms_until_fp_ignore_clear}")
    lines.append(f"  msUntilNextFrame:             {snapshot.ms_until_next_frame}")
    lines.append(f"  msUntilPostHoldEnd:           {snapshot.ms_until_post_hold_end}")
    lines.append("Counters:")
    for name in COUNTER_NAMES:
        lines.append(f"  {name:30} {int(snapshot.counters.get(name, 0))}")
    lines.append(f"  {'version':30} {int(snapshot.counters.get('version', 0))}")
    reset_reasons = "|".join(decode_boot_reset_raw(snapshot.boot_reset_raw))
    temp_c, temp_f = decode_boot_temp_c_x100(snapshot.boot_temp_c_x100)
    lines.append("Boot Snapshot:")
    lines.append(f"  bootResetRaw:                  0x{int(snapshot.boot_reset_raw):04X}")
    lines.append(f"  bootResetReason:               {reset_reasons}")
    lines.append(f"  bootTempCx100:                 {int(snapshot.boot_temp_c_x100)}")
    lines.append(f"  bootTempC:                     {temp_c:.2f}")
    lines.append(f"  bootTempF:                     {temp_f:.2f}")
    return "\n".join(lines)

