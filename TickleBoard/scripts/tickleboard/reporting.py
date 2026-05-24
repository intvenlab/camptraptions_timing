from __future__ import annotations

from pathlib import Path
import csv
import json


def _record_status(row: dict) -> str:
    status = str(row.get("status", "")).strip().lower()
    if status in {"passed", "failed", "skipped"}:
        return status
    if bool(row.get("skipped", False)):
        return "skipped"
    return "passed" if bool(row.get("passed", False)) else "failed"


def write_markdown_report(path: Path, title: str, records: list[dict]) -> None:
    timing_passed = sum(int(r.get("timing_passed", 0)) for r in records)
    timing_total = sum(int(r.get("timing_total", 0)) for r in records)
    functional_passed = sum(int(r.get("functional_passed", 0)) for r in records)
    functional_total = sum(int(r.get("functional_total", 0)) for r in records)
    passed_cases = sum(1 for r in records if _record_status(r) == "passed")
    failed_cases = sum(1 for r in records if _record_status(r) == "failed")
    skipped_cases = sum(1 for r in records if _record_status(r) == "skipped")
    lines: list[str] = [
        f"# {title}",
        "",
        f"- Cases: {passed_cases} passed, {failed_cases} failed, {skipped_cases} skipped, {len(records)} total",
        f"- Timing assertions: {timing_passed}/{timing_total}",
        f"- Functional assertions: {functional_passed}/{functional_total}",
        "",
    ]

    hp_anomaly_rows: list[tuple[str, str]] = []
    for r in records:
        metrics = r.get("metrics", {}) if isinstance(r.get("metrics", {}), dict) else {}
        details: list[str] = []
        if metrics.get("hpReleaseBeforeFinalFrameDetected") is True:
            details.append("HP_OUT released before final FP_OUT release")
        if metrics.get("hpOutPreAssertedBeforeHpIn") is True:
            details.append("HP_OUT pre-asserted before HP_IN (latency unmeasurable)")
        if details:
            hp_anomaly_rows.append((str(r.get("case_id", "")), "; ".join(details)))

    if hp_anomaly_rows:
        lines.extend(
            [
                "## HP Timing Anomalies",
                "",
                "| Case | Detail |",
                "|------|--------|",
            ]
        )
        for case_id, detail in hp_anomaly_rows:
            lines.append(f"| {case_id} | {detail} |")
        lines.append("")

    lines.extend(
        [
        "| Case | Scenario | Status | Failure Class | Timing | Functional | Skip Reason | Notes |",
        "|------|----------|--------|---------------|--------|------------|-------------|-------|",
        ]
    )
    for r in records:
        status = _record_status(r)
        notes = "; ".join(r.get("notes", []))
        skip_reason = str(r.get("skip_reason", ""))
        lines.append(
            f"| {r.get('case_id','')} | {r.get('scenario','')} | {status} | "
            f"{r.get('failure_class','')} | {r.get('timing_passed',0)}/{r.get('timing_total',0)} | "
            f"{r.get('functional_passed',0)}/{r.get('functional_total',0)} | {skip_reason} | {notes} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_csv_rollup(path: Path, records: list[dict]) -> None:
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "case_id",
                "scenario",
                "status",
                "skipped",
                "skip_reason",
                "passed",
                "failure_class",
                "timing_passed",
                "timing_total",
                "functional_passed",
                "functional_total",
                "run_dir",
            ],
        )
        writer.writeheader()
        for r in records:
            writer.writerow(
                {
                    "case_id": r.get("case_id", ""),
                    "scenario": r.get("scenario", ""),
                    "status": _record_status(r),
                    "skipped": bool(r.get("skipped", False)),
                    "skip_reason": r.get("skip_reason", ""),
                    "passed": r.get("passed", False),
                    "failure_class": r.get("failure_class", ""),
                    "timing_passed": r.get("timing_passed", 0),
                    "timing_total": r.get("timing_total", 0),
                    "functional_passed": r.get("functional_passed", 0),
                    "functional_total": r.get("functional_total", 0),
                    "run_dir": r.get("run_dir", ""),
                }
            )


