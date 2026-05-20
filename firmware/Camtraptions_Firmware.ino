/*
 * Camtraptions BLE Battery Monitor – Seeed Studio XIAO nRF52840
 * Phase 3: Dual battery, camera I/O framework + state machine
 *
 * Setup Instructions:
 * 1. Install "Seeed nRF52 Boards" in Arduino IDE Board Manager
 * 2. Select "Seeed XIAO nRF52840" as your board
 * 3. Upload this sketch
 *
 * GATT UUIDs must match GattUuids object in Android AppScreen.kt.
 */

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

// ─── Pins ────────────────────────────────────────────────────────────────────
#define BATTERY_PIN          A0  // D0/A0 – Internal CR2032 power-supply ADC input
#define DEVICE_BATTERY_PIN   A1  // D1/A1 – Device/camera battery ADC input (primary gauge)
#define FP_IN_PIN            2   // D2    – FP/shutter input, FALLING interrupt
#define HP_IN_PIN            3   // D3    – HP input, FALLING interrupt
#define FP_OUT_PIN           4   // D4 – FP output to camera (open-drain)
#define HP_OUT_PIN           5   // D5 – HP output to camera (open-drain)

// ─── ADC ─────────────────────────────────────────────────────────────────────
#define VOLTAGE_DIVIDER_RATIO 1.0f
#define ADC_MAX_VALUE         1024.0f

// ─── Timing ──────────────────────────────────────────────────────────────────
#define ADVERTISING_DURATION_MS 50
#define SLEEP_INTERVAL_MS       1000
#define SHUTTER_DEBOUNCE_MS     100   // FP_IN ISR debounce (backward compat)

// ─── Flash ───────────────────────────────────────────────────────────────────
#define SETTINGS_FILE           "/settings.bin"
#define SETTINGS_VERSION        1
#define CAMERA_SETTINGS_FILE    "/camera.bin"
#define CAMERA_SETTINGS_VERSION 2
#define TELEMETRY_FILE          "/telemetry.bin"
#define TELEMETRY_VERSION       1
#define TELEMETRY_FLUSH_INTERVAL_MS 60000UL

// ─── Device settings (layout unchanged – SETTINGS_VERSION stays at 1) ───────
// Bump SETTINGS_VERSION only if you add/remove/reorder fields here.
struct DeviceConfig {
  uint8_t  version;        // struct version guard
  uint8_t  configured;     // 0 = factory fresh, 1 = user-configured
  char     name[21];       // user-assigned device name, null-terminated
  uint8_t  groupId;        // 0 = no group, 1–255 = group membership
  char     groupName[21];  // shared group label, null-terminated
  uint8_t  deviceType;     // 0=battery_monitor, 1=camera, 2=strobe, 3=focus_light
  uint8_t  chemistry;      // 0=LiPo, 1=LiFePO4, 2=NiMH, 3=Alkaline
  uint8_t  cellCount;      // 1–8
  uint32_t shutterCount;   // camera shutter actuations (incremented by ISR)
};

// ─── Camera config (20 bytes, stored in /camera.bin) ─────────────────────────
// Guards with CAMERA_SETTINGS_VERSION; independent of DeviceConfig.
struct CameraConfig {
  uint8_t version;                      // CAMERA_SETTINGS_VERSION
  uint8_t enabled;                      // 0=disabled, 1=enabled
  uint8_t wakeHalfPressHoldSec;         // X seconds (default 10) – max HP hold before timeout
  uint8_t minHalfPressBeforeShutter;    // T ×100ms (default 5 → 0.5s) – AF settle time
  uint8_t shutterPulseDuration;         // ×10ms (default 10 → 100ms)
  uint8_t startFrameSpacingTenths;      // Y ×100ms (default 10 → 1.0s) between frames
  uint8_t postShutterHpHoldTenths;      // Z ×100ms (default 20 → 2.0s) HP hold after burst
  uint8_t hpDebounceMs;                 // default 35
  uint8_t fpDebounceMs;                 // default 20
  uint8_t frameCount;                   // N frames per sequence (default 4, range 1–8)
  uint8_t maxSequenceCount;             // max sequences per activity (default 4, range 1–8)
  uint8_t wakeHoldRefreshPolicy;        // 0=extend 1=restart 2=ignoreWhileActive
  uint8_t halfPressDuringBurstPolicy;   // 0=independent
  uint8_t fullPressWithoutHpPolicy;     // 0=assertHpThenWait 1=ignoreFP
  uint8_t activityHalfPressHoldPolicy;  // 0=holdUntilActivityEnd
  uint8_t fpAfterMaxSeqCountPolicy;     // 0=ignoreUntilActivityEnd
  uint8_t inputActivePolarity;          // 0=activeLow 1=activeHigh
  uint8_t outputDriveMode;              // 0=openDrain 1=pushPull
  uint8_t powerSaveIdleMode;            // 0=disabled 1=enabled (default)
  uint8_t fullPressIgnoreGapTenths;     // R10 ×100ms (default 31 → 3.1s)
};
// sizeof(CameraConfig) == 20  (verified: 1+19 bytes)

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
  TEL_EVT_HP_DEBOUNCE_REJECT = 12
};

enum TelemetryScenarioHint : uint8_t {
  TEL_SC_NONE = 0,
  TEL_SC_WAKE_TIMEOUT = 1,     // SC-04 / SC-04b / SC-12
  TEL_SC_FP_GAP_IGNORE = 2,    // SC-02 / SC-03 / SC-14
  TEL_SC_COLD_FP = 3,          // SC-06 / SC-08
  TEL_SC_SEQUENCE_CAP = 4,     // SC-09 / SC-10
  TEL_SC_HP_DURING_BURST = 5,  // SC-07 / SC-18
  TEL_SC_DEBOUNCE = 6          // SC-13
};

// Coarse lifetime counters persisted separately from user settings.
struct CameraTelemetryCounters {
  uint8_t  version;
  uint8_t  reserved[3];
  uint32_t wakeTimeoutCount;
  uint32_t acceptedFpCount;
  uint32_t ignoredFpDuringGapCount;
  uint32_t ignoredFpDuringBurstCount;
  uint32_t rejectedFpAtSequenceCapCount;
  uint32_t coldFpSequenceCount;
  uint32_t hpRefreshCount;
  uint32_t hpIgnoredDuringBurstCount;
  uint32_t fpDebounceRejectCount;
  uint32_t hpDebounceRejectCount;
  uint32_t sequenceCompletedCount;
  uint32_t activityCompletedCount;
};

