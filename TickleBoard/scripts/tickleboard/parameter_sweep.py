from __future__ import annotations

from pathlib import Path
from typing import Any

import yaml


def _timing_case(
    *,
    start_spacing_s: float,
    shutter_pulse_ms: int,
    frame_count: int,
) -> dict[str, Any]:
    spacing_ms = int(round(start_spacing_s * 1000))
    case_id = f"SW-TIMING-Y{spacing_ms:04d}-P{shutter_pulse_ms:04d}-F{frame_count:02d}"
    frame_spacing_expect: Any = spacing_ms
    if frame_count <= 1:
        frame_spacing_expect = None

    fixture_ms = 12000
    if frame_count > 1:
        burst_window = (frame_count - 1) * spacing_ms + shutter_pulse_ms
        fixture_ms = max(fixture_ms, int(1000 + burst_window + 7000))

    expect: dict[str, Any] = {
        "fpOut": {
            "pulseCount": frame_count,
            "pulseWidthMs": shutter_pulse_ms,
        },
        "sequences": 1,
        "timing": {
            "firstFrameGateDelayMs": {"minMs": 0, "maxMs": 80},
        },
        "telemetryDeltas": {
            "acceptedFpCount": 1,
            "sequenceCompletedCount": 1,
            "MaxSequenceExceededCount": 0,
        },
    }
    if frame_spacing_expect is not None:
        expect["fpOut"]["frameSpacingMs"] = frame_spacing_expect

    return {
        "id": case_id,
        "scenario": "SWEEP-TIMING",
        "description": (
            "Timing sweep case: verify StartFrameSpacingMin/shutterPulseDuration/FrameCount "
            "mapping into output timing metrics."
        ),
        "tags": ["sweep", "timing", "generated"],
        "requiresBle": True,
        "parameters": {
            "enabled": 1,
            "wakeHalfPressHoldTime": "10s",
            "minHalfPressBeforeShutter": "0.5s",
            "shutterPulseDuration": f"{shutter_pulse_ms}ms",
            "StartFrameSpacingMin": f"{start_spacing_s:.1f}s",
            "PostShutterHalfPressHoldTimeExtension": "2.0s",
            "halfPressInputDebounce": "35ms",
            "fullPressInputDebounce": "20ms",
            "FrameCount": frame_count,
            "MaxSequenceCount": 64,
            "wakeHoldRefreshPolicy": 0,
            "halfPressDuringBurstPolicy": 0,
            "fullPressWithoutPriorHpPolicy": 0,
            "activityHalfPressHoldPolicy": 0,
            "fpAfterMaxSequenceCountPolicy": 0,
            "inputActivePolarity": 0,
            "outputDriveMode": 0,
            "powerSaveIdleMode": 1,
            "fullPressIgnoreGap": "3.1s",
        },
        "fixture": {"captureAfterLastStimulusMs": fixture_ms},
        "stimulus": [
            {"atMs": 0, "signal": "HP_IN_STIM", "state": "active", "durationMs": 100},
            {"atMs": 1000, "signal": "FP_IN_STIM", "state": "active", "durationMs": 100},
        ],
        "expect": expect,
        "metrics": [
            "frameCount",
            "fpPulseWidthMs",
            "frameStartSpacingMs",
            "firstFrameGateDelayMs",
            "sequenceCount",
        ],
    }


