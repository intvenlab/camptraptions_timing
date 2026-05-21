from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from .constants import CAMERA_CONFIG_LEN, CAMERA_FIELDS, CAMERA_SETTINGS_VERSION, CameraField


def _to_ms(value: Any) -> int:
    if isinstance(value, (int, float)):
        return int(value)
    s = str(value).strip().lower()
    if s.endswith("ms"):
        return int(float(s[:-2]))
    if s.endswith("s"):
        return int(float(s[:-1]) * 1000)
    return int(float(s))


def _normalize(value: Any, field: CameraField) -> int:
    if field.scale == "raw":
        v = int(value)
    elif field.scale == "s_to_u8_seconds":
        v = _to_ms(value) // 1000
    elif field.scale == "ms_to_u8":
        v = _to_ms(value)
    elif field.scale == "ms100_to_u8":
        v = _to_ms(value) // 100
    elif field.scale == "ms10_to_u8":
        v = _to_ms(value) // 10
    else:
        raise ValueError(f"unsupported scale: {field.scale}")
    return max(field.min_value, min(field.max_value, v))


@dataclass
class CameraConfigDiff:
    requested: dict[str, int]
    readback: dict[str, int]
    mismatches: dict[str, tuple[int, int]]


def default_camera_config_bytes() -> bytearray:
    raw = bytearray([0] * CAMERA_CONFIG_LEN)
    raw[0] = CAMERA_SETTINGS_VERSION
    raw[1] = 1
    raw[2] = 10
    raw[3] = 5
    raw[4] = 10
    raw[5] = 10
    raw[6] = 20
    raw[7] = 35
    raw[8] = 20
    raw[9] = 4
    raw[10] = 4
    raw[11] = 0
    raw[12] = 0
    raw[13] = 0
    raw[14] = 0
    raw[15] = 0
    raw[16] = 0
    raw[17] = 0
    raw[18] = 1
    raw[19] = 31
    return raw


def apply_named_parameters(base: bytes, params: dict[str, Any]) -> tuple[bytes, dict[str, int]]:
    buf = bytearray(base)
    normalized: dict[str, int] = {}
    for name, value in params.items():
        field = CAMERA_FIELDS.get(name)
        if field is None:
            continue
        n = _normalize(value, field)
        buf[field.index] = n
        normalized[name] = n
    buf[0] = CAMERA_SETTINGS_VERSION
    return bytes(buf), normalized


def diff_readback(readback: bytes, requested_norm: dict[str, int]) -> CameraConfigDiff:
    read_map: dict[str, int] = {}
    mismatch: dict[str, tuple[int, int]] = {}
    for name, req_val in requested_norm.items():
        field = CAMERA_FIELDS[name]
        rb = int(readback[field.index])
        read_map[name] = rb
        if rb != req_val:
            mismatch[name] = (req_val, rb)
    return CameraConfigDiff(requested=requested_norm, readback=read_map, mismatches=mismatch)

