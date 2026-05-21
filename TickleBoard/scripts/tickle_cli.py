#!/usr/bin/env python3
"""TickleBoard validation framework CLI."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

from tickleboard.artifacts import write_json
from tickleboard.ble_adapter import DutBleAdapter
from tickleboard.constants import CAMERA_FIELDS
from tickleboard.fixture_client import FixtureClient
from tickleboard.reporting import write_csv_rollup, write_markdown_report
from tickleboard.runner import preflight, run_case, run_sc15_budget
from tickleboard.telemetry import format_snapshot, parse_payload
from tickleboard.vector_schema import load_suite, load_vector


def _resolve_port(port: str | None) -> str:
    resolved = port or FixtureClient.pick_default_port()
    if not resolved:
        raise RuntimeError("No Arduino Uno serial port found. Provide --port COMx.")
    return resolved


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
    rec = run_case(
        vector=v,
        fixture_port=_resolve_port(args.port),
        artifacts_root=args.artifacts,
        ble_address=args.ble,
        strict_camera_readback=not args.non_strict_camera,
    )
    print(json.dumps(rec, indent=2))
    return 0


def cmd_run_suite(args: argparse.Namespace) -> int:
    suite_paths = load_suite(args.suite)
    records: list[dict] = []
    fixture_port = _resolve_port(args.port)
    if args.ble:
        adapter = DutBleAdapter(args.ble)
        with adapter.open_session() as ble_session:
            for p in suite_paths:
                v = load_vector(p)
                rec = run_case(
                    vector=v,
                    fixture_port=fixture_port,
                    artifacts_root=args.artifacts,
                    ble_address=args.ble,
                    ble_session=ble_session,
                    strict_camera_readback=not args.non_strict_camera,
                )
                records.append(rec)
                print(f"{rec['case_id']} passed={rec['passed']} run_dir={rec['run_dir']}")
    else:
        for p in suite_paths:
            v = load_vector(p)
            rec = run_case(
                vector=v,
                fixture_port=fixture_port,
                artifacts_root=args.artifacts,
                ble_address=None,
                strict_camera_readback=not args.non_strict_camera,
            )
            records.append(rec)
            print(f"{rec['case_id']} passed={rec['passed']} run_dir={rec['run_dir']}")

    rollup = args.artifacts / "suite_rollup.json"
    write_json(rollup, records)
    write_markdown_report(args.artifacts / "suite_rollup.md", title="TickleBoard Suite Report", records=records)
    write_csv_rollup(args.artifacts / "suite_rollup.csv", records=records)
    return 0 if all(r["passed"] for r in records) else 3


def cmd_report(args: argparse.Namespace) -> int:
    data = json.loads(args.input.read_text(encoding="utf-8"))
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

    rsp = sub.add_parser("run-suite", help="Run suite YAML")
    rsp.add_argument("suite", type=Path)
    rsp.add_argument("--port", default=None)
    rsp.add_argument("--ble", default=None)
    rsp.add_argument("--non-strict-camera", action="store_true")

    rep = sub.add_parser("report", help="Render markdown+csv report from rollup JSON")
    rep.add_argument("input", type=Path)
    rep.add_argument("--output-md", type=Path, default=Path("TickleBoard") / "artifacts" / "report.md")
    rep.add_argument("--output-csv", type=Path, default=Path("TickleBoard") / "artifacts" / "report.csv")
    rep.add_argument("--title", default="TickleBoard Report")

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
    except Exception as exc:  # noqa: BLE001
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