// BLE snapshot: live state + persisted counters, little-endian on nRF52.
struct CameraTelemetryPayload {
  uint8_t  version;
  uint8_t  cameraState;
  uint8_t  flags;  // bit0 activityActive, bit1 hpOutAsserted
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

static DeviceConfig cfg;
static CameraConfig camCfg;
static CameraTelemetryCounters telCounters;
static CameraTelemetryPayload  telPayload;
static File         cfgFile(InternalFS);
static File         camFile(InternalFS);
static File         telFile(InternalFS);

// ─── GATT Service & Characteristics ──────────────────────────────────────────
// 128-bit UUIDs – must match GattUuids object in Android AppScreen.kt

BLEService svc("ca500000-0000-0000-0000-000000000000");

BLECharacteristic chrName     ("ca500001-0000-0000-0000-000000000000");
BLECharacteristic chrGroupId  ("ca500002-0000-0000-0000-000000000000");
BLECharacteristic chrGroupName("ca500003-0000-0000-0000-000000000000");
BLECharacteristic chrDevType  ("ca500004-0000-0000-0000-000000000000");
BLECharacteristic chrChemistry("ca500005-0000-0000-0000-000000000000");
BLECharacteristic chrCellCount("ca500006-0000-0000-0000-000000000000");
BLECharacteristic chrShutter  ("ca500007-0000-0000-0000-000000000000"); // Read + Notify
BLECharacteristic chrReset    ("ca500008-0000-0000-0000-000000000000"); // Write only
BLECharacteristic chrFactory  ("ca500009-0000-0000-0000-000000000000"); // Write only
BLECharacteristic chrCamCfg   ("ca50000a-0000-0000-0000-000000000000"); // Camera config R/W
BLECharacteristic chrTelemetry("ca50000b-0000-0000-0000-000000000000"); // Camera telemetry R + Notify

// ─── Runtime state ────────────────────────────────────────────────────────────
volatile bool     isConnected    = false;
volatile bool     settingsDirty  = false;  // shutter count changed in ISR
volatile bool     shutterUpdated = false;  // notify pending
volatile bool     telemetryDirty  = false;  // persisted telemetry changed
volatile bool     telemetryUpdated = false; // notify pending
volatile uint32_t lastShutterMs  = 0;
static uint32_t   nextTelemetryFlushMs = 0;
static uint8_t    lastTelemetryEvent = TEL_EVT_NONE;
static uint8_t    lastTelemetryScenario = TEL_SC_NONE;

// Camera I/O ISR flags
volatile bool     fpPulseFlag    = false;  // FP_IN fired
volatile bool     hpPulseFlag    = false;  // HP_IN fired
volatile uint32_t lastHpMs       = 0;

// Camera state machine
enum CameraState {
  CAM_IDLE,
  CAM_WAKE_AF,
  CAM_COLD_FP_WAIT,
  CAM_BURST_ACTIVE,
  CAM_POST_SHUTTER_EXT
};

static CameraState cameraState              = CAM_IDLE;
static bool        cameraLogicActive        = false;
static bool        activityActive           = false;
static bool        hpOutAsserted            = false;
static bool        coldFpAcceptPending      = false;
static uint8_t     framesFired              = 0;
static uint8_t     sequencesStartedThisActivity = 0;
static uint32_t    hpAssertedMs             = 0;
static uint32_t    wakeHoldDeadlineMs       = 0;
static uint32_t    sequenceStartMs          = 0;
static uint32_t    nextFrameMs              = 0;  // next FP_OUT rising edge target
static uint32_t    lastFpOutStartMs         = 0;
static uint32_t    fpOutReleaseMs           = 0;  // when to release FP_OUT (0 = idle)
static uint32_t    postShutterHoldUntilMs   = 0;
static uint32_t    fullPressIgnoreUntilMs   = 0;  // reject FP triggers during R10 window

// ─── Forward declarations ─────────────────────────────────────────────────────
void loadSettings();
void saveSettings();
void resetToDefaults();
void populateCharacteristics();
void setupGatt();

void loadCameraSettings();
void saveCameraSettings();
void resetCameraToDefaults();
void populateCameraCharacteristics();
void setupCameraGatt();

void resetTelemetryCounters();
void loadTelemetry();
void saveTelemetry();
void markTelemetryChanged(uint8_t eventCode, uint8_t scenarioHint, bool flushSoon);
void markTelemetryEvent(uint8_t eventCode, uint8_t scenarioHint);
uint32_t remainingMs(uint32_t now, uint32_t target);
void populateTelemetryPayload();
void populateTelemetryCharacteristic();
void flushTelemetryIfDue(uint32_t now);
void setupTelemetryGatt();

void advertiseData(int intPct, float intVoltage, uint8_t extPct, uint16_t extVoltMv);
float readBatteryVoltage();
bool  readDeviceBattery(int &pct, float &voltMv);
int   readCR2032Percentage(float voltage);  // internal CR2032 coin cell (A0)
int   readLiPoPercentage(float voltage);    // device/camera LiPo battery  (A1)

void assertPin(int pin);
void releasePin(int pin);
void assertHpOut(uint32_t now);
void releaseHpOut();
bool timeReached(uint32_t now, uint32_t target);
uint32_t minHalfPressMs();
uint32_t shutterPulseMs();
uint32_t startFrameSpacingMs();
uint32_t postShutterHoldMs();
uint32_t wakeHalfPressHoldMs();
uint32_t fullPressIgnoreGapMs();
bool hpLeadSatisfied(uint32_t now);
bool underSequenceCap();
bool tryAcceptFp(uint32_t now);
void startSequence(uint32_t now);
void endActivity();
void handleFpAfterCap();
void runBurstScheduler(uint32_t now);
void processCameraLogic();
void idleWaitWithCameraWake(uint32_t durationMs);

void onConnect   (uint16_t connHdl);
void onDisconnect(uint16_t connHdl, uint8_t reason);
void onShutterPulse();    // FP_IN ISR – state-machine mode (FALLING)
void onHpPulse();         // HP_IN ISR – state-machine mode (FALLING)
void onFpPassthrough();   // FP_IN ISR – pass-through mode  (CHANGE)
void onHpPassthrough();   // HP_IN ISR – pass-through mode  (CHANGE)

void onNameWrite     (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onGroupIdWrite  (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onGroupNameWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onDevTypeWrite  (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onChemWrite     (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onCellWrite     (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onResetWrite    (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onFactoryWrite  (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);
void onCamCfgWrite   (uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l);

// ═════════════════════════════════════════════════════════════════════════════
// setup()
// ═════════════════════════════════════════════════════════════════════════════
void setup() {
  // Battery ADC pins
  pinMode(BATTERY_PIN,        INPUT);
  pinMode(DEVICE_BATTERY_PIN, INPUT);

  // LEDs off (active-low on XIAO nRF52840)
  pinMode(LED_RED,   OUTPUT); digitalWrite(LED_RED,   HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE,  OUTPUT); digitalWrite(LED_BLUE,  HIGH);

  // DC/DC converter for better efficiency; low-power CPU mode
  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

  // Load persisted settings from flash (must precede pin setup)
  InternalFS.begin();
  loadSettings();
  loadCameraSettings();
  loadTelemetry();

  // ── Camera device I/O ────────────────────────────────────────────────────
  if (cfg.deviceType == 1 /* CAMERA */) {
    pinMode(FP_IN_PIN, INPUT_PULLUP);
    pinMode(HP_IN_PIN, INPUT_PULLUP);

    if (camCfg.enabled) {
      // State-machine mode:
      //   FALLING-only interrupts feed the state machine.
      //   Outputs are open-drain: idle as INPUT (high-Z), asserted as OUTPUT LOW.
      attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onShutterPulse, FALLING);
      attachInterrupt(digitalPinToInterrupt(HP_IN_PIN), onHpPulse,      FALLING);
      pinMode(FP_OUT_PIN, INPUT);  // high-Z idle
      pinMode(HP_OUT_PIN, INPUT);  // high-Z idle
    } else {
      // Pass-through mode:
      //   CHANGE interrupts mirror pin state instantly (ISR-driven, no loop latency).
      //   Outputs are push-pull, initialized to match the current input state so
      //   there is no glitch at boot.
      //   FP_IN shutter counting still runs inside onFpPassthrough.
      pinMode(FP_OUT_PIN, OUTPUT);
      digitalWrite(FP_OUT_PIN, digitalRead(FP_IN_PIN));  // sync to current state
      pinMode(HP_OUT_PIN, OUTPUT);
      digitalWrite(HP_OUT_PIN, digitalRead(HP_IN_PIN));  // sync to current state
      attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onFpPassthrough, CHANGE);
      attachInterrupt(digitalPinToInterrupt(HP_IN_PIN), onHpPassthrough, CHANGE);
    }
  } else {
    // Non-camera device: only FP_IN is monitored for shutter counting.
    // HP_IN, FP_OUT, HP_OUT are not touched (remain at power-on INPUT default).
    pinMode(FP_IN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onShutterPulse, FALLING);
  }

  // BLE – allow connections now (needed for GATT)
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.autoConnLed(false);
  Bluefruit.setConnLedInterval(0);

  // Use stored name if configured, otherwise generic discoverable name
  Bluefruit.setName(cfg.configured && cfg.name[0] ? cfg.name : "Camtraptions Device");

  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);

  setupGatt();
  // loop() handles the first advertisement
}

// ═════════════════════════════════════════════════════════════════════════════
// loop()
// ═════════════════════════════════════════════════════════════════════════════
void loop() {
  if (isConnected) {
    // Stay awake while a phone is connected.
    if (shutterUpdated) {
      shutterUpdated = false;
      chrShutter.notify32(cfg.shutterCount);
    }
    if (telemetryUpdated) {
      telemetryUpdated = false;
      populateTelemetryCharacteristic();
      chrTelemetry.notify((uint8_t*)&telPayload, sizeof(telPayload));
    } else {
      populateTelemetryCharacteristic();
    }
    flushTelemetryIfDue(millis());
    delay(50);
    return;
  }

  // ── Camera state machine (camera device type only) ────────────────────────
  if (cfg.deviceType == 1 /* CAMERA */ && camCfg.enabled) {
    processCameraLogic();
  }

  // Sleep guard: don't advertise or sleep while camera logic is running
  if (cameraLogicActive) {
    delay(1);  // keep output pulse timing tight without busy-looping
    return;
  }

  // ── Advertisement cycle (not connected, camera idle or non-camera) ─────────
  float intVoltage = readBatteryVoltage();
  int   intPct     = readCR2032Percentage(intVoltage);

  int   extPctInt = -1;
  float extVoltMvF = 0.0f;
  bool  extPresent = readDeviceBattery(extPctInt, extVoltMvF);
  uint8_t  extBatPct = extPresent ? (uint8_t)extPctInt    : 0xFF;
  uint16_t extBatMv  = extPresent ? (uint16_t)extVoltMvF  : 0xFFFF;

  advertiseData(intPct, intVoltage, extBatPct, extBatMv);
  idleWaitWithCameraWake(ADVERTISING_DURATION_MS);
  Bluefruit.Advertising.stop();

  // Flush any pending flash write (shutter count incremented by ISR)
  if (settingsDirty) {
    saveSettings();
    settingsDirty = false;
  }
  flushTelemetryIfDue(millis());

  idleWaitWithCameraWake(SLEEP_INTERVAL_MS - ADVERTISING_DURATION_MS);
}

// ═════════════════════════════════════════════════════════════════════════════
// Flash storage – DeviceConfig
// ═════════════════════════════════════════════════════════════════════════════
void loadSettings() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.version   = SETTINGS_VERSION;
  cfg.cellCount = 1;

  if (!InternalFS.exists(SETTINGS_FILE)) return;

  if (cfgFile.open(SETTINGS_FILE, FILE_O_READ)) {
    cfgFile.read(&cfg, sizeof(cfg));
    cfgFile.close();
  }

  if (cfg.version != SETTINGS_VERSION) {
    memset(&cfg, 0, sizeof(cfg));
    cfg.version   = SETTINGS_VERSION;
    cfg.cellCount = 1;
  }
}

void saveSettings() {
  InternalFS.remove(SETTINGS_FILE);
  if (cfgFile.open(SETTINGS_FILE, FILE_O_WRITE)) {
    cfgFile.write((const uint8_t*)&cfg, sizeof(cfg));
    cfgFile.close();
  }
}

void resetToDefaults() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.version   = SETTINGS_VERSION;
  cfg.cellCount = 1;
  InternalFS.remove(SETTINGS_FILE);
}

// ═════════════════════════════════════════════════════════════════════════════
// Flash storage – CameraConfig
// ═════════════════════════════════════════════════════════════════════════════
void resetCameraToDefaults() {
  memset(&camCfg, 0, sizeof(camCfg));
  camCfg.version                   = CAMERA_SETTINGS_VERSION;
  camCfg.enabled                   = 0;
  camCfg.wakeHalfPressHoldSec      = 10;
  camCfg.minHalfPressBeforeShutter = 5;
  camCfg.shutterPulseDuration      = 10;
  camCfg.startFrameSpacingTenths   = 10;
  camCfg.postShutterHpHoldTenths   = 20;
  camCfg.hpDebounceMs              = 35;
  camCfg.fpDebounceMs              = 20;
  camCfg.frameCount                = 4;
  camCfg.maxSequenceCount          = 4;
  camCfg.powerSaveIdleMode         = 1;
  camCfg.fullPressIgnoreGapTenths  = 31;
  // all policy and mode fields default to 0
}

void loadCameraSettings() {
  resetCameraToDefaults();
  if (!InternalFS.exists(CAMERA_SETTINGS_FILE)) return;
  if (camFile.open(CAMERA_SETTINGS_FILE, FILE_O_READ)) {
    camFile.read(&camCfg, sizeof(camCfg));
    camFile.close();
  }
  if (camCfg.version != CAMERA_SETTINGS_VERSION) resetCameraToDefaults();
}

void saveCameraSettings() {
  InternalFS.remove(CAMERA_SETTINGS_FILE);
  if (camFile.open(CAMERA_SETTINGS_FILE, FILE_O_WRITE)) {
    camFile.write((const uint8_t*)&camCfg, sizeof(camCfg));
    camFile.close();
  }
}

// ═════════════════════════════════════════════════════════════════════════════
// Flash storage – Camera telemetry counters
// ═════════════════════════════════════════════════════════════════════════════
void resetTelemetryCounters() {
  memset(&telCounters, 0, sizeof(telCounters));
  telCounters.version = TELEMETRY_VERSION;
}

void loadTelemetry() {
  resetTelemetryCounters();
  if (!InternalFS.exists(TELEMETRY_FILE)) return;
  if (telFile.open(TELEMETRY_FILE, FILE_O_READ)) {
    telFile.read(&telCounters, sizeof(telCounters));
    telFile.close();
  }
  if (telCounters.version != TELEMETRY_VERSION) resetTelemetryCounters();
}

void saveTelemetry() {
  CameraTelemetryCounters snapshot;
  noInterrupts();
  memcpy(&snapshot, &telCounters, sizeof(snapshot));
  interrupts();

  InternalFS.remove(TELEMETRY_FILE);
  if (telFile.open(TELEMETRY_FILE, FILE_O_WRITE)) {
    telFile.write((const uint8_t*)&snapshot, sizeof(snapshot));
    telFile.close();
  }
}

void markTelemetryChanged(uint8_t eventCode, uint8_t scenarioHint, bool flushSoon) {
  lastTelemetryEvent = eventCode;
  lastTelemetryScenario = scenarioHint;
  telemetryUpdated = true;
  telemetryDirty = true;

  uint32_t now = millis();
  if (flushSoon || nextTelemetryFlushMs == 0) {
    nextTelemetryFlushMs = flushSoon ? now : now + TELEMETRY_FLUSH_INTERVAL_MS;
  }
}

void markTelemetryEvent(uint8_t eventCode, uint8_t scenarioHint) {
  lastTelemetryEvent = eventCode;
  lastTelemetryScenario = scenarioHint;
  telemetryUpdated = true;
}

uint32_t remainingMs(uint32_t now, uint32_t target) {
  if (target == 0 || timeReached(now, target)) return 0;
  return target - now;
}

void populateTelemetryPayload() {
  uint32_t now = millis();
  memset(&telPayload, 0, sizeof(telPayload));
  telPayload.version = TELEMETRY_VERSION;
  telPayload.cameraState = (uint8_t)cameraState;
  telPayload.flags = (activityActive ? 0x01 : 0x00)
                   | (hpOutAsserted  ? 0x02 : 0x00);
  telPayload.framesFiredThisSequence = framesFired;
  telPayload.sequencesStartedThisActivity = sequencesStartedThisActivity;
  telPayload.lastEventCode = lastTelemetryEvent;
  telPayload.lastScenarioHint = lastTelemetryScenario;
  telPayload.msUntilWakeDeadline = remainingMs(now, wakeHoldDeadlineMs);
  telPayload.msUntilFpIgnoreClear = remainingMs(now, fullPressIgnoreUntilMs);
  telPayload.msUntilNextFrame = remainingMs(now, nextFrameMs);
  telPayload.msUntilPostHoldEnd = remainingMs(now, postShutterHoldUntilMs);
  noInterrupts();
  memcpy(&telPayload.counters, &telCounters, sizeof(telCounters));
  interrupts();
}

void populateTelemetryCharacteristic() {
  populateTelemetryPayload();
  chrTelemetry.write((const uint8_t*)&telPayload, sizeof(telPayload));
}

void flushTelemetryIfDue(uint32_t now) {
  if (!telemetryDirty) return;
  if (nextTelemetryFlushMs != 0 && !timeReached(now, nextTelemetryFlushMs)) return;
  telemetryDirty = false;
  nextTelemetryFlushMs = 0;
  saveTelemetry();
}

// ═════════════════════════════════════════════════════════════════════════════
// GATT setup
// ═════════════════════════════════════════════════════════════════════════════

void populateCharacteristics() {
  size_t nameLen      = strlen(cfg.name);
  size_t groupNameLen = strlen(cfg.groupName);

  chrName.write(cfg.name, nameLen > 0 ? nameLen : 1);
  chrGroupId.write8(cfg.groupId);
  chrGroupName.write(cfg.groupName, groupNameLen > 0 ? groupNameLen : 1);
  chrDevType.write8(cfg.deviceType);
  chrChemistry.write8(cfg.chemistry);
  chrCellCount.write8(cfg.cellCount);
  chrShutter.write32(cfg.shutterCount);
}

void populateCameraCharacteristics() {
  chrCamCfg.write((const uint8_t*)&camCfg, sizeof(camCfg));
}

void setupTelemetryGatt() {
  // ── Camera Telemetry (R + Notify, fixed binary payload) ──────────────────
  chrTelemetry.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrTelemetry.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  chrTelemetry.setFixedLen(sizeof(CameraTelemetryPayload));
  chrTelemetry.begin();
}

void setupCameraGatt() {
  // ── Camera Config (R/W, fixed 20 bytes) ──────────────────────────────────
  chrCamCfg.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrCamCfg.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrCamCfg.setFixedLen(sizeof(CameraConfig));
  chrCamCfg.setWriteCallback(onCamCfgWrite);
  chrCamCfg.begin();
}

void setupGatt() {
  svc.begin();

  // ── Device Name (R/W, up to 20 UTF-8 bytes) ──────────────────────────────
  chrName.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrName.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrName.setMaxLen(20);
  chrName.setWriteCallback(onNameWrite);
  chrName.begin();

  // ── Group ID (R/W, 1 byte: 0=no group, 1–255=group) ──────────────────────
  chrGroupId.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrGroupId.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrGroupId.setFixedLen(1);
  chrGroupId.setWriteCallback(onGroupIdWrite);
  chrGroupId.begin();

  // ── Group Name (R/W, up to 20 bytes) ─────────────────────────────────────
  chrGroupName.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrGroupName.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrGroupName.setMaxLen(20);
  chrGroupName.setWriteCallback(onGroupNameWrite);
  chrGroupName.begin();

  // ── Device Type (R/W, 1 byte) ─────────────────────────────────────────────
  chrDevType.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrDevType.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrDevType.setFixedLen(1);
  chrDevType.setWriteCallback(onDevTypeWrite);
  chrDevType.begin();

  // ── Battery Chemistry (R/W, 1 byte) ──────────────────────────────────────
  chrChemistry.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrChemistry.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrChemistry.setFixedLen(1);
  chrChemistry.setWriteCallback(onChemWrite);
  chrChemistry.begin();

  // ── Cell Count (R/W, 1 byte: 1–8) ────────────────────────────────────────
  chrCellCount.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrCellCount.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrCellCount.setFixedLen(1);
  chrCellCount.setWriteCallback(onCellWrite);
  chrCellCount.begin();

  // ── Shutter Count (Read + Notify, 4 bytes little-endian uint32) ──────────
  chrShutter.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrShutter.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  chrShutter.setFixedLen(4);
  chrShutter.begin();

  // ── Reset Shutter Count (Write only; write 0x01 to reset to 0) ───────────
  chrReset.setProperties(CHR_PROPS_WRITE);
  chrReset.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrReset.setFixedLen(1);
  chrReset.setWriteCallback(onResetWrite);
  chrReset.begin();

  // ── Factory Reset (Write only; write 0x01 to clear flash) ────────────────
  chrFactory.setProperties(CHR_PROPS_WRITE);
  chrFactory.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrFactory.setFixedLen(1);
  chrFactory.setWriteCallback(onFactoryWrite);
  chrFactory.begin();

  // ── Camera Config (R/W, 20 bytes) ────────────────────────────────────────
  setupCameraGatt();
  setupTelemetryGatt();

  // Seed characteristics with values loaded from flash
  populateCharacteristics();
  populateCameraCharacteristics();
  populateTelemetryCharacteristic();
}

// ═════════════════════════════════════════════════════════════════════════════
// Advertising – 13-byte manufacturer-specific packet
// Offset after company ID (Android data[]):
//   [0]    Internal battery %
//   [1-2]  Internal voltage mV LE
//   [3]    External battery % (0xFF = not present)
//   [4-5]  External voltage mV LE (0xFFFF = not present)
//   [6]    Flags (configured | deviceType | chemistry)
//   [7]    Group ID
//   [8]    Cell count
//   [9-10] Shutter count LE uint16
// ═════════════════════════════════════════════════════════════════════════════
void advertiseData(int intPct, float intVoltage, uint8_t extPct, uint16_t extVoltMv) {
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);

  uint8_t flags = 0;
  if (cfg.configured)           flags |= 0x01;
  flags |= (cfg.deviceType & 0x03) << 1;
  flags |= (cfg.chemistry  & 0x03) << 3;

  uint16_t intVoltMv = (uint16_t)(intVoltage * 1000.0f);

  uint8_t mfgData[13];
  mfgData[0]  = 0xFF;                              // Company ID low
  mfgData[1]  = 0xFF;                              // Company ID high
  mfgData[2]  = (uint8_t)intPct;                  // Internal battery %
  mfgData[3]  =  intVoltMv       & 0xFF;           // Internal voltage low
  mfgData[4]  = (intVoltMv >> 8) & 0xFF;           // Internal voltage high
  mfgData[5]  = extPct;                             // External battery % (0xFF=N/A)
  mfgData[6]  =  extVoltMv       & 0xFF;           // External voltage low
  mfgData[7]  = (extVoltMv >> 8) & 0xFF;           // External voltage high
  mfgData[8]  = flags;                              // Flags
  mfgData[9]  = cfg.groupId;                       // Group ID
  mfgData[10] = cfg.cellCount;                     // Cell count
  mfgData[11] =  cfg.shutterCount        & 0xFF;   // Shutter count low
  mfgData[12] = (cfg.shutterCount >> 8)  & 0xFF;   // Shutter count high

  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, mfgData, 13);

