from __future__ import annotations

from contextlib import ExitStack
from dataclasses import asdict, replace
from pathlib import Path
from typing import Any
import json
import threading
import time

try:
    import serial
except ImportError:  # pragma: no cover
    serial = None  # type: ignore[assignment]

from .artifacts import create_run_dir, write_json, write_text
from .ble_adapter import DutBleAdapter, DutBleSession
from .camera_config_protocol import execute_camera_config_protocol
from .debug_log import debug_log
from .evaluator import evaluate_case, evaluate_sc15_power_save_delta
from .fixture_client import FixtureClient
from .metrics import aggregate_stats, extract_metrics
from .telemetry import diff_counters, parse_payload, validate_delta_rules
from .vector_schema import TestVector

DEFAULT_CASE_CAMERA_PARAMS: dict[str, Any] = {
    "enabled": 1,
    "wakeHalfPressHoldTime": "10s",
    "minHalfPressBeforeShutter": "0.5s",
    "shutterPulseDuration": "100ms",
    "StartFrameSpacingMin": "1.0s",
    "PostShutterHalfPressHoldTimeExtension": "2.0s",
    "halfPressInputDebounce": "35ms",
    "fullPressInputDebounce": "20ms",
    "FrameCount": 4,
    "MaxSequenceCount": 4,
    "wakeHoldRefreshPolicy": 0,
    "halfPressDuringBurstPolicy": 0,
    "fullPressWithoutPriorHpPolicy": 0,
    "activityHalfPressHoldPolicy": 0,
    "fpAfterMaxSequenceCountPolicy": 0,
    "inputActivePolarity": 0,
    "outputDriveMode": 0,
    "powerSaveIdleMode": 1,
    "fullPressIgnoreGap": "3.1s",
}


def _normalize_param_value(value: Any) -> Any:
    if isinstance(value, str):
        return value.strip().lower()
    return value


def classify_ble_requirement(vector: TestVector) -> tuple[bool, list[str]]:
    reasons: list[str] = []
    if vector.requires_ble:
        reasons.append("vector explicitly marked requiresBle=true")

    if vector.camera_config_protocol:
        reasons.append("camera config protocol steps")

    tags = {t.strip().lower() for t in vector.tags}
    if "ble-connected" in tags:
        reasons.append("ble-connected scenario")

    non_default_params: list[str] = []
    for name, value in vector.parameters.items():
        if name not in DEFAULT_CASE_CAMERA_PARAMS:
            non_default_params.append(f"{name}=<unknown-default>")
            continue
        default_value = DEFAULT_CASE_CAMERA_PARAMS[name]
        if _normalize_param_value(value) != _normalize_param_value(default_value):
            non_default_params.append(f"{name}={value} (factory={default_value})")
    if non_default_params:
        reasons.append("requires non-default camera params: " + ", ".join(non_default_params))

    return (len(reasons) > 0, reasons)


def _signal_token(signal: str) -> str:
    s = signal.upper()
    if "HP" in s:
        return "HP"
    if "FP" in s:
        return "FP"
    raise ValueError(f"unsupported signal token: {signal}")


def _capture_dut_serial(stop_event: threading.Event, out_lines: list[str], port: str, baud: int) -> None:
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
    ser = serial.Serial(port, baudrate=baud, timeout=0.2)
    start = time.time()
    try:
        ser.reset_input_buffer()
        while not stop_event.is_set():
            raw = ser.readline()
            if not raw:
                continue
            txt = raw.decode("utf-8", errors="replace").rstrip("\r\n")
            ts_ms = int((time.time() - start) * 1000)
            out_lines.append(f"{ts_ms} {txt}")
    finally:
        ser.close()


def _telemetry_is_idle(snapshot: Any) -> bool:
    flags = int(getattr(snapshot, "flags", 0))
    cam_state = int(getattr(snapshot, "camera_state", 0))
    return cam_state == 0 and (flags & 0x03) == 0


def _wait_for_idle_ble(active_ble: DutBleSession, *, timeout_s: float, run_id: str) -> None:
    deadline = time.time() + timeout_s
    last_snapshot = None
    while time.time() < deadline:
        snap = parse_payload(active_ble.read_telemetry_payload())
        last_snapshot = snap
        if _telemetry_is_idle(snap):
            return
        time.sleep(0.2)
    if last_snapshot is not None:
        debug_log(
            run_id=run_id,
            hypothesis_id="H3_parameter_or_readback_mismatch",
            location="runner.py:wait_idle_timeout",
            message="Timed out waiting for idle baseline",
            data={
                "camera_state": last_snapshot.camera_state,
                "flags": last_snapshot.flags,
                "ms_until_wake_deadline": last_snapshot.ms_until_wake_deadline,
                "ms_until_fp_ignore_clear": last_snapshot.ms_until_fp_ignore_clear,
                "ms_until_next_frame": last_snapshot.ms_until_next_frame,
                "ms_until_post_hold_end": last_snapshot.ms_until_post_hold_end,
            },
        )


