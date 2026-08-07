from __future__ import annotations

from statistics import mean
from typing import Any

from .fixture_client import EdgeRecord
from .debug_log import debug_log


def _active_times(edges: list[EdgeRecord], signal: str) -> list[int]:
    return [e.t_ms for e in edges if e.signal == signal and e.state == "ACTIVE"]


def _inactive_times(edges: list[EdgeRecord], signal: str) -> list[int]:
    return [e.t_ms for e in edges if e.signal == signal and e.state == "INACTIVE"]


def _latency(first_a: list[int], first_b: list[int]) -> float | None:
    if not first_a or not first_b:
        return None
    return float(first_b[0] - first_a[0])


def _first_at_or_after(times: list[int], target: int) -> int | None:
    for t in times:
        if t >= target:
            return t
    return None


def _first_between(times: list[int], start: int, end_exclusive: int) -> int | None:
    for t in times:
        if t < start:
            continue
        if t >= end_exclusive:
            break
        return t
    return None


def _pair_widths(active: list[int], inactive: list[int]) -> list[int]:
    widths: list[int] = []
    j = 0
    for a in active:
        while j < len(inactive) and inactive[j] < a:
            j += 1
        if j < len(inactive):
            widths.append(inactive[j] - a)
    return widths


def _spacings(times: list[int]) -> list[int]:
    return [times[i] - times[i - 1] for i in range(1, len(times))]


def _end_to_start_spacings(
    fp_out_active: list[int],
    fp_out_inactive: list[int],
    start_spacings: list[int],
    threshold: int,
) -> list[int]:
    gaps: list[int] = []
    pair_count = min(len(fp_out_inactive), max(0, len(fp_out_active) - 1))
    for i in range(pair_count):
        if i >= len(start_spacings):
            break
        if start_spacings[i] >= threshold:
            continue
        gaps.append(fp_out_active[i + 1] - fp_out_inactive[i])
    return gaps


def _sequence_gap_threshold(spacings: list[int]) -> int:
    if not spacings:
        return 500
    baseline = min(spacings)
    # Inter-sequence gaps are expected to be materially larger than in-sequence cadence.
    return max(400, int(baseline * 1.8))


def _sequence_starts(fp_out_active: list[int]) -> list[int]:
    if not fp_out_active:
        return []
    starts = [fp_out_active[0]]
    spacings = _spacings(fp_out_active)
    if not spacings:
        return starts
    threshold = _sequence_gap_threshold(spacings)
    for idx, spacing in enumerate(spacings, start=1):
        if spacing >= threshold:
            starts.append(fp_out_active[idx])
    return starts


def _sequence_starts_from_hints(fp_out_active: list[int], hints: list[int]) -> list[int]:
    if not fp_out_active or not hints:
        return []
    starts: list[int] = []
    edge_idx = 0
    for hint in sorted(int(t) for t in hints):
        while edge_idx < len(fp_out_active) and fp_out_active[edge_idx] < hint:
            edge_idx += 1
        if edge_idx >= len(fp_out_active):
            break
        candidate = fp_out_active[edge_idx]
        if not starts or candidate != starts[-1]:
            starts.append(candidate)
    return starts


def _select_sequence_starts(
    fp_out_active: list[int],
    *,
    fp_sequence_start_hints: list[int] | None,
    expected_sequence_count: int | None,
) -> tuple[list[int], str]:
    heuristic_starts = _sequence_starts(fp_out_active)
    hinted_starts = _sequence_starts_from_hints(fp_out_active, fp_sequence_start_hints or [])
    if len(hinted_starts) >= 2:
        selected = hinted_starts
        source = "hinted"
    elif len(heuristic_starts) >= 2:
        selected = heuristic_starts
        source = "heuristic"
    elif hinted_starts:
        selected = hinted_starts
        source = "hinted_single"
    else:
        selected = heuristic_starts
        source = "heuristic_single" if heuristic_starts else "none"

    expected = int(expected_sequence_count) if expected_sequence_count is not None else None
    if expected is not None and expected >= 0 and len(selected) != expected:
        source = f"{source}_expected_{expected}_got_{len(selected)}"
    return selected, source


def _first_signal_time(*series: list[int]) -> int | None:
    vals = [v for arr in series for v in arr]
    if not vals:
        return None
    return min(vals)


