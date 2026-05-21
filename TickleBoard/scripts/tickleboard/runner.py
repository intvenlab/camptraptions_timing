from __future__ import annotations

from contextlib import ExitStack
from dataclasses import asdict, replace
from pathlib import Path
from typing import Any
import json

from .artifacts import create_run_dir, write_json, write_text
from .ble_adapter import DutBleAdapter, DutBleSession
from .debug_log import debug_log
from .evaluator import evaluate_case, evaluate_sc15_power_save_delta
from .fixture_client import FixtureClient
from .metrics import aggregate_stats, extract_metrics
from .telemetry import diff_counters, parse_payload, validate_delta_rules
from .vector_schema import TestVector


def _signal_token(signal: str) -> str:
    s = signal.upper()
    if "HP" in s:
        return "HP"
    if "FP" in s:
        return "FP"
    raise ValueError(f"unsupported signal token: {signal}")


def run_vector_on_fixture(fixture: FixtureClient, vector: TestVector, run_id: str = "unknown") -> dict[str, Any]:
    capture_ms = int(vector.fixture.get("captureAfterLastStimulusMs", 8000))
    fixture.command_ok("RESET")
    fixture.command_ok("MAP HP_IN=5 FP_IN=4 HP_OUT=3 FP_OUT=2 POL=ACTIVE_LOW")
    fixture.command_ok(f"ARM {capture_ms}")
    for step in vector.stimulus:
        sig = _signal_token(step.signal)
        if step.duration_ms is not None:
            fixture.command_ok(f"PULSE {sig} {step.at_ms} {step.duration_ms}")
        else:
            state = "ACTIVE" if step.state == "active" else "INACTIVE"
            fixture.command_ok(f"LEVEL {sig} {step.at_ms} {state}")
    fixture.command_ok("RUN", total_timeout_s=(capture_ms / 1000.0) + 6.0)
    dump = fixture.dump()
    metrics = extract_metrics(dump.edges, run_id=run_id)
    return {"dump": dump, "metrics": metrics}


def run_case(
    vector: TestVector,
    fixture_port: str,
    artifacts_root: Path,
    ble_address: str | None = None,
    ble_session: DutBleSession | None = None,
    strict_camera_readback: bool = True,
) -> dict[str, Any]:
    case_dir = create_run_dir(artifacts_root, vector.case_id)
    fixture = FixtureClient(fixture_port)
    before_tel = None
    after_tel = None
    camera_rw: dict[str, Any] = {}
    try:
        with ExitStack() as stack:
            # region agent log
            debug_log(
                run_id=vector.case_id,
                hypothesis_id="H3_parameter_or_readback_mismatch",
                location="runner.py:61",
                message="Case start context",
                data={
                    "caseId": vector.case_id,
                    "scenario": vector.scenario,
                    "bleAddressProvided": ble_address is not None,
                    "requestedShutterPulseDuration": vector.parameters.get("shutterPulseDuration"),
                },
            )
            # endregion
            active_ble = ble_session
            if ble_address and active_ble is None:
                active_ble = stack.enter_context(DutBleAdapter(ble_address).open_session())

            if ble_address and active_ble is not None:
                beacon = DutBleAdapter(ble_address).read_beacon_snapshot(timeout_s=4.0)
                # region agent log
                debug_log(
                    run_id=vector.case_id,
                    hypothesis_id="H5_fix_not_deployed_or_not_running",
                    location="runner.py:71",
                    message="Beacon snapshot before BLE connect",
                    data={
                        "beaconFound": beacon is not None,
                        "beacon": beacon or {},
                    },
                )
                # endregion
                before_payload = active_ble.read_telemetry_payload()
                before_tel = parse_payload(before_payload)
                camera_rw = active_ble.write_camera_config_params(vector.parameters, strict=strict_camera_readback)

            result = run_vector_on_fixture(fixture, vector, run_id=vector.case_id)

            if ble_address and active_ble is not None:
                after_payload = active_ble.read_telemetry_payload()
                after_tel = parse_payload(after_payload)

        deltas = diff_counters(before_tel, after_tel)
        delta_notes = validate_delta_rules(deltas)
        checks = evaluate_case(vector.expect, result["metrics"], deltas, run_id=vector.case_id)
        passed = all(c.passed for c in checks)
        timing_total = sum(1 for c in checks if c.category == "timing")
        timing_passed = sum(1 for c in checks if c.category == "timing" and c.passed)
        functional_total = sum(1 for c in checks if c.category != "timing")
        functional_passed = sum(1 for c in checks if c.category != "timing" and c.passed)
        failed_timing = any((c.category == "timing") and (not c.passed) for c in checks)
        failure_class = "pass" if passed else ("timing_tolerance" if failed_timing else "logic_mismatch")
        # region agent log
        debug_log(
            run_id=vector.case_id,
            hypothesis_id="H4_tolerance_checking_logic",
            location="runner.py:94",
            message="Case evaluation summary",
            data={
                "passed": passed,
                "checkNames": [c.name for c in checks],
                "checkPass": [c.passed for c in checks],
                "cameraReadback": camera_rw.get("readback_norm", {}),
                "timingPassed": timing_passed,
                "timingTotal": timing_total,
                "failureClass": failure_class,
            },
        )
        # endregion

        payload = {
            "case_id": vector.case_id,
            "scenario": vector.scenario,
            "description": vector.description,
            "vector": asdict(vector),
            "fixture_dump": asdict(result["dump"]),
            "metrics": result["metrics"],
            "telemetry_before": asdict(before_tel) if before_tel else None,
            "telemetry_after": asdict(after_tel) if after_tel else None,
            "telemetry_deltas": deltas,
            "telemetry_notes": delta_notes,
            "checks": [asdict(c) for c in checks],
            "check_summary": {
                "timing_passed": timing_passed,
                "timing_total": timing_total,
                "functional_passed": functional_passed,
                "functional_total": functional_total,
            },
            "failure_class": failure_class,
            "camera_rw": camera_rw,
            "passed": passed,
        }
        write_json(case_dir / "result.json", payload)
        write_text(case_dir / "raw_edges.log", "\n".join([f"{e.t_ms} {e.signal} {e.state}" for e in result["dump"].edges]) + "\n")
        return {
            "run_dir": str(case_dir),
            "case_id": vector.case_id,
            "scenario": vector.scenario,
            "passed": passed,
            "failure_class": failure_class,
            "timing_passed": timing_passed,
            "timing_total": timing_total,
            "functional_passed": functional_passed,
            "functional_total": functional_total,
            "notes": delta_notes,
        }
    finally:
        fixture.close()


