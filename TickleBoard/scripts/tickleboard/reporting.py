from __future__ import annotations

from pathlib import Path
import csv


def write_markdown_report(path: Path, title: str, records: list[dict]) -> None:
    timing_passed = sum(int(r.get("timing_passed", 0)) for r in records)
    timing_total = sum(int(r.get("timing_total", 0)) for r in records)
    functional_passed = sum(int(r.get("functional_passed", 0)) for r in records)
    functional_total = sum(int(r.get("functional_total", 0)) for r in records)
    lines: list[str] = [
        f"# {title}",
        "",
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
        "| Case | Scenario | Passed | Failure Class | Timing | Functional | Notes |",
        "|------|----------|--------|---------------|--------|------------|-------|",
        ]
    )
    for r in records:
        notes = "; ".join(r.get("notes", []))
        lines.append(
            f"| {r.get('case_id','')} | {r.get('scenario','')} | {r.get('passed', False)} | "
            f"{r.get('failure_class','')} | {r.get('timing_passed',0)}/{r.get('timing_total',0)} | "
            f"{r.get('functional_passed',0)}/{r.get('functional_total',0)} | {notes} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_csv_rollup(path: Path, records: list[dict]) -> None:
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "case_id",
                "scenario",
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
                    "passed": r.get("passed", False),
                    "failure_class": r.get("failure_class", ""),
                    "timing_passed": r.get("timing_passed", 0),
                    "timing_total": r.get("timing_total", 0),
                    "functional_passed": r.get("functional_passed", 0),
                    "functional_total": r.get("functional_total", 0),
                    "run_dir": r.get("run_dir", ""),
                }
            )