def extract_metrics(
    edges: list[EdgeRecord],
    run_id: str = "unknown",
    *,
    fp_sequence_start_hints: list[int] | None = None,
    expected_sequence_count: int | None = None,
) -> dict[str, float | int | bool | str | None]:
    hp_in_a = _active_times(edges, "HP_IN")
    fp_in_a = _active_times(edges, "FP_IN")
    hp_out_a = _active_times(edges, "HP_OUT")
    hp_out_i = _inactive_times(edges, "HP_OUT")
    fp_out_a = _active_times(edges, "FP_OUT")
    fp_out_i = _inactive_times(edges, "FP_OUT")

    fp_widths = _pair_widths(fp_out_a, fp_out_i)
    fp_spacings_all = _spacings(fp_out_a)
    seq_gap_threshold = _sequence_gap_threshold(fp_spacings_all)
    fp_spacings = [s for s in fp_spacings_all if s < seq_gap_threshold]
    fp_end_to_start_spacings = _end_to_start_spacings(fp_out_a, fp_out_i, fp_spacings_all, seq_gap_threshold)
    seq_starts, seq_start_source = _select_sequence_starts(
        fp_out_a,
        fp_sequence_start_hints=fp_sequence_start_hints,
        expected_sequence_count=expected_sequence_count,
    )
    # region agent log
    debug_log(
        run_id=run_id,
        hypothesis_id="H2_pairing_or_capture_artifact",
        location="metrics.py:49",
        message="FP_OUT edge pairing snapshot",
        data={
            "fpOutActive": fp_out_a,
            "fpOutInactive": fp_out_i,
            "pairedWidthsMs": fp_widths,
            "edgeCount": len(edges),
        },
    )
    # endregion

    inferred_hp_out_start = hp_out_a[0] if hp_out_a else None
    if inferred_hp_out_start is None and hp_out_i:
        # Some captures miss the initial HP_OUT ACTIVE transition.
        # Infer start from earliest related signal so hold/gate metrics stay usable.
        inferred_hp_out_start = _first_signal_time(hp_in_a, fp_in_a, fp_out_a, fp_out_i)

    if (
        inferred_hp_out_start is not None
        and hp_out_i
        and hp_out_i[-1] < inferred_hp_out_start
    ):
        inferred_hp_out_start = _first_signal_time(hp_in_a, fp_in_a, fp_out_a, fp_out_i)

    hp_hold_after_last_frame: float | None = None
    hp_hold_after_last_frame_reason: str | None = None
    hp_assert_to_final_fp_release: float | None = None
    hp_release_before_final_frame_detected = False
    if fp_out_i:
        final_fp_release = fp_out_i[-1]
        if inferred_hp_out_start is not None:
            hp_assert_to_final_fp_release = float(final_fp_release - inferred_hp_out_start)
        hp_release_before_final_frame_detected = any(
            (t < final_fp_release) and (inferred_hp_out_start is None or t >= inferred_hp_out_start)
            for t in hp_out_i
        )
        hp_release_after_final = _first_at_or_after(hp_out_i, final_fp_release)
        if hp_release_after_final is not None:
            hp_hold_after_last_frame = float(hp_release_after_final - final_fp_release)
        else:
            hp_hold_after_last_frame_reason = "no_hp_release_after_final_fp_release"
    else:
        hp_hold_after_last_frame_reason = "no_fp_out_release_edges"

    hp_in_to_hp_out_latency: float | None = None
    hp_out_pre_asserted_before_hp_in = False
    hp_in_to_hp_out_latency_reason: str | None = None
    if hp_in_a and (hp_out_a or inferred_hp_out_start is not None):
        first_hp_in = hp_in_a[0]
        first_hp_out_on_or_after_hp_in = _first_at_or_after(hp_out_a, first_hp_in)
        hp_reference = hp_out_a[0] if hp_out_a else inferred_hp_out_start
        hp_out_pre_asserted_before_hp_in = (hp_reference is not None) and (hp_reference < first_hp_in)
        if hp_out_pre_asserted_before_hp_in:
            hp_in_to_hp_out_latency_reason = "hp_out_already_asserted_before_hp_in"
        elif first_hp_out_on_or_after_hp_in is not None:
            hp_in_to_hp_out_latency = float(first_hp_out_on_or_after_hp_in - first_hp_in)
        elif hp_reference is not None and hp_reference >= first_hp_in:
            hp_in_to_hp_out_latency = float(hp_reference - first_hp_in)
        else:
            hp_in_to_hp_out_latency_reason = "no_hp_out_assert_after_hp_in"
    elif hp_in_a and not hp_out_a:
        hp_in_to_hp_out_latency_reason = "no_hp_out_active_edges"

    hp_interframe_release_count = 0
    hp_interframe_reassert_count = 0
    hp_interframe_low_ms: list[int] = []
    interframe_pair_count = min(len(fp_out_i), max(0, len(fp_out_a) - 1))
    if interframe_pair_count > 0:
        for i in range(interframe_pair_count):
            gap_start = fp_out_i[i]
            gap_end = fp_out_a[i + 1]
            releases = [t for t in hp_out_i if gap_start <= t < gap_end]
            reasserts = [t for t in hp_out_a if gap_start <= t < gap_end]
            hp_interframe_release_count += len(releases)
            hp_interframe_reassert_count += len(reasserts)
            for release_t in releases:
                reassert_t = _first_between(hp_out_a, release_t, gap_end)
                if reassert_t is not None and reassert_t >= release_t:
                    hp_interframe_low_ms.append(reassert_t - release_t)

    wake_only_hold = None
    if inferred_hp_out_start is not None and hp_out_i and not fp_out_a:
        wake_only_hold = float(hp_out_i[-1] - inferred_hp_out_start)

    ignored_fp_count = max(0, len(fp_in_a) - max(1, len(fp_out_a)))

    inter_sequence_gap_ms = None
    second_sequence_start_delay_ms = None
    if len(seq_starts) >= 2:
        second_seq_start = seq_starts[1]
        prev_fp_out_inactive = [t for t in fp_out_i if t < second_seq_start]
        if prev_fp_out_inactive:
            inter_sequence_gap_ms = float(second_seq_start - prev_fp_out_inactive[-1])
        if len(fp_in_a) >= 2:
            second_sequence_start_delay_ms = float(second_seq_start - fp_in_a[1])

    out: dict[str, Any] = {
        "hpInToHpOutLatencyMs": hp_in_to_hp_out_latency,
        "fpInToHpOutLatencyMs": (
            _latency(fp_in_a, hp_out_a)
            if hp_out_a
            else (float(inferred_hp_out_start - fp_in_a[0]) if fp_in_a and inferred_hp_out_start is not None else None)
        ),
        "fpInToFpOutLatencyMs": _latency(fp_in_a, fp_out_a),
        "firstFrameAfLeadMs": (
            float(fp_out_a[0] - inferred_hp_out_start)
            if fp_out_a and inferred_hp_out_start is not None
            else None
        ),
        "firstFrameGateDelayMs": _latency(fp_in_a, fp_out_a),
        "fpPulseWidthMs": float(mean(fp_widths)) if fp_widths else None,
        "frameStartSpacingMs": float(mean(fp_spacings)) if fp_spacings else None,
        "frameEndToStartSpacingMs": (
            float(mean(fp_end_to_start_spacings)) if fp_end_to_start_spacings else None
        ),
        "hpHoldAfterLastFrameMs": hp_hold_after_last_frame,
        "hpReleaseBeforeFinalFrameDetected": hp_release_before_final_frame_detected,
        "hpHoldAfterLastFrameReason": hp_hold_after_last_frame_reason,
        "hpAssertToFinalFrameReleaseMs": hp_assert_to_final_fp_release,
        "hpOutPreAssertedBeforeHpIn": hp_out_pre_asserted_before_hp_in,
        "hpInToHpOutLatencyReason": hp_in_to_hp_out_latency_reason,
        "hpInterFrameReleaseCount": hp_interframe_release_count,
        "hpInterFrameReassertCount": hp_interframe_reassert_count,
        "hpInterFrameLowMs": float(mean(hp_interframe_low_ms)) if hp_interframe_low_ms else None,
        "hpOutContinuityMs": (
            float(hp_out_i[-1] - inferred_hp_out_start)
            if inferred_hp_out_start is not None and hp_out_i
            else None
        ),
        "wakeOnlyHoldMs": wake_only_hold,
        "sequenceCount": len(seq_starts),
        "interSequenceGapMs": inter_sequence_gap_ms,
        "secondSequenceStartDelayMs": second_sequence_start_delay_ms,
        "frameCount": len(fp_out_a),
        "ignoredFpCount": ignored_fp_count,
    }
    # region agent log
    debug_log(
        run_id=run_id,
        hypothesis_id="H1_firmware_cadence_when_ble_connected",
        location="metrics.py:84",
        message="Computed timing metrics",
        data={
            "fpPulseWidthMs": out["fpPulseWidthMs"],
            "frameStartSpacingMs": out["frameStartSpacingMs"],
            "frameEndToStartSpacingMs": out["frameEndToStartSpacingMs"],
            "fpInToFpOutLatencyMs": out["fpInToFpOutLatencyMs"],
            "frameCount": out["frameCount"],
            "sequenceStarts": seq_starts,
            "sequenceStartSource": seq_start_source,
            "expectedSequenceCount": expected_sequence_count,
            "sequenceStartHints": fp_sequence_start_hints or [],
        },
    )
    # endregion
    return out


def aggregate_stats(samples: list[float]) -> dict[str, float]:
    if not samples:
        return {}
    s = sorted(samples)
    n = len(s)
    def _p(q: float) -> float:
        idx = max(0, min(n - 1, int(round((n - 1) * q))))
        return float(s[idx])
    return {
        "min": float(s[0]),
        "mean": float(mean(s)),
        "max": float(s[-1]),
        "p95": _p(0.95),
        "p99": _p(0.99),
    }

