from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any
import json

import yaml


@dataclass
class StimulusStep:
    at_ms: int
    signal: str
    state: str
    duration_ms: int | None = None


@dataclass
class TestVector:
    case_id: str
    scenario: str
    description: str
    parameters: dict[str, Any]
    fixture: dict[str, Any]
    stimulus: list[StimulusStep]
    expect: dict[str, Any] = field(default_factory=dict)
    metrics: list[str] = field(default_factory=list)
    tags: list[str] = field(default_factory=list)


def _require(data: dict[str, Any], key: str) -> Any:
    if key not in data:
        raise ValueError(f"vector missing required key: {key}")
    return data[key]


def load_raw(path: Path) -> dict[str, Any]:
    text = path.read_text(encoding="utf-8")
    if path.suffix.lower() in (".yaml", ".yml"):
        return yaml.safe_load(text)
    return json.loads(text)


def parse_vector(data: dict[str, Any]) -> TestVector:
    steps: list[StimulusStep] = []
    for s in data.get("stimulus", []):
        steps.append(
            StimulusStep(
                at_ms=int(s.get("atMs", 0)),
                signal=str(s.get("signal", "")),
                state=str(s.get("state", "active")).lower(),
                duration_ms=int(s["durationMs"]) if "durationMs" in s else None,
            )
        )

    expect = dict(data.get("expect", {}))
    if "fpOut" in expect and isinstance(expect["fpOut"], dict):
        expect["fpOut"] = dict(expect["fpOut"])
    if "timing" in expect and isinstance(expect["timing"], dict):
        expect["timing"] = dict(expect["timing"])

    return TestVector(
        case_id=str(_require(data, "id")),
        scenario=str(_require(data, "scenario")),
        description=str(data.get("description", "")),
        parameters=dict(data.get("parameters", {})),
        fixture=dict(data.get("fixture", {})),
        stimulus=steps,
        expect=expect,
        metrics=[str(m) for m in data.get("metrics", [])],
        tags=[str(t) for t in data.get("tags", [])],
    )


def load_vector(path: Path) -> TestVector:
    return parse_vector(load_raw(path))


def load_suite(path: Path) -> list[Path]:
    data = load_raw(path)
    if "cases" not in data:
        raise ValueError(f"suite file missing cases: {path}")
    base = path.parent
    return [base / str(item) for item in data["cases"]]