_KEY_METRIC_FIELDS: tuple[str, ...] = (
    "frameCount",
    "sequenceCount",
    "hpInToHpOutLatencyMs",
    "fpInToFpOutLatencyMs",
    "fpPulseWidthMs",
    "frameEndToStartSpacingMs",
    "frameStartSpacingMs",
    "firstFrameGateDelayMs",
    "firstFrameAfLeadMs",
    "hpOutContinuityMs",
    "hpHoldAfterLastFrameMs",
    "wakeOnlyHoldMs",
    "interSequenceGapMs",
    "secondSequenceStartDelayMs",
    "ignoredFpCount",
    "hpAssertToFinalFrameReleaseMs",
)


def _format_metric_value(value: object) -> str | None:
    if value is None:
        return None
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, float):
        return f"{value:.1f}"
    return str(value)


def _load_case_payload(record: dict) -> dict:
    run_dir = record.get("run_dir")
    if not run_dir:
        return {}
    result_path = Path(str(run_dir)) / "result.json"
    if not result_path.is_file():
        return {}
    return json.loads(result_path.read_text(encoding="utf-8"))


def _key_metrics_line(metrics: dict) -> str:
    parts: list[str] = []
    for name in _KEY_METRIC_FIELDS:
        if name not in metrics:
            continue
        formatted = _format_metric_value(metrics.get(name))
        if formatted is None:
            continue
        parts.append(f"{name}={formatted}")
    return "; ".join(parts)


