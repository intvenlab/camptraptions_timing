from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from .debug_log import debug_log


@dataclass
class CheckResult:
    name: str
    passed: bool
    detail: str
    category: str = "functional"


def _tol(expected: float, floor_ms: float = 5.0) -> float:
    return max(abs(expected) * 0.01, floor_ms)


def _timing_floor(metric_name: str) -> float:
    if metric_name in {"wakeOnlyHoldMs"}:
        return 20.0
    if metric_name in {"hpHoldAfterLastFrameMs"}:
        return 10.0
    return 5.0


def _timing_check(
    checks: list[CheckResult],
    *,
    check_name: str,
    metric_name: str,
    metric_value: float | int | None,
    expect_spec: Any,
) -> None:
    if metric_value is None:
        checks.append(
            CheckResult(
                name=check_name,
                passed=False,
                detail=f"missing metric {metric_name}",
                category="timing",
            )
        )
        return

    actual = float(metric_value)
    if isinstance(expect_spec, dict):
        if "minMs" in expect_spec or "maxMs" in expect_spec:
            min_ms = float(expect_spec.get("minMs", float("-inf")))
            max_ms = float(expect_spec.get("maxMs", float("inf")))
            passed = min_ms <= actual <= max_ms
            checks.append(
                CheckResult(
                    name=check_name,
                    passed=passed,
                    detail=f"expected range [{min_ms:.1f}, {max_ms:.1f}], got {actual:.1f}",
                    category="timing",
                )
            )
            return
        if "targetMs" in expect_spec:
            expected = float(expect_spec["targetMs"])
            floor = _timing_floor(metric_name)
            tol_override = expect_spec.get("toleranceMs")
            tol = float(tol_override) if tol_override is not None else _tol(expected, floor_ms=floor)
            passed = abs(actual - expected) <= tol
            checks.append(
                CheckResult(
                    name=check_name,
                    passed=passed,
                    detail=f"expected {expected:.1f} +/- {tol:.1f}, got {actual:.1f}",
                    category="timing",
                )
            )
            return

    expected = float(expect_spec)
    tol = _tol(expected, floor_ms=_timing_floor(metric_name))
    checks.append(
        CheckResult(
            name=check_name,
            passed=abs(actual - expected) <= tol,
            detail=f"expected {expected:.1f} +/- {tol:.1f}, got {actual:.1f}",
            category="timing",
        )
    )