  Bluefruit.ScanResponse.clearData();
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.setInterval(32, 32);
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.start(0);
}

// ═════════════════════════════════════════════════════════════════════════════
// BLE connection callbacks
// ═════════════════════════════════════════════════════════════════════════════
void onConnect(uint16_t connHdl) {
  (void)connHdl;
  isConnected = true;
  telemetryUpdated = true;
}

void onDisconnect(uint16_t connHdl, uint8_t reason) {
  (void)connHdl; (void)reason;
  isConnected = false;
  if (telemetryDirty) {
    telemetryDirty = false;
    nextTelemetryFlushMs = 0;
    saveTelemetry();
  }
}

// ═════════════════════════════════════════════════════════════════════════════
// GATT write callbacks
// ═════════════════════════════════════════════════════════════════════════════
static void markConfigured() { cfg.configured = 1; }

void onNameWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  uint16_t len = (l < 20) ? l : 20;
  memcpy(cfg.name, d, len);
  cfg.name[len] = '\0';
  markConfigured();
  saveSettings();
  Bluefruit.setName(cfg.name);
}

void onGroupIdWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1) return;
  cfg.groupId = d[0];
  markConfigured();
  saveSettings();
}

void onGroupNameWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  uint16_t len = (l < 20) ? l : 20;
  memcpy(cfg.groupName, d, len);
  cfg.groupName[len] = '\0';
  markConfigured();
  saveSettings();
}

void onDevTypeWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1) return;
  cfg.deviceType = d[0];
  markConfigured();
  saveSettings();
}

void onChemWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1) return;
  cfg.chemistry = d[0];
  markConfigured();
  saveSettings();
}

void onCellWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1) return;
  uint8_t val = d[0];
  if (val < 1) val = 1;
  if (val > 8) val = 8;
  cfg.cellCount = val;
  markConfigured();
  saveSettings();
}

void onResetWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1 || d[0] != 0x01) return;
  cfg.shutterCount = 0;
  saveSettings();
  chrShutter.notify32(0);
}

void onFactoryWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 1 || d[0] != 0x01) return;
  resetToDefaults();
  resetTelemetryCounters();
  InternalFS.remove(TELEMETRY_FILE);
  populateCharacteristics();
  populateTelemetryCharacteristic();
  Bluefruit.setName("Camtraptions Device");
}

void onCamCfgWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < sizeof(camCfg)) return;
  memcpy(&camCfg, d, sizeof(camCfg));
  camCfg.version = CAMERA_SETTINGS_VERSION;  // enforce version
  // Clamp critical range fields
  if (camCfg.frameCount < 1)       camCfg.frameCount = 1;
  if (camCfg.frameCount > 8)       camCfg.frameCount = 8;
  if (camCfg.maxSequenceCount < 1) camCfg.maxSequenceCount = 1;
  if (camCfg.maxSequenceCount > 8) camCfg.maxSequenceCount = 8;
  if (camCfg.hpDebounceMs < 1) camCfg.hpDebounceMs = 1;
  if (camCfg.fpDebounceMs < 1) camCfg.fpDebounceMs = 1;
  if (camCfg.fullPressIgnoreGapTenths < 5) camCfg.fullPressIgnoreGapTenths = 5;
  saveCameraSettings();
}