def write_detailed_validation_report(
    path: Path,
    title: str,
    records: list[dict],
    *,
    suite_path: str = "TickleBoard/vectors/suites/full_validation_suite.yaml",
    artifacts_root: str = "TickleBoard/artifacts",
    run_label: str | None = None,
) -> None:
    payloads = [_load_case_payload(record) for record in records]

    timing_passed = sum(
        int(p.get("check_summary", {}).get("timing_passed", record.get("timing_passed", 0)))
        for p, record in zip(payloads, records, strict=True)
    )
    timing_total = sum(
        int(p.get("check_summary", {}).get("timing_total", record.get("timing_total", 0)))
        for p, record in zip(payloads, records, strict=True)
    )
    functional_passed = sum(
        int(p.get("check_summary", {}).get("functional_passed", record.get("functional_passed", 0)))
        for p, record in zip(payloads, records, strict=True)
    )
    functional_total = sum(
        int(p.get("check_summary", {}).get("functional_total", record.get("functional_total", 0)))
        for p, record in zip(payloads, records, strict=True)
    )
    protocol_passed = sum(int(p.get("check_summary", {}).get("protocol_passed", 0)) for p in payloads)
    protocol_total = sum(int(p.get("check_summary", {}).get("protocol_total", 0)) for p in payloads)

    passed_cases = sum(1 for record in records if _record_status(record) == "passed")
    failed_cases = sum(1 for record in records if _record_status(record) == "failed")
    skipped_cases = sum(1 for record in records if _record_status(record) == "skipped")
    total_cases = len(records)

    if run_label is None and records:
        run_dir = str(records[0].get("run_dir", ""))
        run_label = Path(run_dir).name.split("_", 2)[0] if run_dir else "latest"

    lines: list[str] = [
        f"# {title}",
        "",
        "## Executive Summary",
        "",
        "This report captures results from the latest **BLE-enabled authoritative full-suite run**.",
        "",
        f"- Suite: `{suite_path}`",
        f"- Artifacts root: `{artifacts_root}`",
        f"- Run batch: `{run_label}`",
        f"- Cases executed: {total_cases}",
        f"- Outcome: **{passed_cases} passed, {failed_cases} failed, {skipped_cases} skipped**",
        f"- Timing assertions: **{timing_passed}/{timing_total} passed**",
        f"- Functional assertions: **{functional_passed}/{functional_total} passed**",
    ]
    if protocol_total:
        lines.append(f"- Protocol assertions: **{protocol_passed}/{protocol_total} passed**")
    lines.extend(["", "## Master Results Table", ""])
    lines.extend(
        [
            "| Case | Scenario | Status | Failure Class | Timing | Functional | Notes |",
            "|------|----------|--------|---------------|--------|------------|-------|",
        ]
    )
    for record in records:
        status = _record_status(record)
        notes = "; ".join(record.get("notes", []))
        lines.append(
            f"| {record.get('case_id', '')} | {record.get('scenario', '')} | {status} | "
            f"{record.get('failure_class', '')} | {record.get('timing_passed', 0)}/{record.get('timing_total', 0)} | "
            f"{record.get('functional_passed', 0)}/{record.get('functional_total', 0)} | {notes} |"
        )

    lines.extend(
        [
            "",
            "## Results Statistics",
            "",
            f"- Total cases: {total_cases}",
            f"- Passed: {passed_cases}",
            f"- Failed: {failed_cases}",
            f"- Skipped: {skipped_cases}",
            f"- Timing assertions passed: {timing_passed}/{timing_total}",
            f"- Functional assertions passed: {functional_passed}/{functional_total}",
        ]
    )
    if protocol_total:
        lines.append(f"- Protocol assertions passed: {protocol_passed}/{protocol_total}")
    lines.extend(["", "## Detailed Test Results", ""])

    for record, payload in zip(records, payloads, strict=True):
        case_id = str(record.get("case_id", ""))
        scenario = str(record.get("scenario", ""))
        status = _record_status(record)
        failure_class = str(record.get("failure_class", ""))
        description = str(payload.get("description", ""))
        run_dir = str(record.get("run_dir", ""))
        notes = record.get("notes", [])
        check_summary = payload.get("check_summary", {})
        timing_line = (
            f"{check_summary.get('timing_passed', record.get('timing_passed', 0))}/"
            f"{check_summary.get('timing_total', record.get('timing_total', 0))}"
        )
        functional_line = (
            f"{check_summary.get('functional_passed', record.get('functional_passed', 0))}/"
            f"{check_summary.get('functional_total', record.get('functional_total', 0))}"
        )
        protocol_line = f"{check_summary.get('protocol_passed', 0)}/{check_summary.get('protocol_total', 0)}"
        metrics = payload.get("metrics", {}) if isinstance(payload.get("metrics"), dict) else {}
        key_metrics = _key_metrics_line(metrics)

        lines.append(f"### {case_id} ({scenario})")
        lines.append("")
        lines.append(f"- Status: {status}")
        lines.append(f"- Failure class: {failure_class}")
        lines.append(f"- Timing assertions: {timing_line}")
        lines.append(f"- Functional assertions: {functional_line}")
        if int(check_summary.get("protocol_total", 0)) > 0:
            lines.append(f"- Protocol assertions: {protocol_line}")
        lines.append(f"- Artifact path: `{run_dir}`")
        if description:
            lines.append(f"- Description: {description}")
        if notes:
            lines.append(f"- Notes: {'; '.join(str(n) for n in notes)}")
        if key_metrics:
            lines.append(f"- Key metrics: {key_metrics}")

        checks = payload.get("checks", [])
        if isinstance(checks, list) and checks:
            lines.append("- Assertion details:")
            for check in checks:
                if not isinstance(check, dict):
                    continue
                name = str(check.get("name", ""))
                category = str(check.get("category", "functional"))
                detail = str(check.get("detail", ""))
                passed = bool(check.get("passed", False))
                mark = "PASS" if passed else "FAIL"
                lines.append(f"  - [{mark}] `{name}` ({category}) - {detail}")
        lines.append("")

    lines.append("## Conclusion")
    lines.append("")
    if failed_cases == 0 and skipped_cases == 0:
        lines.append(
            "All BLE-enabled full-suite cases passed with complete timing, functional, and protocol assertion coverage."
        )
    else:
        lines.append(
            f"Suite completed with {passed_cases} passed, {failed_cases} failed, and {skipped_cases} skipped case(s)."
        )
    lines.append("")

    path.write_text("\n".join(lines), encoding="utf-8")