def preflight(fixture_port: str, ble_address: str | None = None) -> dict[str, Any]:
    fixture = FixtureClient(fixture_port)
    try:
        ident = fixture.identify()
        out: dict[str, Any] = {"fixture_identity": ident.line}
        if ble_address:
            ble = DutBleAdapter(ble_address)
            cfg = ble.read_camera_config()
            tel = ble.read_telemetry_payload()
            out["ble_camera_config_len"] = len(cfg)
            out["ble_telemetry_len"] = len(tel)
        return out
    finally:
        fixture.close()


def run_sc15_budget(
    base_vector: TestVector,
    fixture_port: str,
    ble_address: str,
    artifacts_root: Path,
    repeats: int = 20,
) -> dict[str, Any]:
    if repeats < 2:
        raise ValueError("repeats must be >= 2")

    enabled_samples: list[float] = []
    disabled_samples: list[float] = []
    runs: list[dict[str, Any]] = []

    adapter = DutBleAdapter(ble_address)
    with adapter.open_session() as ble_session:
        for mode in (1, 0):
            mode_name = "enabled" if mode == 1 else "disabled"
            for i in range(repeats):
                params = dict(base_vector.parameters)
                params["powerSaveIdleMode"] = mode
                v = replace(base_vector, parameters=params, case_id=f"{base_vector.case_id}-{mode_name}-{i+1:02d}")
                rec = run_case(
                    v,
                    fixture_port,
                    artifacts_root,
                    ble_address=ble_address,
                    ble_session=ble_session,
                    strict_camera_readback=True,
                )
                runs.append(rec)
                result_json = Path(rec["run_dir"]) / "result.json"
                payload = json.loads(result_json.read_text(encoding="utf-8"))
                value = payload["metrics"].get("fpInToFpOutLatencyMs")
                if isinstance(value, (int, float)):
                    if mode == 1:
                        enabled_samples.append(float(value))
                    else:
                        disabled_samples.append(float(value))

    enabled_stats = aggregate_stats(enabled_samples)
    disabled_stats = aggregate_stats(disabled_samples)
    check = evaluate_sc15_power_save_delta(enabled_samples, disabled_samples)

    out = {
        "scenario": "SC-15",
        "repeats": repeats,
        "enabled_samples": enabled_samples,
        "disabled_samples": disabled_samples,
        "enabled_stats": enabled_stats,
        "disabled_stats": disabled_stats,
        "delta_check": asdict(check),
        "runs": runs,
    }
    summary_dir = create_run_dir(artifacts_root, "SC15-BUDGET")
    write_json(summary_dir / "sc15_summary.json", out)
    return out

