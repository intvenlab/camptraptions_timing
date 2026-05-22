from __future__ import annotations

import threading
import time
from typing import Any

from .ble_adapter import DutBleSession
from .camera_config import (
    apply_named_parameters,
    decode_camera_config,
    default_camera_config_bytes,
)
from .constants import CAMCFG_STATUS_BY_NAME, CAMERA_CONFIG_LEN, CAMERA_FIELDS
from .evaluator import CheckResult
from .fixture_client import FixtureClient
from .telemetry import parse_payload


def _signal_token(signal: str) -> str:
    s = signal.upper()
    if "HP" in s:
        return "HP"
    if "FP" in s:
        return "FP"
    raise ValueError(f"unsupported signal token: {signal}")


def _telemetry_is_idle(snapshot: Any) -> bool:
    flags = int(getattr(snapshot, "flags", 0))
    cam_state = int(getattr(snapshot, "camera_state", 0))
    return cam_state == 0 and (flags & 0x03) == 0


def _wait_for_idle_ble(session: DutBleSession, *, timeout_s: float, run_id: str) -> None:
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        snap = parse_payload(session.read_telemetry_payload())
        if _telemetry_is_idle(snap):
            return
        time.sleep(0.2)
    _ = run_id


def _status_code(name: str) -> int:
    key = name.strip().upper()
    if key not in CAMCFG_STATUS_BY_NAME:
        raise ValueError(f"unknown camera config status name: {name!r}")
    return CAMCFG_STATUS_BY_NAME[key]


def _write_field_raw(buf: bytearray, field_name: str, value: int) -> None:
    field = CAMERA_FIELDS[field_name]
    if field.scale in {"ms10_to_u16"}:
        buf[field.index] = value & 0xFF
        buf[field.index + 1] = (value >> 8) & 0xFF
    else:
        buf[field.index] = value


def _build_raw_payload(spec: dict[str, Any]) -> bytes:
    if "payloadHex" in spec:
        payload = bytes.fromhex(str(spec["payloadHex"]).replace(" ", ""))
        if "truncateTo" in spec:
            payload = payload[: int(spec["truncateTo"])]
        return payload

    if spec.get("payloadFromDefaults"):
        base = bytearray(default_camera_config_bytes())
        mutate = dict(spec.get("mutate", {}))
        truncate = mutate.pop("truncateTo", None)
        version = mutate.pop("version", None)
        for key, val in mutate.items():
            if key not in CAMERA_FIELDS:
                raise ValueError(f"unsupported mutate field: {key}")
            _write_field_raw(base, key, int(val))
        if version is not None:
            base[0] = int(version)
        if truncate is not None:
            return bytes(base[: int(truncate)])
        return bytes(base)

    raise ValueError("writeRaw step requires payloadHex or payloadFromDefaults")


def _check_status(step_name: str, expected: str, actual: int, actual_name: str) -> CheckResult:
    expected_code = _status_code(expected)
    passed = actual == expected_code
    return CheckResult(
        name=f"{step_name}.writeStatus",
        passed=passed,
        detail=f"expected {expected} (0x{expected_code:02X}), got {actual_name} (0x{actual:02X})",
        category="protocol",
    )


def _check_config_unchanged(step_name: str, before_hex: str, after_hex: str) -> CheckResult:
    passed = before_hex == after_hex
    return CheckResult(
        name=f"{step_name}.configUnchanged",
        passed=passed,
        detail="readback unchanged" if passed else f"before={before_hex} after={after_hex}",
        category="protocol",
    )


