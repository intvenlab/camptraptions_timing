from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from .constants import (
    CAMERA_CONFIG_LEN,
    CAMERA_FIELDS,
    CAMERA_SETTINGS_VERSION,
    CameraField,
)


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
    elif field.scale == "ms10_to_u16":
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
    raw[4] = 10  # shutterPulseDuration (100ms) little-endian ticks
    raw[5] = 0
    raw[6] = 100  # StartFrameSpacingMin (1.0s) little-endian ticks
    raw[7] = 0
    raw[8] = 20
    raw[9] = 35
    raw[10] = 20
    raw[11] = 4
    raw[12] = 4
    raw[13] = 0
    raw[14] = 0
    raw[15] = 0
    raw[16] = 0
    raw[17] = 0
    raw[18] = 0
    raw[19] = 0
    raw[20] = 1
    raw[21] = 31
    return raw


def _write_field(buf: bytearray, field: CameraField, value: int) -> None:
    if field.scale in {"ms10_to_u16"}:
        buf[field.index] = value & 0xFF
        buf[field.index + 1] = (value >> 8) & 0xFF
    else:
        buf[field.index] = value


def _read_field(raw: bytes, field: CameraField) -> int:
    if field.scale in {"ms10_to_u16"}:
        return int(raw[field.index]) | (int(raw[field.index + 1]) << 8)
    return int(raw[field.index])


def sanitize_camera_config_bytes(raw: bytes | bytearray) -> bytearray:
    """Mirror firmware sanitizeCameraConfig() for pre-write validation."""
    buf = bytearray(raw[:CAMERA_CONFIG_LEN].ljust(CAMERA_CONFIG_LEN, b"\x00"))
    buf[0] = CAMERA_SETTINGS_VERSION
    buf[1] = 1 if buf[1] else 0

    buf[2] = max(1, min(60, int(buf[2])))
    buf[3] = max(1, min(100, int(buf[3])))

    pulse = _read_field(buf, CAMERA_FIELDS["shutterPulseDuration"])
    pulse = max(1, min(3000, pulse))
    _write_field(buf, CAMERA_FIELDS["shutterPulseDuration"], pulse)

    spacing = _read_field(buf, CAMERA_FIELDS["StartFrameSpacingMin"])
    spacing = max(1, min(3000, spacing))
    _write_field(buf, CAMERA_FIELDS["StartFrameSpacingMin"], spacing)

    buf[8] = max(1, min(200, int(buf[8])))
    buf[9] = max(1, min(250, int(buf[9])))
    buf[10] = max(1, min(250, int(buf[10])))
    buf[11] = max(1, min(8, int(buf[11])))
    buf[12] = max(1, min(64, int(buf[12])))

    buf[13] = min(2, int(buf[13])) if int(buf[13]) <= 2 else 0
    buf[14] = 0
    buf[15] = 1 if int(buf[15]) > 1 else int(buf[15])
    buf[16] = 0
    buf[17] = 1 if int(buf[17]) > 1 else int(buf[17])
    buf[18] = 0
    buf[19] = 0
    buf[20] = 1 if buf[20] else 0
    buf[21] = max(5, min(250, int(buf[21])))
    return buf


def camera_config_has_invalid_values(raw: bytes | bytearray) -> bool:
    """True when firmware would NACK the payload with CAMCFG_NACK_OUT_OF_RANGE."""
    incoming = bytes(raw[:CAMERA_CONFIG_LEN].ljust(CAMERA_CONFIG_LEN, b"\x00"))
    sanitized = bytes(sanitize_camera_config_bytes(incoming))
    return incoming != sanitized


def decode_camera_config(raw: bytes) -> dict[str, int]:
    out: dict[str, int] = {}
    for name, field in CAMERA_FIELDS.items():
        out[name] = _read_field(raw, field)
    out["version"] = int(raw[0]) if raw else 0
    return out


def apply_named_parameters(base: bytes, params: dict[str, Any]) -> tuple[bytes, dict[str, int]]:
    buf = bytearray(base[:CAMERA_CONFIG_LEN].ljust(CAMERA_CONFIG_LEN, b"\x00"))
    normalized: dict[str, int] = {}
    for name, value in params.items():
        field = CAMERA_FIELDS.get(name)
        if field is None:
            continue
        n = _normalize(value, field)
        _write_field(buf, field, n)
        normalized[name] = n
    buf[0] = CAMERA_SETTINGS_VERSION
    return bytes(buf), normalized


def build_camera_config_payload(params: dict[str, Any]) -> tuple[bytes, dict[str, int]]:
    """Build a full v3 payload from factory defaults plus named overrides."""
    return apply_named_parameters(bytes(default_camera_config_bytes()), params)


def diff_readback(readback: bytes, requested_norm: dict[str, int]) -> CameraConfigDiff:
    read_map: dict[str, int] = {}
    mismatch: dict[str, tuple[int, int]] = {}
    for name, req_val in requested_norm.items():
        field = CAMERA_FIELDS[name]
        rb = _read_field(readback, field)
        read_map[name] = rb
        if rb != req_val:
            mismatch[name] = (req_val, rb)
    return CameraConfigDiff(requested=requested_norm, readback=read_map, mismatches=mismatch)

