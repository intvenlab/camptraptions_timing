#pragma once

#include <Arduino.h>
#include <stdint.h>

// Uncomment in Camtraptions_Firmware.ino to emit state-machine pin transition logs over USB serial.
// #define DEBUG_CAMERA_LOGIC_PINS

// ─── Pins ────────────────────────────────────────────────────────────────────
#define BATTERY_PIN          A0
#define DEVICE_BATTERY_PIN   A1
#define FP_IN_PIN            2
#define HP_IN_PIN            3
#define FP_OUT_PIN           4
#define HP_OUT_PIN           5

// ─── ADC ─────────────────────────────────────────────────────────────────────
#define ADC_MAX_VALUE          1024.0f
#define ADC_REFERENCE_VOLTAGE  3.0f
#define EXT_BATT_DIVIDER_RATIO 6.0f

// ─── Timing ──────────────────────────────────────────────────────────────────
#define ADVERTISING_DURATION_MS 50
#define SLEEP_INTERVAL_MS       1000
#define SHUTTER_DEBOUNCE_MS     100

// ─── Flash ───────────────────────────────────────────────────────────────────
#define SETTINGS_FILE           "/settings.bin"
#define SETTINGS_VERSION        1
#define CAMERA_SETTINGS_FILE    "/camera.bin"
#define CAMERA_SETTINGS_VERSION 3
#define TELEMETRY_FILE          "/telemetry.bin"
#define TELEMETRY_VERSION       2
#define CAL_FILE                "/cal.bin"
#define CALIBRATION_VERSION     1
#define INT_CAL_FILE            "/int_cal.bin"
#define BEACON_LAYOUT_VERSION   2
#define TELEMETRY_FLUSH_INTERVAL_MS 60000UL

#define CONNECTED_ACTIVE_TELEMETRY_MIN_INTERVAL_MS 200UL
#define CONNECTED_POST_TELEMETRY_MIN_INTERVAL_MS   100UL
#define CONNECTED_IDLE_TELEMETRY_MIN_INTERVAL_MS    75UL
#define CONNECTED_POST_MODE_WINDOW_MS              1500UL
#define CONNECTED_ACTIVE_BLE_BUDGET_US              500UL
#define CONNECTED_POST_BLE_BUDGET_US               2500UL
#define CONNECTED_IDLE_BLE_BUDGET_US               3500UL

// ─── Device settings ─────────────────────────────────────────────────────────
struct DeviceConfig {
  uint8_t  version;
  uint8_t  configured;
  char     name[21];
  uint8_t  groupId;
  char     groupName[21];
  uint8_t  deviceType;
  uint8_t  chemistry;
  uint8_t  cellCount;
  uint32_t shutterCount;
};

// ─── Camera config (22 bytes) ────────────────────────────────────────────────
struct __attribute__((packed)) CameraConfig {
  uint8_t version;
  uint8_t enabled;
  uint8_t wakeHalfPressHoldSec;
  uint8_t minHalfPressBeforeShutter;
  uint16_t shutterPulseDuration;
  uint16_t startFrameSpacingTicks;
  uint8_t postShutterHpHoldTenths;
  uint8_t hpDebounceMs;
  uint8_t fpDebounceMs;
  uint8_t frameCount;
  uint8_t maxSequenceCount;
  uint8_t wakeHoldRefreshPolicy;
  uint8_t halfPressDuringBurstPolicy;
  uint8_t fullPressWithoutHpPolicy;
  uint8_t activityHalfPressHoldPolicy;
  uint8_t fpAfterMaxSeqCountPolicy;
  uint8_t inputActivePolarity;
  uint8_t outputDriveMode;
  uint8_t powerSaveIdleMode;
  uint8_t fullPressIgnoreGapTenths;
};

enum TelemetryEvent : uint8_t {
  TEL_EVT_NONE = 0,
  TEL_EVT_HP_WAKE = 1,
  TEL_EVT_HP_REFRESH = 2,
  TEL_EVT_FP_ACCEPTED = 3,
  TEL_EVT_WAKE_TIMEOUT = 4,
  TEL_EVT_FP_REJECT_GAP = 5,
  TEL_EVT_FP_REJECT_CAP = 6,
  TEL_EVT_BURST_COMPLETE = 7,
  TEL_EVT_ACTIVITY_END = 8,
  TEL_EVT_COLD_FP = 9,
  TEL_EVT_HP_IGNORED_BURST = 10,
  TEL_EVT_FP_DEBOUNCE_REJECT = 11,
  TEL_EVT_HP_DEBOUNCE_REJECT = 12,
  TEL_EVT_FP_ACCEPTED_AT_GAP_BOUNDARY = 13
};