def _check_readback_named(step_name: str, readback: bytes, expected: dict[str, Any]) -> list[CheckResult]:
    decoded = decode_camera_config(readback[:CAMERA_CONFIG_LEN])
    _, expected_norm = apply_named_parameters(bytes(default_camera_config_bytes()), expected)
    checks: list[CheckResult] = []
    for name, value in expected.items():
        if name == "version":
            actual = decoded.get("version", readback[0] if readback else -1)
            expected_val = int(value)
        elif name in expected_norm:
            actual = decoded.get(name)
            expected_val = expected_norm[name]
        else:
            checks.append(
                CheckResult(
                    name=f"{step_name}.readback.{name}",
                    passed=False,
                    detail=f"unknown field {name}",
                    category="protocol",
                )
            )
            continue
        passed = int(actual) == int(expected_val)
        checks.append(
            CheckResult(
                name=f"{step_name}.readback.{name}",
                passed=passed,
                detail=f"expected {expected_val}, got {actual}",
                category="protocol",
            )
        )
    return checks


def _write_named_during_activity(
    session: DutBleSession,
    fixture: FixtureClient,
    spec: dict[str, Any],
    *,
    run_id: str,
) -> tuple[list[CheckResult], dict[str, Any]]:
    step_name = str(spec.get("stepName", "writeNamedDuringActivity"))
    capture_ms = int(spec.get("captureAfterLastStimulusMs", 12000))
    write_at_ms = int(spec.get("writeAtMs", 1500))
    params = dict(spec.get("params", {}))
    expect_status = str(spec.get("expectStatus", "NACK_BUSY"))
    require_unchanged = bool(spec.get("configUnchanged", False))

    fixture.command_ok("RESET")
    fixture.command_ok("MAP HP_IN=5 FP_IN=4 HP_OUT=3 FP_OUT=2 POL=ACTIVE_LOW")
    fixture.command_ok(f"ARM {capture_ms}")
    for stim in spec.get("stimulus", []):
        sig = _signal_token(str(stim.get("signal", "")))
        at_ms = int(stim.get("atMs", 0))
        duration_ms = int(stim.get("durationMs", 100))
        fixture.command_ok(f"PULSE {sig} {at_ms} {duration_ms}")

    run_error: list[str] = []

    def _run_fixture() -> None:
        try:
            fixture.command_ok("RUN", total_timeout_s=(capture_ms / 1000.0) + 8.0)
        except Exception as exc:  # pragma: no cover
            run_error.append(str(exc))

    thread = threading.Thread(target=_run_fixture, daemon=True)
    thread.start()
    time.sleep(write_at_ms / 1000.0)

    snap = parse_payload(session.read_telemetry_payload())
    activity_active = not _telemetry_is_idle(snap)
    before_hex = session.read_camera_config()[:CAMERA_CONFIG_LEN].hex()
    rw = session.write_camera_config_params(params, strict=False)
    thread.join(timeout=(capture_ms / 1000.0) + 10.0)

    checks: list[CheckResult] = [
        CheckResult(
            name=f"{step_name}.activityActiveAtWrite",
            passed=activity_active,
            detail=f"camera_state={snap.camera_state} flags=0x{snap.flags:02x}",
            category="protocol",
        ),
        _check_status(step_name, expect_status, int(rw["write_status"]), str(rw["write_status_name"])),
    ]
    if require_unchanged:
        after_hex = str(rw["payload_readback_hex"])
        checks.append(_check_config_unchanged(step_name, before_hex, after_hex))
    if run_error:
        checks.append(
            CheckResult(
                name=f"{step_name}.fixtureRun",
                passed=False,
                detail=run_error[0],
                category="protocol",
            )
        )

    _wait_for_idle_ble(session, timeout_s=20.0, run_id=run_id)
    log = {
        "step": step_name,
        "writeAtMs": write_at_ms,
        "activityActiveAtWrite": activity_active,
        "camera_rw": rw,
        "telemetryAtWrite": {
            "camera_state": snap.camera_state,
            "flags": snap.flags,
        },
    }
    return checks, log


