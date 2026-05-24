#!/usr/bin/env python3
"""TickleBoard validation framework CLI."""

from __future__ import annotations

import argparse
from datetime import datetime
import json
from pathlib import Path
import subprocess
import sys
import time

from tickleboard.artifacts import write_json, write_text
from tickleboard.ble_adapter import DutBleAdapter
from tickleboard.constants import CAMERA_FIELDS
from tickleboard.fixture_client import FixtureClient
from tickleboard.parameter_sweep import build_parameter_sweep_vectors
from tickleboard.reporting import write_csv_rollup, write_detailed_validation_report, write_markdown_report
from tickleboard.runner import classify_ble_requirement, preflight, run_case, run_sc15_budget
from tickleboard.telemetry import format_snapshot, parse_payload
from tickleboard.vector_schema import load_suite, load_vector

try:
    import serial
    import serial.tools.list_ports
except ImportError:  # pragma: no cover
    serial = None  # type: ignore[assignment]


def _print_suite_progress(
    case_id: str,
    idx: int,
    total: int,
    *,
    started: bool,
    status: str | None = None,
    passed: bool | None = None,
    run_dir: str | None = None,
    passed_count: int | None = None,
    failed_count: int | None = None,
    skipped_count: int | None = None,
) -> None:
    prefix = f"[{idx}/{total}] {case_id}"
    if started:
        print(f"{prefix}: started...", flush=True)
        return
    if status:
        outcome = status.upper()
    else:
        outcome = "PASSED" if passed else "FAILED"
    print(
        f"{prefix}: {outcome} run_dir={run_dir} "
        f"(totals: {passed_count} passed, {failed_count} failed, {skipped_count} skipped)",
        flush=True,
    )


def _resolve_port(port: str | None) -> str:
    resolved = port or FixtureClient.pick_default_port()
    if not resolved:
        raise RuntimeError("No Arduino Uno serial port found. Provide --port COMx.")
    return resolved


def _auto_pick_dut_serial_port(fixture_port: str | None) -> str | None:
    if serial is None:
        return None
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None

    fixture_lower = (fixture_port or "").lower()
    candidates = [p for p in ports if p.device.lower() != fixture_lower]
    if not candidates:
        return None

    preferred_tokens = ("xiao", "nrf", "seeed", "camtraptions", "usb serial")
    excluded_tokens = ("arduino uno", "ch340", "cp210", "usb-serial ch340")

    for p in candidates:
        desc = (p.description or "").lower()
        if any(token in desc for token in excluded_tokens):
            continue
        if any(token in desc for token in preferred_tokens):
            return p.device
    return candidates[0].device


def _resolve_dut_serial_port(explicit_port: str | None, fixture_port: str | None) -> tuple[str | None, str]:
    if explicit_port:
        return explicit_port, "explicit"
    auto = _auto_pick_dut_serial_port(fixture_port)
    if auto:
        return auto, "auto"
    return None, "none"


def _read_serial_lines(port: str, baud: int, timeout_s: float) -> list[str]:
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
    out: list[str] = []
    with serial.Serial(port, baudrate=baud, timeout=0.2) as ser:
        ser.reset_input_buffer()
        started = time.time()
        while (time.time() - started) < timeout_s:
            raw = ser.readline()
            if not raw:
                continue
            txt = raw.decode("utf-8", errors="replace").rstrip("\r\n")
            ts_ms = int((time.time() - started) * 1000)
            out.append(f"{ts_ms} {txt}")
    return out


def _print_dut_serial_selection(port: str | None, source: str) -> None:
    if source == "explicit" and port:
        print(f"DUT serial capture: using explicit port {port}", flush=True)
        return
    if source == "auto" and port:
        print(f"DUT serial capture: auto-selected port {port}", flush=True)
        return
    print("DUT serial capture: disabled (no port provided or detected)", flush=True)


def cmd_ports() -> int:
    for dev, desc in FixtureClient.list_ports():
        print(f"{dev}\t{desc}")
    return 0


def cmd_discover(args: argparse.Namespace) -> int:
    devices = DutBleAdapter.discover(timeout_s=args.timeout, name_filter=args.name_filter)
    for d in devices:
        print(f"{d.address}\t{d.name}")
    return 0