// ═════════════════════════════════════════════════════════════════════════════
// FP_IN ISR (shutter pulse) – fires on FALLING edge of FP_IN_PIN
// Debounced in software. Does NOT call BLE stack (not ISR-safe).
// ═════════════════════════════════════════════════════════════════════════════
void onShutterPulse() {
  uint32_t now = millis();
  uint32_t debounceMs = (cfg.deviceType == 1 && camCfg.enabled)
                      ? camCfg.fpDebounceMs
                      : SHUTTER_DEBOUNCE_MS;
  if (now - lastShutterMs < debounceMs) {
    if (cfg.deviceType == 1 && camCfg.enabled) {
      telCounters.fpDebounceRejectCount++;
      markTelemetryChanged(TEL_EVT_FP_DEBOUNCE_REJECT, TEL_SC_DEBOUNCE, false);
    }
    return;
  }
  lastShutterMs  = now;
  cfg.shutterCount++;
  settingsDirty  = true;
  shutterUpdated = true;
  fpPulseFlag    = true;  // camera logic consumes this in main loop
}

// ═════════════════════════════════════════════════════════════════════════════
// HP_IN ISR – fires on FALLING edge of HP_IN_PIN (state-machine mode)
// ═════════════════════════════════════════════════════════════════════════════
void onHpPulse() {
  uint32_t now = millis();
  if (now - lastHpMs < camCfg.hpDebounceMs) {
    telCounters.hpDebounceRejectCount++;
    markTelemetryChanged(TEL_EVT_HP_DEBOUNCE_REJECT, TEL_SC_DEBOUNCE, false);
    return;
  }
  lastHpMs    = now;
  hpPulseFlag = true;
}