enum TelemetryScenarioHint : uint8_t {
  TEL_SC_NONE = 0,
  TEL_SC_WAKE_TIMEOUT = 1,
  TEL_SC_FP_GAP_IGNORE = 2,
  TEL_SC_COLD_FP = 3,
  TEL_SC_SEQUENCE_CAP = 4,
  TEL_SC_HP_DURING_BURST = 5,
  TEL_SC_DEBOUNCE = 6
};

struct CameraTelemetryCounters {
  uint8_t  version;
  uint8_t  reserved[3];
  uint32_t wakeTimeoutCount;
  uint32_t acceptedFpCount;
  uint32_t ignoredFpDuringGapCount;
  uint32_t ignoredFpDuringBurstCount;
  uint32_t MaxSequenceExceededCount;
  uint32_t coldFpSequenceCount;
  uint32_t hpRefreshCount;
  uint32_t hpIgnoredDuringBurstCount;
  uint32_t fpDebounceRejectCount;
  uint32_t hpDebounceRejectCount;
  uint32_t sequenceCompletedCount;
  uint32_t activityCompletedCount;
  uint32_t bootCount;
};

struct CameraTelemetryPayload {
  uint8_t  version;
  uint8_t  cameraState;
  uint8_t  flags;
  uint8_t  framesFiredThisSequence;
  uint8_t  sequencesStartedThisActivity;
  uint8_t  lastEventCode;
  uint8_t  lastScenarioHint;
  uint8_t  reserved;
  uint32_t msUntilWakeDeadline;
  uint32_t msUntilFpIgnoreClear;
  uint32_t msUntilNextFrame;
  uint32_t msUntilPostHoldEnd;
  CameraTelemetryCounters counters;
};

struct CalibrationConfig {
  uint8_t version;
  uint8_t reserved[3];
  float   dividerRatio;
};

struct IntCalibrationConfig {
  uint8_t version;
  uint8_t reserved[3];
  float   scaleFactor;
};

enum CamCfgWriteStatusCode : uint8_t {
  CAMCFG_ACK_APPLIED       = 0x00,
  CAMCFG_NACK_BAD_FORMAT   = 0xE1,
  CAMCFG_NACK_OUT_OF_RANGE = 0xE2,
  CAMCFG_NACK_BUSY         = 0xE3
};

enum CameraState {
  CAM_IDLE,
  CAM_WAKE_AF,
  CAM_COLD_FP_WAIT,
  CAM_BURST_ACTIVE,
  CAM_POST_SHUTTER_EXT
};

inline bool timeReached(uint32_t now, uint32_t target) {
  return (int32_t)(now - target) >= 0;
}

// ─── Shared runtime state (defined in module .cpp files) ───────────────────────
extern DeviceConfig cfg;
extern CameraConfig camCfg;
extern CameraTelemetryCounters telCounters;
extern CameraTelemetryPayload telPayload;

extern volatile bool isConnected;
extern volatile bool settingsDirty;
extern volatile bool shutterUpdated;
extern volatile bool telemetryDirty;
extern volatile bool telemetryUpdated;
extern volatile uint32_t lastShutterMs;

extern uint32_t nextTelemetryFlushMs;
extern uint8_t  lastTelemetryEvent;
extern uint8_t  lastTelemetryScenario;

extern volatile bool fpPulseFlag;
extern volatile bool hpPulseFlag;
extern volatile uint32_t lastHpMs;

extern CameraState cameraState;
extern bool cameraLogicActive;
extern bool activityActive;
extern bool hpOutAsserted;
extern bool coldFpAcceptPending;
extern uint8_t framesFired;
extern uint8_t sequencesStartedThisActivity;
extern uint32_t hpAssertedMs;
extern uint32_t wakeHoldDeadlineMs;
extern uint32_t sequenceStartMs;
extern uint32_t nextFrameMs;
extern uint32_t lastFpOutStartMs;
extern uint32_t fpOutReleaseMs;
extern uint32_t postShutterHoldUntilMs;
extern uint32_t fullPressIgnoreUntilMs;
extern uint32_t maxSequenceTimeoutUntilMs;
extern bool fpAcceptedAtGapBoundary;
extern bool pendingCamCfgApply;
extern volatile bool runtimeIoReconfigurePending;
extern CameraConfig pendingCamCfg;
extern uint8_t camCfgWriteStatus;
extern uint32_t lastActivityEndMs;
extern uint32_t lastTelemetryServiceMs;

extern volatile bool pendingCalApply;
extern volatile bool pendingIntCalApply;