def cmd_preflight(args: argparse.Namespace) -> int:
    result = preflight(_resolve_port(args.port), ble_address=args.ble)
    print(json.dumps(result, indent=2))
    return 0


def cmd_run_case(args: argparse.Namespace) -> int:
    v = load_vector(args.vector)
    fixture_port = _resolve_port(args.port)
    dut_serial_port, dut_serial_source = _resolve_dut_serial_port(args.dut_serial_port, fixture_port)
    _print_dut_serial_selection(dut_serial_port, dut_serial_source)
    if not args.ble:
        requires_ble, reasons = classify_ble_requirement(v)
        if requires_ble:
            rec = {
                "run_dir": "",
                "case_id": v.case_id,
                "scenario": v.scenario,
                "status": "skipped",
                "skipped": True,
                "skip_reason": "; ".join(reasons),
                "passed": True,
                "failure_class": "skipped",
                "timing_passed": 0,
                "timing_total": 0,
                "functional_passed": 0,
                "functional_total": 0,
                "notes": ["skipped_no_ble"],
            }
            print(json.dumps(rec, indent=2))
            return 0
    rec = run_case(
        vector=v,
        fixture_port=fixture_port,
        artifacts_root=args.artifacts,
        ble_address=args.ble,
        strict_camera_readback=not args.non_strict_camera,
        dut_serial_port=dut_serial_port,
        dut_serial_baud=args.dut_serial_baud,
    )
    rec["status"] = "passed" if rec.get("passed") else "failed"
    rec["skipped"] = False
    rec["skip_reason"] = ""
    print(json.dumps(rec, indent=2))
    return 0


def cmd_run_suite(args: argparse.Namespace) -> int:
    suite_paths = load_suite(args.suite)
    records: list[dict] = []
    fixture_port = _resolve_port(args.port)
    dut_serial_port, dut_serial_source = _resolve_dut_serial_port(args.dut_serial_port, fixture_port)
    _print_dut_serial_selection(dut_serial_port, dut_serial_source)
    total = len(suite_paths)
    passed_count = 0
    failed_count = 0
    skipped_count = 0
    print(f"Starting suite with {total} case(s)...", flush=True)
    if args.ble:
        adapter = DutBleAdapter(args.ble)
        with adapter.open_session() as ble_session:
            for idx, p in enumerate(suite_paths, start=1):
                v = load_vector(p)
                _print_suite_progress(v.case_id, idx, total, started=True)
                rec = run_case(
                    vector=v,
                    fixture_port=fixture_port,
                    artifacts_root=args.artifacts,
                    ble_address=args.ble,
                    ble_session=ble_session,
                    strict_camera_readback=not args.non_strict_camera,
                    dut_serial_port=dut_serial_port,
                    dut_serial_baud=args.dut_serial_baud,
                )
                rec["status"] = "passed" if rec.get("passed") else "failed"
                rec["skipped"] = False
                rec["skip_reason"] = ""
                records.append(rec)
                if rec["status"] == "passed":
                    passed_count += 1
                else:
                    failed_count += 1
                _print_suite_progress(
                    rec["case_id"],
                    idx,
                    total,
                    started=False,
                    status=rec["status"],
                    passed=bool(rec["passed"]),
                    run_dir=str(rec["run_dir"]),
                    passed_count=passed_count,
                    failed_count=failed_count,
                    skipped_count=skipped_count,
                )
    else:
        for idx, p in enumerate(suite_paths, start=1):
            v = load_vector(p)
            _print_suite_progress(v.case_id, idx, total, started=True)
            requires_ble, reasons = classify_ble_requirement(v)
            if requires_ble:
                rec = {
                    "run_dir": "",
                    "case_id": v.case_id,
                    "scenario": v.scenario,
                    "status": "skipped",
                    "skipped": True,
                    "skip_reason": "; ".join(reasons),
                    "passed": True,
                    "failure_class": "skipped",
                    "timing_passed": 0,
                    "timing_total": 0,
                    "functional_passed": 0,
                    "functional_total": 0,
                    "notes": ["skipped_no_ble"] + reasons,
                }
                records.append(rec)
                skipped_count += 1
                _print_suite_progress(
                    rec["case_id"],
                    idx,
                    total,
                    started=False,
                    status=rec["status"],
                    passed=True,
                    run_dir=str(rec["run_dir"]),
                    passed_count=passed_count,
                    failed_count=failed_count,
                    skipped_count=skipped_count,
                )
                continue
            rec = run_case(
                vector=v,
                fixture_port=fixture_port,
                artifacts_root=args.artifacts,
                ble_address=None,
                strict_camera_readback=not args.non_strict_camera,
                dut_serial_port=dut_serial_port,
                dut_serial_baud=args.dut_serial_baud,
            )
            rec["status"] = "passed" if rec.get("passed") else "failed"
            rec["skipped"] = False
            rec["skip_reason"] = ""
            records.append(rec)
            if rec["status"] == "passed":
                passed_count += 1
            else:
                failed_count += 1
            _print_suite_progress(
                rec["case_id"],
                idx,
                total,
                started=False,
                status=rec["status"],
                passed=bool(rec["passed"]),
                run_dir=str(rec["run_dir"]),
                passed_count=passed_count,
                failed_count=failed_count,
                skipped_count=skipped_count,
            )

    rollup = args.artifacts / "suite_rollup.json"
    write_json(rollup, records)
    write_markdown_report(args.artifacts / "suite_rollup.md", title="TickleBoard Suite Report", records=records)
    write_csv_rollup(args.artifacts / "suite_rollup.csv", records=records)
    print(
        f"Suite complete: {passed_count} passed, {failed_count} failed, {skipped_count} skipped, {total} total. "
        f"Rollup: {rollup}",
        flush=True,
    )
    return 0 if failed_count == 0 else 3