// ═════════════════════════════════════════════════════════════════════════════
// Pass-through ISRs – used when camCfg.enabled == 0 (CHANGE trigger).
// Outputs are pre-configured as OUTPUT in setup(), so digitalWrite() is safe
// here without any pinMode() call.
// FP: also counts shutter actuations on the active (LOW) edge.
// ═════════════════════════════════════════════════════════════════════════════
void onFpPassthrough() {
  int state = digitalRead(FP_IN_PIN);
  digitalWrite(FP_OUT_PIN, state);          // mirror immediately

  if (state == LOW) {                        // active edge only
    uint32_t now = millis();
    if (now - lastShutterMs >= SHUTTER_DEBOUNCE_MS) {
      lastShutterMs  = now;
      cfg.shutterCount++;
      settingsDirty  = true;
      shutterUpdated = true;
    }
  }
}

void onHpPassthrough() {
  digitalWrite(HP_OUT_PIN, digitalRead(HP_IN_PIN));  // mirror immediately
}

// ═════════════════════════════════════════════════════════════════════════════
// Open-drain output helpers
// Assert: drive LOW. Release: float (INPUT / high-Z).
// ═════════════════════════════════════════════════════════════════════════════
void assertPin(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void releasePin(int pin) {
  pinMode(pin, INPUT);
}

void assertHpOut(uint32_t now) {
  if (!hpOutAsserted) {
    hpAssertedMs = now;
  }
  assertPin(HP_OUT_PIN);
  hpOutAsserted = true;
}

void releaseHpOut() {
  releasePin(HP_OUT_PIN);
  hpOutAsserted = false;
}

bool timeReached(uint32_t now, uint32_t target) {
  return (int32_t)(now - target) >= 0;
}

uint32_t minHalfPressMs() {
  return (uint32_t)camCfg.minHalfPressBeforeShutter * 100UL;
}

uint32_t shutterPulseMs() {
  return (uint32_t)camCfg.shutterPulseDuration * 10UL;
}

uint32_t startFrameSpacingMs() {
  return (uint32_t)camCfg.startFrameSpacingTenths * 100UL;
}

uint32_t postShutterHoldMs() {
  return (uint32_t)camCfg.postShutterHpHoldTenths * 100UL;
}

uint32_t wakeHalfPressHoldMs() {
  return (uint32_t)camCfg.wakeHalfPressHoldSec * 1000UL;
}

uint32_t fullPressIgnoreGapMs() {
  return (uint32_t)camCfg.fullPressIgnoreGapTenths * 100UL;
}

bool hpLeadSatisfied(uint32_t now) {
  return hpOutAsserted && (now - hpAssertedMs >= minHalfPressMs());
}

bool underSequenceCap() {
  return camCfg.maxSequenceCount == 0
      || sequencesStartedThisActivity < camCfg.maxSequenceCount;
}

bool tryAcceptFp(uint32_t now) {
  if (!timeReached(now, fullPressIgnoreUntilMs)) {
    telCounters.ignoredFpDuringGapCount++;
    markTelemetryChanged(TEL_EVT_FP_REJECT_GAP, TEL_SC_FP_GAP_IGNORE, false);
    return false;
  }
  if (!underSequenceCap()) {
    telCounters.rejectedFpAtSequenceCapCount++;
    markTelemetryChanged(TEL_EVT_FP_REJECT_CAP, TEL_SC_SEQUENCE_CAP, false);
    return false;
  }
  return true;
}

void startSequence(uint32_t now) {
  bool wasColdFp = coldFpAcceptPending;

  activityActive = true;
  coldFpAcceptPending = false;
  sequencesStartedThisActivity++;
  telCounters.acceptedFpCount++;
  if (wasColdFp) {
    telCounters.coldFpSequenceCount++;
  }
  markTelemetryChanged(TEL_EVT_FP_ACCEPTED,
                       wasColdFp ? TEL_SC_COLD_FP : TEL_SC_NONE,
                       false);

  sequenceStartMs = now;
  fullPressIgnoreUntilMs = now + fullPressIgnoreGapMs();
  wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();

  framesFired = 0;
  fpOutReleaseMs = 0;
  lastFpOutStartMs = 0;
  nextFrameMs = now;
  cameraState = CAM_BURST_ACTIVE;
  cameraLogicActive = true;
}

void endActivity() {
  bool hadActivity = activityActive || hpOutAsserted || cameraState != CAM_IDLE;

  releasePin(FP_OUT_PIN);
  releaseHpOut();
  activityActive = false;
  coldFpAcceptPending = false;
  cameraLogicActive = false;
  cameraState = CAM_IDLE;
  framesFired = 0;
  sequencesStartedThisActivity = 0;
  fpOutReleaseMs = 0;
  nextFrameMs = 0;
  lastFpOutStartMs = 0;
  postShutterHoldUntilMs = 0;
  fullPressIgnoreUntilMs = 0;

  if (hadActivity) {
    telCounters.activityCompletedCount++;
    if (lastTelemetryEvent == TEL_EVT_WAKE_TIMEOUT) {
      markTelemetryChanged(TEL_EVT_WAKE_TIMEOUT, TEL_SC_WAKE_TIMEOUT, true);
    } else {
      markTelemetryChanged(TEL_EVT_ACTIVITY_END, TEL_SC_NONE, true);
    }
  } else {
    markTelemetryEvent(TEL_EVT_ACTIVITY_END, TEL_SC_NONE);
  }
}

void handleFpAfterCap() {
  if (camCfg.fpAfterMaxSeqCountPolicy == 1) {
    endActivity();
  }
}

void runBurstScheduler(uint32_t now) {
  if (fpOutReleaseMs != 0 && timeReached(now, fpOutReleaseMs)) {
    releasePin(FP_OUT_PIN);
    fpOutReleaseMs = 0;
  }

  if (fpOutReleaseMs == 0 && framesFired < camCfg.frameCount && timeReached(now, nextFrameMs)) {
    if (!hpOutAsserted) {
      assertHpOut(now);
      nextFrameMs = now + minHalfPressMs();
      return;
    }

    if (!hpLeadSatisfied(now)) {
      nextFrameMs = hpAssertedMs + minHalfPressMs();
      return;
    }

    assertPin(FP_OUT_PIN);
    lastFpOutStartMs = now;
    fpOutReleaseMs = now + shutterPulseMs();
    framesFired++;
    nextFrameMs = now + startFrameSpacingMs();
  }

  if (framesFired >= camCfg.frameCount && fpOutReleaseMs == 0) {
    telCounters.sequenceCompletedCount++;
    markTelemetryChanged(TEL_EVT_BURST_COMPLETE, TEL_SC_NONE, false);
    postShutterHoldUntilMs = now + postShutterHoldMs();
    cameraState = CAM_POST_SHUTTER_EXT;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
// Camera logic state machine – called from loop() when deviceType==CAMERA
// ═════════════════════════════════════════════════════════════════════════════
void processCameraLogic() {
  uint32_t now = millis();

  // Consume ISR flags atomically
  bool hpTrig = false, fpTrig = false;
  noInterrupts();
  if (hpPulseFlag) { hpPulseFlag = false; hpTrig = true; }
  if (fpPulseFlag) { fpPulseFlag = false; fpTrig = true; }
  interrupts();

  switch (cameraState) {

    // ── IDLE ──────────────────────────────────────────────────────────────────
    case CAM_IDLE:
      cameraLogicActive = false;

      if (hpTrig) {
        assertHpOut(now);
        activityActive = false;
        sequencesStartedThisActivity = 0;
        wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
        fullPressIgnoreUntilMs = 0;
        cameraState = CAM_WAKE_AF;
        cameraLogicActive = true;
        markTelemetryEvent(TEL_EVT_HP_WAKE, TEL_SC_NONE);
      } else if (fpTrig && camCfg.fullPressWithoutHpPolicy == 0) {
        assertHpOut(now);
        activityActive = true;
        sequencesStartedThisActivity = 0;
        wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
        fullPressIgnoreUntilMs = 0;
        coldFpAcceptPending = true;
        cameraState = CAM_COLD_FP_WAIT;
        cameraLogicActive = true;
        markTelemetryEvent(TEL_EVT_COLD_FP, TEL_SC_COLD_FP);
      }
      break;

    // ── WAKE_AF ───────────────────────────────────────────────────────────────
    case CAM_WAKE_AF:
      cameraLogicActive = true;

      if (hpTrig) {
        telCounters.hpRefreshCount++;
        markTelemetryChanged(TEL_EVT_HP_REFRESH, TEL_SC_NONE, false);
        switch (camCfg.wakeHoldRefreshPolicy) {
          case 0:
            wakeHoldDeadlineMs += wakeHalfPressHoldMs();
            break;                         // extend by one hold interval
          case 1:
            wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
            break;                         // restart full hold from this edge
          case 2:
            break;                         // ignoreWhileActive
        }
      }

      if (fpTrig) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        } else if (!underSequenceCap()) {
          handleFpAfterCap();
        }
        break;
      }

      if (!activityActive && timeReached(now, wakeHoldDeadlineMs)) {
        telCounters.wakeTimeoutCount++;
        markTelemetryChanged(TEL_EVT_WAKE_TIMEOUT, TEL_SC_WAKE_TIMEOUT, true);
        endActivity();
      }
      break;

    // ── COLD_FP_WAIT ──────────────────────────────────────────────────────────
    case CAM_COLD_FP_WAIT:
      cameraLogicActive = true;

      if (!coldFpAcceptPending) {
        endActivity();
        break;
      }

      if (hpLeadSatisfied(now)) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        } else {
          endActivity();
        }
      }
      break;

    // ── BURST_ACTIVE ──────────────────────────────────────────────────────────
    case CAM_BURST_ACTIVE:
      cameraLogicActive = true;

      // R14: HP input during burst is independent of scheduling; FP is ignored by R10.
      if (hpTrig) {
        telCounters.hpIgnoredDuringBurstCount++;
        markTelemetryChanged(TEL_EVT_HP_IGNORED_BURST, TEL_SC_HP_DURING_BURST, false);
      }
      if (fpTrig) {
        telCounters.ignoredFpDuringBurstCount++;
        if (!timeReached(now, fullPressIgnoreUntilMs)) {
          telCounters.ignoredFpDuringGapCount++;
        }
        markTelemetryChanged(TEL_EVT_FP_REJECT_GAP, TEL_SC_FP_GAP_IGNORE, false);
      }
      runBurstScheduler(now);
      break;

    // ── POST_SHUTTER_EXT ──────────────────────────────────────────────────────
    case CAM_POST_SHUTTER_EXT:
      cameraLogicActive = true;

      if (fpTrig) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        } else if (!underSequenceCap()) {
          handleFpAfterCap();
        }
        break;
      }

      if (timeReached(now, postShutterHoldUntilMs)) {
        if (!underSequenceCap() || timeReached(now, wakeHoldDeadlineMs)) {
          endActivity();
        }
      }
      break;
  }
}