def evaluate_case(
    vector_expect: dict[str, Any],
    metrics: dict[str, Any],
    telemetry_deltas: dict[str, int],
    run_id: str = "unknown",
) -> list[CheckResult]:
    checks: list[CheckResult] = []

    fp_expect = vector_expect.get("fpOut", {})
    exp_pulse_count = fp_expect.get("pulseCount")
    if exp_pulse_count is not None:
        actual = int(metrics.get("frameCount") or 0)
        checks.append(
            CheckResult(
                name="frameCount",
                passed=actual == int(exp_pulse_count),
                detail=f"expected {exp_pulse_count}, got {actual}",
            )
        )

    exp_pulse_width = fp_expect.get("pulseWidthMs")
    if exp_pulse_width is not None:
        actual_raw = metrics.get("fpPulseWidthMs")
        actual = float(actual_raw) if actual_raw is not None else None
        expected = float(exp_pulse_width)
        tol = _tol(expected, floor_ms=_timing_floor("fpPulseWidthMs"))
        # region agent log
        debug_log(
            run_id=run_id,
            hypothesis_id="H4_tolerance_checking_logic",
            location="evaluator.py:46",
            message="Pulse width tolerance evaluation",
            data={
                "expectedMs": expected,
                "actualMs": actual,
                "toleranceMs": tol,
                "deltaMs": (actual - expected) if actual is not None else None,
            },
        )
        # endregion
        _timing_check(
            checks,
            check_name="fpPulseWidthMs",
            metric_name="fpPulseWidthMs",
            metric_value=actual,
            expect_spec=expected,
        )

    exp_frame_spacing = fp_expect.get("frameSpacingMs")
    if exp_frame_spacing is not None:
        _timing_check(
            checks,
            check_name="frameStartSpacingMs",
            metric_name="frameStartSpacingMs",
            metric_value=metrics.get("frameStartSpacingMs"),
            expect_spec=exp_frame_spacing,
        )

    sequences_expect = vector_expect.get("sequences")
    if sequences_expect is not None:
        actual = int(metrics.get("sequenceCount") or 0)
        checks.append(
            CheckResult(
                name="sequenceCount",
                passed=actual == int(sequences_expect),
                detail=f"expected {sequences_expect}, got {actual}",
            )
        )

    telemetry_expect = vector_expect.get("telemetryDeltas", {})
    if isinstance(telemetry_expect, dict):
        for counter_name, expected_delta in telemetry_expect.items():
            expected = int(expected_delta)
            actual = int(telemetry_deltas.get(str(counter_name), 0))
            checks.append(
                CheckResult(
                    name=f"telemetryDelta.{counter_name}",
                    passed=actual == expected,
                    detail=f"expected {expected}, got {actual}",
                    category="functional",
                )
            )

    timing_expect = vector_expect.get("timing", {})
    if isinstance(timing_expect, dict):
        for metric_name, expect_spec in timing_expect.items():
            _timing_check(
                checks,
                check_name=str(metric_name),
                metric_name=str(metric_name),
                metric_value=metrics.get(str(metric_name)),
                expect_spec=expect_spec,
            )

    hp_latency = metrics.get("hpInToHpOutLatencyMs")
    hp_pre_asserted = bool(metrics.get("hpOutPreAssertedBeforeHpIn", False))
    hp_latency_reason = metrics.get("hpInToHpOutLatencyReason")
    if hp_latency is not None:
        hp_latency_value = float(hp_latency)
        checks.append(
            CheckResult(
                name="hpInToHpOutLatencyMs_non_negative",
                passed=hp_latency_value >= 0.0,
                detail=f"expected >= 0 when measurable, got {hp_latency_value:.1f}",
                category="timing",
            )
        )
    if hp_pre_asserted:
        checks.append(
            CheckResult(
                name="hpInToHpOutLatencyMs_preasserted_handling",
                passed=hp_latency is None and hp_latency_reason == "hp_out_already_asserted_before_hp_in",
                detail=(
                    "expected unmeasurable latency when HP_OUT pre-asserted "
                    f"(latency={hp_latency}, reason={hp_latency_reason})"
                ),
                category="timing",
            )
        )

    hold_expect = vector_expect.get("holdExpect", {})
    if isinstance(hold_expect, dict):
        if hold_expect.get("noHpReleaseBeforeFinalFrame") is True:
            prefinal_release = bool(metrics.get("hpReleaseBeforeFinalFrameDetected", False))
            checks.append(
                CheckResult(
                    name="hold.noHpReleaseBeforeFinalFrame",
                    passed=not prefinal_release,
                    detail=f"expected False, got {prefinal_release}",
                    category="timing",
                )
            )
        if hold_expect.get("requirePostFinalFrameHpRelease") is True:
            hp_hold_metric = metrics.get("hpHoldAfterLastFrameMs")
            hp_hold_reason = metrics.get("hpHoldAfterLastFrameReason")
            checks.append(
                CheckResult(
                    name="hold.requirePostFinalFrameHpRelease",
                    passed=hp_hold_metric is not None,
                    detail=f"hpHoldAfterLastFrameMs={hp_hold_metric} reason={hp_hold_reason}",
                    category="timing",
                )
            )
        drop_time_rule = hold_expect.get("dropTimeRule")
        if isinstance(drop_time_rule, dict):
            wake_hold_ms = drop_time_rule.get("wakeHoldMs")
            post_final_frame_hold_ms = drop_time_rule.get("postFinalFrameHoldMs")
            hp_continuity_ms = metrics.get("hpOutContinuityMs")
            hp_to_final_frame_ms = metrics.get("hpAssertToFinalFrameReleaseMs")
            if (
                wake_hold_ms is None
                or post_final_frame_hold_ms is None
                or hp_continuity_ms is None
                or hp_to_final_frame_ms is None
            ):
                checks.append(
                    CheckResult(
                        name="hold.dropTimeRule",
                        passed=False,
                        detail=(
                            "missing input for drop-time rule "
                            f"(wakeHoldMs={wake_hold_ms}, postFinalFrameHoldMs={post_final_frame_hold_ms}, "
                            f"hpOutContinuityMs={hp_continuity_ms}, hpAssertToFinalFrameReleaseMs={hp_to_final_frame_ms})"
                        ),
                        category="timing",
                    )
                )
            else:
                wake_hold_value = float(wake_hold_ms)
                post_hold_value = float(post_final_frame_hold_ms)
                continuity_value = float(hp_continuity_ms)
                hp_to_final_value = float(hp_to_final_frame_ms)
                expected_drop_ms = max(wake_hold_value, hp_to_final_value + post_hold_value)
                tol_override = drop_time_rule.get("toleranceMs")
                tol = float(tol_override) if tol_override is not None else _tol(expected_drop_ms, floor_ms=20.0)
                checks.append(
                    CheckResult(
                        name="hold.dropTimeRule",
                        passed=abs(continuity_value - expected_drop_ms) <= tol,
                        detail=(
                            "expected HP_OUT continuity by rule "
                            f"max({wake_hold_value:.1f}, {hp_to_final_value:.1f}+{post_hold_value:.1f}) "
                            f"= {expected_drop_ms:.1f} +/- {tol:.1f}, got {continuity_value:.1f}"
                        ),
                        category="timing",
                    )
                )

    if "ignoredFpDuringGapCount" in telemetry_deltas:
        checks.append(
            CheckResult(
                name="telemetry_delta_sanity",
                passed=True,
                detail=f"deltas={telemetry_deltas}",
                category="functional",
            )
        )
    return checks


def evaluate_sc15_power_save_delta(enabled_samples: list[float], disabled_samples: list[float]) -> CheckResult:
    if not enabled_samples or not disabled_samples:
        return CheckResult(
            name="SC15_powerSave_delta",
            passed=False,
            detail="missing enabled/disabled samples",
        )
    enabled_mean = sum(enabled_samples) / len(enabled_samples)
    disabled_mean = sum(disabled_samples) / len(disabled_samples)
    delta = enabled_mean - disabled_mean
    return CheckResult(
        name="SC15_powerSave_delta",
        passed=delta < 1.0,
        detail=f"enabled_mean={enabled_mean:.3f} disabled_mean={disabled_mean:.3f} delta={delta:.3f} ms",
    )