def cmd_report(args: argparse.Namespace) -> int:
    data = json.loads(args.input.read_text(encoding="utf-8"))
    if args.detailed:
        write_detailed_validation_report(
            args.output_md,
            title=args.title,
            records=data,
            suite_path=args.suite_path,
            artifacts_root=args.artifacts_root,
            run_label=args.run_label,
        )
    else:
        write_markdown_report(args.output_md, title=args.title, records=data)
    write_csv_rollup(args.output_csv, records=data)
    return 0


def cmd_resume(args: argparse.Namespace) -> int:
    data = json.loads(args.rollup.read_text(encoding="utf-8"))
    failed = [d for d in data if not d.get("passed", False)]
    if not failed:
        print("No failed cases to resume.")
        return 0
    print(f"Failed cases ({len(failed)}):")
    for row in failed:
        print(f"- {row.get('case_id')} ({row.get('scenario')})")
    return 4


def cmd_run_sc15(args: argparse.Namespace) -> int:
    v = load_vector(args.vector)
    if not args.ble:
        raise RuntimeError("--ble is required for SC-15 power-save budget run")
    out = run_sc15_budget(
        base_vector=v,
        fixture_port=_resolve_port(args.port),
        ble_address=args.ble,
        artifacts_root=args.artifacts,
        repeats=args.repeats,
    )
    print(json.dumps(out["delta_check"], indent=2))
    return 0 if out["delta_check"]["passed"] else 5


def cmd_ble_smoke(args: argparse.Namespace) -> int:
    devices = DutBleAdapter.discover(timeout_s=args.timeout, name_filter=args.name_filter)
    selected_address = args.ble
    selected_name = ""
    if not selected_address:
        if not devices:
            raise RuntimeError("No BLE devices found for smoke test.")
        selected_address = devices[0].address
        selected_name = devices[0].name
    else:
        for d in devices:
            if d.address.lower() == selected_address.lower():
                selected_name = d.name
                break

    adapter = DutBleAdapter(selected_address)
    before = adapter.read_camera_config()

    field = CAMERA_FIELDS[args.parameter]
    old = int(before[field.index])
    if args.value is not None:
        target = int(args.value)
    else:
        target = old + 1 if old < field.max_value else old - 1
        target = max(field.min_value, min(field.max_value, target))

    rw = adapter.write_camera_config_params({args.parameter: target}, strict=True)
    after = adapter.read_camera_config()
    new_val = int(after[field.index])

    adapter.write_camera_config_params({args.parameter: old}, strict=True)
    restored = adapter.read_camera_config()
    restored_val = int(restored[field.index])

    report = {
        "beacon_discovery_ok": len(devices) > 0,
        "discovered_count": len(devices),
        "selected_address": selected_address,
        "selected_name": selected_name,
        "parameter": args.parameter,
        "read_before": old,
        "write_requested": target,
        "read_after_write": new_val,
        "restore_to": old,
        "read_after_restore": restored_val,
        "write_readback_match": rw["mismatches"] == {},
        "smoke_passed": (new_val == target and restored_val == old and rw["mismatches"] == {}),
    }
    print(json.dumps(report, indent=2))
    return 0 if report["smoke_passed"] else 6


