from __future__ import annotations

from dataclasses import dataclass

SERVICE_UUID = "ca500000-0000-0000-0000-000000000000"
CHAR_CAMERA_CONFIG_UUID = "ca50000a-0000-0000-0000-000000000000"
CHAR_TELEMETRY_UUID = "ca50000b-0000-0000-0000-000000000000"
CHAR_FACTORY_RESET_UUID = "ca500009-0000-0000-0000-000000000000"
CHAR_CAMERA_CONFIG_STATUS_UUID = "ca50000e-0000-0000-0000-000000000000"

CAMCFG_ACK_APPLIED = 0x00
CAMCFG_NACK_BAD_FORMAT = 0xE1
CAMCFG_NACK_OUT_OF_RANGE = 0xE2
CAMCFG_NACK_BUSY = 0xE3

CAMCFG_STATUS_NAMES: dict[int, str] = {
    CAMCFG_ACK_APPLIED: "ACK_APPLIED",
    CAMCFG_NACK_BAD_FORMAT: "NACK_BAD_FORMAT",
    CAMCFG_NACK_OUT_OF_RANGE: "NACK_OUT_OF_RANGE",
    CAMCFG_NACK_BUSY: "NACK_BUSY",
}

CAMCFG_STATUS_BY_NAME: dict[str, int] = {v: k for k, v in CAMCFG_STATUS_NAMES.items()}

CAMERA_SETTINGS_VERSION = 3
CAMERA_CONFIG_LEN = 22
TELEMETRY_VERSION = 3
TELEMETRY_PAYLOAD_LEN = 84

# Delay after issuing any BLE setting write so firmware can settle runtime I/O updates.
BLE_SETTING_CHANGE_SETTLE_S = 1.0

# Delay after camera-config write status before reading ca50000a readback (GATT cache lag).
CAMERA_CONFIG_READBACK_DELAY_S = 0.10


@dataclass(frozen=True)
class CameraField:
    index: int
    min_value: int
    max_value: int
    scale: str


CAMERA_FIELDS: dict[str, CameraField] = {
    "enabled": CameraField(1, 0, 1, "raw"),
    "wakeHalfPressHoldTime": CameraField(2, 1, 60, "s_to_u8_seconds"),
    "minHalfPressBeforeShutter": CameraField(3, 1, 100, "ms100_to_u8"),
    "shutterPulseDuration": CameraField(4, 1, 3000, "ms10_to_u16"),
    "StartFrameSpacingMin": CameraField(6, 1, 3000, "ms10_to_u16"),
    "PostShutterHalfPressHoldTimeExtension": CameraField(8, 0, 200, "ms100_to_u8"),
    "halfPressInputDebounce": CameraField(9, 1, 100, "ms_to_u8"),
    "fullPressInputDebounce": CameraField(10, 1, 100, "ms_to_u8"),
    "FrameCount": CameraField(11, 1, 8, "raw"),
    "MaxSequenceCount": CameraField(12, 1, 64, "raw"),
    "wakeHoldRefreshPolicy": CameraField(13, 0, 2, "raw"),
    "halfPressDuringBurstPolicy": CameraField(14, 0, 1, "raw"),
    "fullPressWithoutPriorHpPolicy": CameraField(15, 0, 1, "raw"),
    "activityHalfPressHoldPolicy": CameraField(16, 0, 1, "raw"),
    "fpAfterMaxSequenceCountPolicy": CameraField(17, 0, 1, "raw"),
    "inputActivePolarity": CameraField(18, 0, 1, "raw"),
    "outputDriveMode": CameraField(19, 0, 1, "raw"),
    "powerSaveIdleMode": CameraField(20, 0, 1, "raw"),
    "fullPressIgnoreGap": CameraField(21, 5, 250, "ms100_to_u8"),
}

