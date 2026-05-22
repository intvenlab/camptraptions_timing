from __future__ import annotations

from dataclasses import dataclass

SERVICE_UUID = "ca500000-0000-0000-0000-000000000000"
CHAR_CAMERA_CONFIG_UUID = "ca50000a-0000-0000-0000-000000000000"
CHAR_TELEMETRY_UUID = "ca50000b-0000-0000-0000-000000000000"
CHAR_FACTORY_RESET_UUID = "ca500009-0000-0000-0000-000000000000"

CAMERA_SETTINGS_VERSION = 2
CAMERA_CONFIG_LEN = 20
TELEMETRY_VERSION = 1


@dataclass(frozen=True)
class CameraField:
    index: int
    min_value: int
    max_value: int
    scale: str


CAMERA_FIELDS: dict[str, CameraField] = {
    "enabled": CameraField(1, 0, 1, "raw"),
    "wakeHalfPressHoldTime": CameraField(2, 1, 60, "s_to_u8_seconds"),
    "minHalfPressBeforeShutter": CameraField(3, 1, 20, "ms100_to_u8"),
    "shutterPulseDuration": CameraField(4, 1, 200, "ms10_to_u8"),
    "StartFrameSpacingMin": CameraField(5, 2, 50, "ms100_to_u8"),
    "PostShutterHalfPressHoldTimeExtension": CameraField(6, 1, 100, "ms100_to_u8"),
    "halfPressInputDebounce": CameraField(7, 1, 100, "ms_to_u8"),
    "fullPressInputDebounce": CameraField(8, 1, 100, "ms_to_u8"),
    "FrameCount": CameraField(9, 1, 8, "raw"),
    "MaxSequenceCount": CameraField(10, 1, 64, "raw"),
    "wakeHoldRefreshPolicy": CameraField(11, 0, 2, "raw"),
    "halfPressDuringBurstPolicy": CameraField(12, 0, 1, "raw"),
    "fullPressWithoutPriorHpPolicy": CameraField(13, 0, 1, "raw"),
    "activityHalfPressHoldPolicy": CameraField(14, 0, 1, "raw"),
    "fpAfterMaxSequenceCountPolicy": CameraField(15, 0, 1, "raw"),
    "inputActivePolarity": CameraField(16, 0, 1, "raw"),
    "outputDriveMode": CameraField(17, 0, 1, "raw"),
    "powerSaveIdleMode": CameraField(18, 0, 1, "raw"),
    "fullPressIgnoreGap": CameraField(19, 5, 255, "ms100_to_u8"),
}