void idleWaitWithCameraWake(uint32_t durationMs) {
  if (!(cfg.deviceType == 1 && camCfg.enabled && camCfg.powerSaveIdleMode)) {
    delay(durationMs);
    return;
  }

  uint32_t startMs = millis();
  while (millis() - startMs < durationMs) {
    if (cameraLogicActive || hpPulseFlag || fpPulseFlag) {
      processCameraLogic();
      if (cameraLogicActive) return;
      if (hpPulseFlag || fpPulseFlag) continue;
    }
    // GPIO interrupts wake WFE immediately, avoiding the old blind 950 ms delay.
    __WFE();
  }
}

// ═════════════════════════════════════════════════════════════════════════════
// Battery reading – internal
// ═════════════════════════════════════════════════════════════════════════════
float readBatteryVoltage() {
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(10);
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(BATTERY_PIN);
    delayMicroseconds(100);
  }
  return ((float)(total / 5) / ADC_MAX_VALUE) * 3.6f * VOLTAGE_DIVIDER_RATIO;
}

// ═════════════════════════════════════════════════════════════════════════════
// Battery reading – device/camera battery (A1)
// Returns true if a battery is detected (voltage >= 0.5V).
// On return: pct = 0–100, voltMv = millivolts.
// ═════════════════════════════════════════════════════════════════════════════
bool readDeviceBattery(int &pct, float &voltMv) {
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(10);
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(DEVICE_BATTERY_PIN);
    delayMicroseconds(100);
  }
  float voltage = ((float)(total / 5) / ADC_MAX_VALUE) * 3.6f * VOLTAGE_DIVIDER_RATIO;
  if (voltage < 0.5f) return false;  // not present
  voltMv = voltage * 1000.0f;
  pct    = readLiPoPercentage(voltage);
  return true;
}