def run_vector_on_fixture(
    fixture: FixtureClient,
    vector: TestVector,
    run_id: str = "unknown",
    dut_serial_port: str | None = None,
    dut_serial_baud: int = 115200,
) -> dict[str, Any]:
    capture_ms = int(vector.fixture.get("captureAfterLastStimulusMs", 8000))
    dut_serial_lines: list[str] = []
    serial_stop = threading.Event()
    serial_thread: threading.Thread | None = None
    if dut_serial_port:
        serial_thread = threading.Thread(
            target=_capture_dut_serial,
            args=(serial_stop, dut_serial_lines, dut_serial_port, dut_serial_baud),
            daemon=True,
        )
        serial_thread.start()
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
    if serial_thread is not None:
        serial_stop.set()
        serial_thread.join(timeout=2.0)
    metrics = extract_metrics(dump.edges, run_id=run_id)
    return {"dump": dump, "metrics": metrics, "dut_serial_lines": dut_serial_lines}


def run_case(
    vector: TestVector,
    fixture_port: str,
    artifacts_root: Path,
    ble_address: str | None = None,
    ble_session: DutBleSession | None = None,
    strict_camera_readback: bool = True,
    dut_serial_port: str | None = None,
    dut_serial_baud: int = 115200,
) -> dict[str, Any]:
    case_dir = create_run_dir(artifacts_root, vector.case_id)
    fixture = FixtureClient(fixture_port)
    before_tel = None
    after_tel = None
    camera_rw: dict[str, Any] = {}
    protocol_checks: list[Any] = []
    protocol_log: list[dict[str, Any]] = []
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
                # Ensure each case starts from a true idle baseline so short-lead and
                # wake-hold assertions are not contaminated by prior-case activity.
                _wait_for_idle_ble(active_ble, timeout_s=15.0, run_id=vector.case_id)
                before_payload = active_ble.read_telemetry_payload()
                before_tel = parse_payload(before_payload)

                if vector.camera_config_protocol:
                    protocol_checks, protocol_log = execute_camera_config_protocol(
                        vector.camera_config_protocol,
                        active_ble,
                        fixture=fixture,
                        run_id=vector.case_id,
                    )

                if vector.baseline_config_write:
                    # Start every case from a known baseline so omitted vector params
                    # do not inherit stale values from prior scenarios.
                    requested_params = dict(DEFAULT_CASE_CAMERA_PARAMS)
                    requested_params.update(vector.parameters)
                    camera_rw = active_ble.write_camera_config_params(requested_params, strict=strict_camera_readback)

            result = run_vector_on_fixture(
                fixture,
                vector,
                run_id=vector.case_id,
                dut_serial_port=dut_serial_port,
                dut_serial_baud=dut_serial_baud,
            )

            if ble_address and active_ble is not None:
                after_payload = active_ble.read_telemetry_payload()
                after_tel = parse_payload(after_payload)

        telemetry_available = before_tel is not None and after_tel is not None
        deltas = diff_counters(before_tel, after_tel)
        delta_notes = validate_delta_rules(deltas)
        if not telemetry_available and isinstance(vector.expect.get("telemetryDeltas"), dict):
            delta_notes.append("telemetry_assertions_skipped_no_ble")
        sequence_start_hints = [
            int(step.at_ms)
            for step in vector.stimulus
            if (step.signal.upper() in {"FP_IN_STIM", "FP_IN"}) and (step.state == "active")
        ]
        expected_sequence_count: int | None = None
        if telemetry_available:
            expected_sequence_count = int(deltas.get("acceptedFpCount", 0))
        else:
            sequences_expect = vector.expect.get("sequences")
            if sequences_expect is not None:
                expected_sequence_count = int(sequences_expect)

        metrics = extract_metrics(
            result["dump"].edges,
            run_id=vector.case_id,
            fp_sequence_start_hints=sequence_start_hints,
            expected_sequence_count=expected_sequence_count,
        )
        result["metrics"] = metrics

        checks = protocol_checks + evaluate_case(
            vector.expect,
            metrics,
            deltas,
            telemetry_available=telemetry_available,
            run_id=vector.case_id,
            vector_parameters=vector.parameters,
        )
        passed = all(c.passed for c in checks)
        timing_total = sum(1 for c in checks if c.category == "timing")
        timing_passed = sum(1 for c in checks if c.category == "timing" and c.passed)
        functional_total = sum(1 for c in checks if c.category not in ("timing", "protocol"))
        functional_passed = sum(1 for c in checks if c.category not in ("timing", "protocol") and c.passed)
        protocol_total = sum(1 for c in checks if c.category == "protocol")
        protocol_passed = sum(1 for c in checks if c.category == "protocol" and c.passed)
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
                "protocol_passed": protocol_passed,
                "protocol_total": protocol_total,
            },
            "failure_class": failure_class,
            "camera_rw": camera_rw,
            "camera_config_protocol_log": protocol_log,
            "passed": passed,
        }
        write_json(case_dir / "result.json", payload)
        write_text(case_dir / "raw_edges.log", "\n".join([f"{e.t_ms} {e.signal} {e.state}" for e in result["dump"].edges]) + "\n")
        if result.get("dut_serial_lines"):
            write_text(case_dir / "dut_serial.log", "\n".join(result["dut_serial_lines"]) + "\n")
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