def _cap_gap_case(*, max_sequence_count: int, full_press_ignore_gap_s: float) -> dict[str, Any]:
    gap_ms = int(round(full_press_ignore_gap_s * 1000))
    case_id = f"SW-CAPGAP-M{max_sequence_count:02d}-G{gap_ms:04d}"
    t_first = 1000
    t_gap_before = t_first + gap_ms - 100
    t_gap_after = t_first + gap_ms + 150

    accepted_fp = 1 + (1 if max_sequence_count >= 2 else 0)
    max_sequence_exceeded = 0 if max_sequence_count >= 2 else 1

    return {
        "id": case_id,
        "scenario": "SWEEP-CAP-GAP",
        "description": (
            "Interaction sweep case: verify fullPressIgnoreGap boundary behavior and "
            "MaxSequenceCount cap behavior together."
        ),
        "tags": ["sweep", "cap-gap", "generated"],
        "requiresBle": True,
        "parameters": {
            "enabled": 1,
            "wakeHalfPressHoldTime": "10s",
            "minHalfPressBeforeShutter": "0.5s",
            "shutterPulseDuration": "100ms",
            "StartFrameSpacingMin": "1.0s",
            "PostShutterHalfPressHoldTimeExtension": "2.0s",
            "halfPressInputDebounce": "35ms",
            "fullPressInputDebounce": "20ms",
            "FrameCount": 1,
            "MaxSequenceCount": max_sequence_count,
            "wakeHoldRefreshPolicy": 0,
            "halfPressDuringBurstPolicy": 0,
            "fullPressWithoutPriorHpPolicy": 0,
            "activityHalfPressHoldPolicy": 0,
            "fpAfterMaxSequenceCountPolicy": 0,
            "inputActivePolarity": 0,
            "outputDriveMode": 0,
            "powerSaveIdleMode": 1,
            "fullPressIgnoreGap": f"{full_press_ignore_gap_s:.1f}s",
        },
        "fixture": {"captureAfterLastStimulusMs": max(13000, t_gap_after + 3500)},
        "stimulus": [
            {"atMs": 0, "signal": "HP_IN_STIM", "state": "active", "durationMs": 100},
            {"atMs": t_first, "signal": "FP_IN_STIM", "state": "active", "durationMs": 100},
            {"atMs": t_gap_before, "signal": "FP_IN_STIM", "state": "active", "durationMs": 80},
            {"atMs": t_gap_after, "signal": "FP_IN_STIM", "state": "active", "durationMs": 100},
        ],
        "expect": {
            "fpOut": {"pulseCount": accepted_fp},
            "sequences": accepted_fp,
            "telemetryDeltas": {
                "acceptedFpCount": accepted_fp,
                "ignoredFpDuringGapCount": 1,
                "MaxSequenceExceededCount": max_sequence_exceeded,
                "sequenceCompletedCount": accepted_fp,
            },
            "timing": {
                "firstFrameGateDelayMs": {"minMs": 0, "maxMs": 100},
            },
        },
        "metrics": ["frameCount", "sequenceCount", "firstFrameGateDelayMs", "ignoredFpCount"],
    }


def build_parameter_sweep_vectors(output_root: Path) -> dict[str, Any]:
    scenarios_dir = output_root / "generated" / "parameter_sweep"
    suites_dir = output_root / "suites"
    scenarios_dir.mkdir(parents=True, exist_ok=True)
    suites_dir.mkdir(parents=True, exist_ok=True)

    generated_files: list[str] = []
    suite_cases: list[str] = []

    spacing_values = [0.2, 1.0, 2.5]
    pulse_values = [20, 100, 500]
    frame_values = [1, 4, 8]
    for spacing in spacing_values:
        for pulse in pulse_values:
            for frames in frame_values:
                payload = _timing_case(
                    start_spacing_s=spacing,
                    shutter_pulse_ms=pulse,
                    frame_count=frames,
                )
                path = scenarios_dir / f"{payload['id']}.yaml"
                path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")
                generated_files.append(str(path))
                suite_cases.append(f"../generated/parameter_sweep/{path.name}")

    max_sequence_values = [1, 2, 4, 16, 64]
    gap_values = [0.5, 1.5, 3.1, 8.0]
    for max_seq in max_sequence_values:
        for gap in gap_values:
            payload = _cap_gap_case(max_sequence_count=max_seq, full_press_ignore_gap_s=gap)
            path = scenarios_dir / f"{payload['id']}.yaml"
            path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")
            generated_files.append(str(path))
            suite_cases.append(f"../generated/parameter_sweep/{path.name}")

    suite_payload = {
        "name": "parameter_sweep_suite",
        "description": (
            "Generated parameter sweep for StartFrameSpacingMin, shutterPulseDuration, FrameCount, "
            "MaxSequenceCount, and fullPressIgnoreGap."
        ),
        "cases": suite_cases,
    }
    suite_path = suites_dir / "parameter_sweep_suite.yaml"
    suite_path.write_text(yaml.safe_dump(suite_payload, sort_keys=False), encoding="utf-8")

    return {
        "suite_path": str(suite_path),
        "scenario_dir": str(scenarios_dir),
        "case_count": len(suite_cases),
        "generated_files": generated_files,
    }