def execute_camera_config_protocol(
    steps: list[dict[str, Any]],
    session: DutBleSession,
    *,
    fixture: FixtureClient | None = None,
    run_id: str = "unknown",
) -> tuple[list[CheckResult], list[dict[str, Any]]]:
    checks: list[CheckResult] = []
    log: list[dict[str, Any]] = []

    for index, step in enumerate(steps):
        step_name = f"protocol[{index}]"

        if "writeNamed" in step:
            spec = dict(step["writeNamed"])
            spec.setdefault("stepName", step_name)
            name = str(spec["stepName"])
            params = dict(spec.get("params", {}))
            expect_status = str(spec.get("expectStatus", "ACK_APPLIED"))
            before_hex = session.read_camera_config()[:CAMERA_CONFIG_LEN].hex()
            rw = session.write_camera_config_params(params, strict=False)
            checks.append(_check_status(name, expect_status, int(rw["write_status"]), str(rw["write_status_name"])))
            if spec.get("configUnchanged"):
                after_hex = str(rw["payload_readback_hex"])
                checks.append(_check_config_unchanged(name, before_hex, after_hex))
            log.append({"step": name, "type": "writeNamed", "camera_rw": rw})

        elif "writeRaw" in step:
            spec = dict(step["writeRaw"])
            spec.setdefault("stepName", step_name)
            name = str(spec["stepName"])
            expect_status = str(spec.get("expectStatus", "NACK_BAD_FORMAT"))
            before_hex = session.read_camera_config()[:CAMERA_CONFIG_LEN].hex()
            payload = _build_raw_payload(spec)
            transport_error: str | None = None
            rw: dict[str, Any]
            try:
                rw = session.write_camera_config_raw(payload)
            except Exception as exc:
                transport_error = str(exc)
                rw = {
                    "write_status": None,
                    "write_status_name": "TRANSPORT_ERROR",
                    "payload_written_hex": payload.hex(),
                    "payload_written_len": len(payload),
                    "payload_readback_hex": before_hex,
                }

            if transport_error and spec.get("allowTransportError"):
                checks.append(
                    CheckResult(
                        name=f"{name}.transportRejected",
                        passed=True,
                        detail=f"ATT/GATT rejected write before firmware: {transport_error}",
                        category="protocol",
                    )
                )
            elif transport_error:
                checks.append(
                    CheckResult(
                        name=f"{name}.writeStatus",
                        passed=False,
                        detail=f"transport error: {transport_error}",
                        category="protocol",
                    )
                )
            else:
                checks.append(
                    _check_status(
                        name,
                        expect_status,
                        int(rw["write_status"]),
                        str(rw["write_status_name"]),
                    )
                )
                if spec.get("configUnchanged"):
                    after_hex = str(rw["payload_readback_hex"])
                    checks.append(_check_config_unchanged(name, before_hex, after_hex))
            log.append({"step": name, "type": "writeRaw", "camera_rw": rw, "transport_error": transport_error})

        elif step.get("factoryReset"):
            session.factory_reset()
            time.sleep(0.6)
            _wait_for_idle_ble(session, timeout_s=10.0, run_id=run_id)
            readback = session.read_camera_config()
            log.append({"step": step_name, "type": "factoryReset", "readback_hex": readback[:CAMERA_CONFIG_LEN].hex()})

        elif "assertReadbackNamed" in step:
            spec = dict(step["assertReadbackNamed"])
            name = str(spec.get("stepName", step_name))
            readback = session.read_camera_config()
            checks.extend(_check_readback_named(name, readback, dict(spec.get("params", {}))))
            log.append({"step": name, "type": "assertReadbackNamed", "readback_hex": readback[:CAMERA_CONFIG_LEN].hex()})

        elif "writeNamedDuringActivity" in step:
            if fixture is None:
                raise RuntimeError("writeNamedDuringActivity requires fixture access")
            spec = dict(step["writeNamedDuringActivity"])
            spec.setdefault("stepName", step_name)
            step_checks, step_log = _write_named_during_activity(session, fixture, spec, run_id=run_id)
            checks.extend(step_checks)
            log.append(step_log)

            expect_after = spec.get("assertReadbackAfterIdle")
            if expect_after:
                readback = session.read_camera_config()
                checks.extend(_check_readback_named(str(spec["stepName"]), readback, dict(expect_after)))

        else:
            raise ValueError(f"unsupported camera config protocol step: {step!r}")

    return checks, log