_KEY_METRIC_FIELDS: tuple[str, ...] = (
    "frameCount",
    "sequenceCount",
    "hpInToHpOutLatencyMs",
    "fpInToFpOutLatencyMs",
    "fpPulseWidthMs",
    "frameEndToStartSpacingMs",
    "frameStartSpacingMs",
    "firstFrameGateDelayMs",
    "firstFrameAfLeadMs",
    "hpOutContinuityMs",
    "hpHoldAfterLastFrameMs",
    "wakeOnlyHoldMs",
    "interSequenceGapMs",
    "secondSequenceStartDelayMs",
    "ignoredFpCount",
    "hpAssertToFinalFrameReleaseMs",
)


def _format_metric_value(value: object) -> str | None:
    if value is None:
        return None
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, float):
        return f"{value:.1f}"
    return str(value)


def _load_case_payload(record: dict) -> dict:
    run_dir = record.get("run_dir")
    if not run_dir:
        return {}
    result_path = Path(str(run_dir)) / "result.json"
    if not result_path.is_file():
        return {}
    return json.loads(result_path.read_text(encoding="utf-8"))


def _key_metrics_line(metrics: dict) -> str:
    parts: list[str] = []
    for name in _KEY_METRIC_FIELDS:
        if name not in metrics:
            continue
        formatted = _format_metric_value(metrics.get(name))
        if formatted is None:
            continue
        parts.append(f"{name}={formatted}")
    return "; ".join(parts)