def _format_beacon_report(beacon: dict[str, object] | None) -> str:
    if beacon is None:
        return "Beacon Snapshot\n---------------\nNo matching beacon payload captured."

    lines: list[str] = []
    lines.append("Beacon Snapshot")
    lines.append("---------------")
    lines.append(f"Address: {beacon.get('address', '')}")
    lines.append(f"Name: {beacon.get('name', '')}")
    lines.append(f"Company ID: {beacon.get('companyId', '')}")
    lines.append(f"Manufacturer Length: {beacon.get('manufacturerLen', 0)}")
    lines.append(f"Manufacturer Hex: {beacon.get('manufacturerHex', '')}")
    mbytes = beacon.get("manufacturerBytes")
    if isinstance(mbytes, list):
        lines.append(f"Manufacturer Bytes: {' '.join(f'{int(v):02X}' for v in mbytes)}")

    lines.append("Decoded Fields:")
    decoded_keys = [
        "internalBatteryPercent",
        "internalVoltageMv",
        "externalBatteryPercent",
        "externalVoltageMv",
        "legacyFlags",
        "configured",
        "deviceType",
        "chemistry",
        "groupId",
        "cellCount",
        "shutterCount",
        "beaconLayoutVersion",
        "cameraStateBeacon",
        "cameraFlagsBeacon",
        "activityActiveBeacon",
        "hpOutAssertedBeacon",
        "buildTimestamp",
    ]
    for key in decoded_keys:
        if key in beacon:
            lines.append(f"  {key}: {beacon[key]}")
    return "\n".join(lines)


def cmd_ble_telemetry_smoke(args: argparse.Namespace) -> int:
    devices = DutBleAdapter.discover(timeout_s=args.timeout, name_filter=args.name_filter)
    selected_address = args.ble
    selected_name = ""
    if not selected_address:
        if not devices:
            raise RuntimeError("No BLE devices found for telemetry smoke test.")
        selected_address = devices[0].address
        selected_name = devices[0].name
    else:
        for d in devices:
            if d.address.lower() == selected_address.lower():
                selected_name = d.name
                break

    adapter = DutBleAdapter(selected_address)
    beacon = adapter.read_beacon_snapshot(timeout_s=args.timeout)
    payload = adapter.read_telemetry_payload()
    snapshot = parse_payload(payload)

    print("BLE Telemetry Smoke Test")
    print("========================")
    print(f"Discovery matched devices: {len(devices)}")
    print(f"Selected address: {selected_address}")
    print(f"Selected name: {selected_name}")
    print("")
    print(_format_beacon_report(beacon))
    print("")
    print(f"Telemetry Payload Length: {len(payload)}")
    print(f"Telemetry Payload Hex: {payload.hex()}")
    print("")
    print(format_snapshot(snapshot))
    return 0


def cmd_gen_parameter_sweep(args: argparse.Namespace) -> int:
    out = build_parameter_sweep_vectors(args.vectors_root)
    print(json.dumps(out, indent=2))
    return 0