// ═════════════════════════════════════════════════════════════════════════════
// CR2032 coin cell percentage (internal battery, A0)
// Nominal: 3.0 V full, 2.5 V depleted.
// Discharge is very flat; piecewise linear approximation:
//   ≥3.0 V → 100 %   2.9 V → 80 %   2.8 V → 50 %   2.7 V → 20 %   ≤2.5 V → 0 %
// ═════════════════════════════════════════════════════════════════════════════
int readCR2032Percentage(float voltage) {
  if (voltage >= 3.0f) return 100;
  if (voltage <= 2.5f) return 0;

  float pct;
  if      (voltage >= 2.9f) pct = 80.0f + (voltage - 2.9f) / 0.1f * 20.0f;  // 2.9–3.0 V → 80–100 %
  else if (voltage >= 2.8f) pct = 50.0f + (voltage - 2.8f) / 0.1f * 30.0f;  // 2.8–2.9 V → 50–80 %
  else if (voltage >= 2.7f) pct = 20.0f + (voltage - 2.7f) / 0.1f * 30.0f;  // 2.7–2.8 V → 20–50 %
  else                      pct =          (voltage - 2.5f) / 0.2f * 20.0f;  // 2.5–2.7 V →  0–20 %

  int result = (int)pct;
  if (result > 100) result = 100;
  if (result < 0)   result = 0;
  return result;
}

// ═════════════════════════════════════════════════════════════════════════════
// LiPo percentage (external camera battery, A1)
// Operating range 3.0–4.2 V; piecewise linear approximation.
// ═════════════════════════════════════════════════════════════════════════════
int readLiPoPercentage(float voltage) {
  if (voltage > 4.2f) voltage = 4.2f;
  if (voltage < 3.0f) voltage = 3.0f;

  float pct;
  if      (voltage >= 4.1f) pct = 90.0f + (voltage - 4.1f) * 100.0f;
  else if (voltage >= 3.9f) pct = 70.0f + (voltage - 3.9f) * 100.0f;
  else if (voltage >= 3.7f) pct = 40.0f + (voltage - 3.7f) * 150.0f;
  else if (voltage >= 3.5f) pct = 20.0f + (voltage - 3.5f) * 100.0f;
  else if (voltage >= 3.3f) pct =  5.0f + (voltage - 3.3f) *  75.0f;
  else                      pct =          (voltage - 3.0f) *  16.67f;

  int result = (int)pct;
  if (result > 100) result = 100;
  if (result < 0)   result = 0;
  return result;
}