def write_detailed_validation_report(
    path: Path,
    title: str,
    records: list[dict],
    *,
    suite_path: str = "TickleBoard/vectors/suites/full_validation_suite.yaml",
    artifacts_root: str = "TickleBoard/artifacts",
    run_label: str | None = None,
) -> None:
    payloads = [_load_case_payload(record) for record in records]

    timing_passed = sum(int(p.get("check_summary", {}).get("timing_passed", record.get("timing_passed", 0))) for p, record in zip(payloads, records, strict=True))
    timing_total = sum(int(p.get("check_summary", {}).get("timing_total", record.get("timing_total", 0))) for p, record in zip(payloads, records, strict=True))
    functional_passed = sum(int(p.get("check_summary", {}).get("functional_passed", record.get("functional_passed", 0))) for p, record in zip(payloads, records, strict=True))
    functional_total = sum(int(p.get("check_summary", {}).get("functional_total", record.get("functional_total", 0))) for p, record in zip(payloads, records, strict=True))
    protocol_passed = sum(int(p.get("check_summary", {}).get("protocol_passed", 0)) for p in payloads)
    protocol_total = sum(int(p.get("check_summary", {}).get("protocol_total", 0)) for p in payloads)

    passed_cases = sum(1 for record in records if _record_status(record) == "passed")
    failed_cases = sum(1 for record in records if _record_status(record) == "failed")
    skipped_cases = sum(1 for record in records if _record_status(record) == "skipped")
    total_cases = len(records)

    if run_label is None and records:
        run_dir = str(records[0].get("run_dir", ""))
        run_label = Path(run_dir).name.split("_", 2)[0] if run_dir else "latest"

    lines: list[str] = [
        f"# {title}",
        "",
        "## Executive Summary",
        "",
        "This report captures results from the latest **BLE-enabled authoritative full-suite run**.",
        "",
        f"- Suite: `{suite_path}`",
        f"- Artifacts root: `{artifacts_root}`",
        f"- Run batch: `{run_label}`",
        f"- Cases executed: {total_cases}",
        f"- Outcome: **{passed_cases} passed, {failed_cases} failed, {skipped_cases} skipped**",
        f"- Timing assertions: **{timing_passed}/{timing_total} passed**",
        f"- Functional assertions: **{functional_passed}/{functional_total} passed**",
    ]
    if protocol_total:
        lines.append(f"- Protocol assertions: **{protocol_passed}/{protocol_total} passed**")
    lines.extend(["", "## Master Results Table", ""])
    lines.extend(
        [
            "| Case | Scenario | Status | Failure Class | Timing | Functional | Notes |",
            "|------|----------|--------|---------------|--------|------------|-------|",
        ]
    )
    for record in records:
        status = _record_status(record)
        notes = "; ".join(record.get("notes", []))
        lines.append(
            f"| {record.get('case_id', '')} | {record.get('scenario', '')} | {status} | "
            f"{record.get('failure_class', '')} | {record.get('timing_passed', 0)}/{record.get('timing_total', 0)} | "
            f"{record.get('functional_passed', 0)}/{record.get('functional_total', 0)} | {notes} |"
        )

    lines.extend(
        [
            "",
            "## Results Statistics",
            "",
            f"- Total cases: {total_cases}",
            f"- Passed: {passed_cases}",
            f"- Failed: {failed_cases}",
            f"- Skipped: {skipped_cases}",
            f"- Timing assertions passed: {timing_passed}/{timing_total}",
            f"- Functional assertions passed: {functional_passed}/{functional_total}",
        ]
    )
    if protocol_total:
        lines.append(f"- Protocol assertions passed: {protocol_passed}/{protocol_total}")
    lines.extend(["", "## Detailed Test Results", ""])

    for record, payload in zip(records, payloads, strict=True):
        case_id = str(record.get("case_id", ""))
        scenario = str(record.get("scenario", ""))
        status = _record_status(record)
        failure_class = str(record.get("failure_class", ""))
        description = str(payload.get("description", ""))
        run_dir = str(record.get("run_dir", ""))
        notes = record.get("notes", [])
        check_summary = payload.get("check_summary", {})
        timing_line = f"{check_summary.get('timing_passed', record.get('timing_passed', 0))}/{check_summary.get('timing_total', record.get('timing_total', 0))}"
        functional_line = f"{check_summary.get('functional_passed', record.get('functional_passed', 0))}/{check_summary.get('functional_total', record.get('functional_total', 0))}"
        protocol_line = f"{check_summary.get('protocol_passed', 0)}/{check_summary.get('protocol_total', 0)}"
        metrics = payload.get("metrics", {}) if isinstance(payload.get("metrics"), dict) else {}
        key_metrics = _key_metrics_line(metrics)

        lines.append(f"### {case_id} ({scenario})")
        lines.append("")
        lines.append(f"- Status: {status}")
        lines.append(f"- Failure class: {failure_class}")
        lines.append(f"- Timing assertions: {timing_line}")
        lines.append(f"- Functional assertions: {functional_line}")
        if int(check_summary.get("protocol_total", 0)) > 0:
            lines.append(f"- Protocol assertions: {protocol_line}")
        lines.append(f"- Artifact path: `{run_dir}`")
        if description:
            lines.append(f"- Description: {description}")
        if notes:
            lines.append(f"- Notes: {'; '.join(str(n) for n in notes)}")
        if key_metrics:
            lines.append(f"- Key metrics: {key_metrics}")

        checks = payload.get("checks", [])
        if isinstance(checks, list) and checks:
            lines.append("- Assertion details:")
            for check in checks:
                if not isinstance(check, dict):
                    continue
                name = str(check.get("name", ""))
                category = str(check.get("category", "functional"))
                detail = str(check.get("detail", ""))
                passed = bool(check.get("passed", False))
                mark = "PASS" if passed else "FAIL"
                lines.append(f"  - [{mark}] `{name}` ({category}) - {detail}")
        lines.append("")

    lines.append("## Conclusion")
    lines.append("")
    if failed_cases == 0 and skipped_cases == 0:
        lines.append(
            "All BLE-enabled full-suite cases passed with complete timing, functional, and protocol assertion coverage."
        )
    else:
        lines.append(
            f"Suite completed with {passed_cases} passed, {failed_cases} failed, and {skipped_cases} skipped case(s)."
        )
    lines.append("")

    path.write_text("\n".join(lines), encoding="utf-8")