def cmd_flash_dut(args: argparse.Namespace) -> int:
    sketch_path = args.sketch.resolve()
    if not sketch_path.exists():
        raise RuntimeError(f"Sketch path not found: {sketch_path}")
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")

    flash_port = args.port or _auto_pick_dut_serial_port(fixture_port=None)
    if not flash_port:
        raise RuntimeError("No DUT serial port found. Provide --port COMx.")

    command = [
        args.arduino_cli,
        "compile",
        "--upload",
        "--fqbn",
        args.fqbn,
        str(sketch_path),
        "--port",
        flash_port,
    ]
    print(f"Flashing with command: {' '.join(command)}", flush=True)
    started = time.time()
    proc = subprocess.run(command, capture_output=True, text=True, check=False)
    elapsed_ms = int((time.time() - started) * 1000)

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    flash_dir = args.artifacts / f"{stamp}_FLASH_DUT"
    flash_dir.mkdir(parents=True, exist_ok=True)
    write_text(flash_dir / "upload_stdout.log", proc.stdout or "")
    write_text(flash_dir / "upload_stderr.log", proc.stderr or "")

    if proc.returncode != 0:
        write_json(
            flash_dir / "flash_result.json",
            {
                "ok": False,
                "returncode": proc.returncode,
                "port": flash_port,
                "fqbn": args.fqbn,
                "sketch": str(sketch_path),
                "elapsed_ms": elapsed_ms,
                "boot_capture_lines": 0,
                "boot_banner_lines": [],
            },
        )
        raise RuntimeError(f"Flash failed (exit {proc.returncode}). See {flash_dir / 'upload_stderr.log'}")

    time.sleep(args.boot_capture_delay_s)
    serial_lines = _read_serial_lines(flash_port, args.dut_serial_baud, args.boot_capture_seconds)
    write_text(
        flash_dir / "post_flash_serial.log",
        "\n".join(serial_lines) + ("\n" if serial_lines else ""),
    )
    boot_lines = [line for line in serial_lines if "[BOOT]" in line]
    write_json(
        flash_dir / "flash_result.json",
        {
            "ok": True,
            "returncode": proc.returncode,
            "port": flash_port,
            "fqbn": args.fqbn,
            "sketch": str(sketch_path),
            "elapsed_ms": elapsed_ms,
            "boot_capture_lines": len(serial_lines),
            "boot_banner_lines": boot_lines,
        },
    )
    print(
        json.dumps(
            {
                "ok": True,
                "port": flash_port,
                "flash_artifacts": str(flash_dir),
                "boot_banner_lines": boot_lines,
            },
            indent=2,
        )
    )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="TickleBoard validation framework")
    parser.add_argument(
        "--artifacts",
        type=Path,
        default=Path("TickleBoard") / "artifacts",
        help="Artifact output root",
    )
    sub = parser.add_subparsers(
        dest="cmd",
        required=True,
    )

    sub.add_parser("ports", help="List serial ports")

    dp = sub.add_parser("discover", help="Discover DUT BLE devices")
    dp.add_argument("--timeout", type=float, default=6.0)
    dp.add_argument("--name-filter", default="Camtraptions")

    pfp = sub.add_parser("preflight", help="Run fixture and optional BLE preflight checks")
    pfp.add_argument("--port", default=None)
    pfp.add_argument("--ble", default=None, help="BLE address")

    rcp = sub.add_parser("run-case", help="Run one case vector")
    rcp.add_argument("vector", type=Path)
    rcp.add_argument("--port", default=None)
    rcp.add_argument("--ble", default=None)
    rcp.add_argument("--non-strict-camera", action="store_true", help="Allow readback mismatches")
    rcp.add_argument("--dut-serial-port", default=None, help="Optional DUT USB serial port for debug log capture")
    rcp.add_argument("--dut-serial-baud", type=int, default=115200, help="Baud rate for DUT serial capture")

    rsp = sub.add_parser("run-suite", help="Run suite YAML")
    rsp.add_argument("suite", type=Path)
    rsp.add_argument("--port", default=None)
    rsp.add_argument("--ble", default=None)
    rsp.add_argument("--non-strict-camera", action="store_true")
    rsp.add_argument("--dut-serial-port", default=None, help="Optional DUT USB serial port for debug log capture")
    rsp.add_argument("--dut-serial-baud", type=int, default=115200, help="Baud rate for DUT serial capture")

    rep = sub.add_parser("report", help="Render markdown+csv report from rollup JSON")
    rep.add_argument("input", type=Path)
    rep.add_argument("--output-md", type=Path, default=Path("TickleBoard") / "artifacts" / "report.md")
    rep.add_argument("--output-csv", type=Path, default=Path("TickleBoard") / "artifacts" / "report.csv")
    rep.add_argument("--title", default="TickleBoard Report")
    rep.add_argument(
        "--detailed",
        action="store_true",
        help="Emit detailed validation report (Executive Summary + per-case assertion details)",
    )
    rep.add_argument(
        "--suite-path",
        default="TickleBoard/vectors/suites/full_validation_suite.yaml",
        help="Suite YAML path shown in detailed report header",
    )
    rep.add_argument(
        "--artifacts-root",
        default="TickleBoard/artifacts",
        help="Artifacts root shown in detailed report header",
    )
    rep.add_argument(
        "--run-label",
        default=None,
        help="Optional run batch label for detailed report header",
    )

    rspm = sub.add_parser("resume", help="List failed cases from a suite rollup")
    rspm.add_argument("rollup", type=Path, help="Path to suite_rollup.json")

    sc15 = sub.add_parser("run-sc15", help="Run SC-15 enabled vs disabled repeated budget check")
    sc15.add_argument("vector", type=Path, help="SC-15 vector file")
    sc15.add_argument("--port", default=None)
    sc15.add_argument("--ble", default=None)
    sc15.add_argument("--repeats", type=int, default=20)

    bsmoke = sub.add_parser("ble-smoke", help="Run BLE smoke test: discover/read/write/readback/restore")
    bsmoke.add_argument("--ble", default=None, help="BLE address (optional; auto-picks first discovered)")
    bsmoke.add_argument("--timeout", type=float, default=8.0)
    bsmoke.add_argument("--name-filter", default="Tickle")
    bsmoke.add_argument("--parameter", default="FrameCount", help="Parameter name to write/readback")
    bsmoke.add_argument("--value", type=int, default=None, help="Optional explicit test value")

    tsmoke = sub.add_parser(
        "ble-telemetry-smoke",
        help="Run BLE telemetry smoke: read beacon + telemetry and print decoded details",
    )
    tsmoke.add_argument("--ble", default=None, help="BLE address (optional; auto-picks first discovered)")
    tsmoke.add_argument("--timeout", type=float, default=8.0)
    tsmoke.add_argument("--name-filter", default="Tickle")

    gps = sub.add_parser(
        "gen-parameter-sweep",
        help="Generate parameter sweep vectors/suite for timing + cap/gap interactions",
    )
    gps.add_argument(
        "--vectors-root",
        type=Path,
        default=Path("TickleBoard") / "vectors",
        help="Vectors root directory (default: TickleBoard/vectors)",
    )

    flash = sub.add_parser(
        "flash-dut",
        help="Flash DUT firmware and capture post-flash boot serial banner",
    )
    flash.add_argument(
        "--sketch",
        type=Path,
        default=Path("Camtraptions_Firmware"),
        help="Path to Arduino sketch folder",
    )
    flash.add_argument("--port", default=None, help="DUT serial port (auto-detected if omitted)")
    flash.add_argument(
        "--fqbn",
        default="Seeeduino:nrf52:xiaonRF52840",
        help="Arduino FQBN for DUT firmware",
    )
    flash.add_argument(
        "--arduino-cli",
        default="arduino-cli",
        help="arduino-cli executable path",
    )
    flash.add_argument("--dut-serial-baud", type=int, default=115200, help="Baud rate for post-flash serial capture")
    flash.add_argument(
        "--boot-capture-seconds",
        type=float,
        default=3.0,
        help="Seconds to capture serial after flash",
    )
    flash.add_argument(
        "--boot-capture-delay-s",
        type=float,
        default=0.4,
        help="Delay before starting post-flash serial capture",
    )

    args = parser.parse_args()
    args.artifacts.mkdir(parents=True, exist_ok=True)

    try:
        if args.cmd == "ports":
            return cmd_ports()
        if args.cmd == "discover":
            return cmd_discover(args)
        if args.cmd == "preflight":
            return cmd_preflight(args)
        if args.cmd == "run-case":
            return cmd_run_case(args)
        if args.cmd == "run-suite":
            return cmd_run_suite(args)
        if args.cmd == "report":
            return cmd_report(args)
        if args.cmd == "resume":
            return cmd_resume(args)
        if args.cmd == "run-sc15":
            return cmd_run_sc15(args)
        if args.cmd == "ble-smoke":
            return cmd_ble_smoke(args)
        if args.cmd == "ble-telemetry-smoke":
            return cmd_ble_telemetry_smoke(args)
        if args.cmd == "gen-parameter-sweep":
            return cmd_gen_parameter_sweep(args)
        if args.cmd == "flash-dut":
            return cmd_flash_dut(args)
    except Exception as exc:  # noqa: BLE001
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
