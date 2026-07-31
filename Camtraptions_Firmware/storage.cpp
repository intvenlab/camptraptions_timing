#include "storage.h"

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

DeviceConfig cfg;
CameraConfig camCfg;
FeederConfig feederCfg;
CameraTelemetryCounters telCounters;
CameraTelemetryPayload telPayload;

volatile bool telemetryDirty  = false;
volatile bool telemetryUpdated = false;

uint32_t nextTelemetryFlushMs = 0;
uint8_t  lastTelemetryEvent = TEL_EVT_NONE;
uint8_t  lastTelemetryScenario = TEL_SC_NONE;
uint16_t lastBootResetRaw = 0;
int16_t  lastBootTempCx100 = 0;

static File cfgFile(InternalFS);
static File camFile(InternalFS);
static File feederFile(InternalFS);
static File telFile(InternalFS);

void loadSettings() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.version   = SETTINGS_VERSION;
  cfg.deviceType = 1;
  cfg.cellCount = 1;

  if (InternalFS.exists(SETTINGS_FILE)) {
    if (cfgFile.open(SETTINGS_FILE, FILE_O_READ)) {
      cfgFile.read(&cfg, sizeof(cfg));
      cfgFile.close();
    }

    if (cfg.version != SETTINGS_VERSION) {
      memset(&cfg, 0, sizeof(cfg));
      cfg.version   = SETTINGS_VERSION;
      cfg.deviceType = 1;
      cfg.cellCount = 1;
    }
  }

#if USE_HARDCODED_DEVICE_IDENTITY
  // Always wins, regardless of what flash produced above (or whether a settings
  // file existed at all) -- see the HARD-CODED DEVICE IDENTITY block in config.h.
  cfg.configured = 1;
  strncpy(cfg.name, HARDCODED_DEVICE_NAME, sizeof(cfg.name) - 1);
  cfg.name[sizeof(cfg.name) - 1] = '\0';
  cfg.deviceType = HARDCODED_DEVICE_TYPE;
  cfg.cellCount  = HARDCODED_CELL_COUNT;
  cfg.groupId    = HARDCODED_KIT_NUMBER;
#endif
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
  cfg.deviceType = 1;
  cfg.cellCount = 1;
  InternalFS.remove(SETTINGS_FILE);
}

void resetCameraToDefaults() {
  memset(&camCfg, 0, sizeof(camCfg));
  camCfg.version                   = CAMERA_SETTINGS_VERSION;
  camCfg.enabled                   = 1;
  camCfg.wakeHalfPressHoldSec      = 10;
  camCfg.minHalfPressBeforeShutter = 5;
  camCfg.shutterPulseDuration      = 20;
  camCfg.startFrameSpacingTicks    = 100;
  camCfg.postShutterHpHoldTenths   = 20;
  camCfg.hpDebounceMs              = 35;
  camCfg.fpDebounceMs              = 20;
  camCfg.frameCount                = 4;
  camCfg.maxSequenceCount          = 4;
  camCfg.powerSaveIdleMode         = 1;
  camCfg.fullPressIgnoreGapTenths  = 50;
}

void loadCameraSettings() {
  resetCameraToDefaults();
  if (!InternalFS.exists(CAMERA_SETTINGS_FILE)) return;
  if (camFile.open(CAMERA_SETTINGS_FILE, FILE_O_READ)) {
    camFile.read(&camCfg, sizeof(camCfg));
    camFile.close();
  }
  if (camCfg.version != CAMERA_SETTINGS_VERSION) {
    resetCameraToDefaults();
  } else {
    sanitizeCameraConfig(camCfg);
  }
}

void saveCameraSettings() {
  InternalFS.remove(CAMERA_SETTINGS_FILE);
  if (camFile.open(CAMERA_SETTINGS_FILE, FILE_O_WRITE)) {
    camFile.write((const uint8_t*)&camCfg, sizeof(camCfg));
    camFile.close();
  }
}

void sanitizeCameraConfig(CameraConfig &cfgToSanitize) {
  cfgToSanitize.version = CAMERA_SETTINGS_VERSION;
  cfgToSanitize.enabled = cfgToSanitize.enabled ? 1 : 0;

  if (cfgToSanitize.wakeHalfPressHoldSec < 1)  cfgToSanitize.wakeHalfPressHoldSec = 1;
  if (cfgToSanitize.wakeHalfPressHoldSec > 60) cfgToSanitize.wakeHalfPressHoldSec = 60;

  if (cfgToSanitize.minHalfPressBeforeShutter < 1)   cfgToSanitize.minHalfPressBeforeShutter = 1;
  if (cfgToSanitize.minHalfPressBeforeShutter > 100) cfgToSanitize.minHalfPressBeforeShutter = 100;

  if (cfgToSanitize.shutterPulseDuration < 1)    cfgToSanitize.shutterPulseDuration = 1;
  if (cfgToSanitize.shutterPulseDuration > 3000) cfgToSanitize.shutterPulseDuration = 3000;

  if (cfgToSanitize.startFrameSpacingTicks < 1)    cfgToSanitize.startFrameSpacingTicks = 1;
  if (cfgToSanitize.startFrameSpacingTicks > 3000) cfgToSanitize.startFrameSpacingTicks = 3000;

  if (cfgToSanitize.postShutterHpHoldTenths < 1)   cfgToSanitize.postShutterHpHoldTenths = 1;
  if (cfgToSanitize.postShutterHpHoldTenths > 200) cfgToSanitize.postShutterHpHoldTenths = 200;

  if (cfgToSanitize.hpDebounceMs < 1)   cfgToSanitize.hpDebounceMs = 1;
  if (cfgToSanitize.hpDebounceMs > 250) cfgToSanitize.hpDebounceMs = 250;
  if (cfgToSanitize.fpDebounceMs < 1)   cfgToSanitize.fpDebounceMs = 1;
  if (cfgToSanitize.fpDebounceMs > 250) cfgToSanitize.fpDebounceMs = 250;

  if (cfgToSanitize.frameCount < 1) cfgToSanitize.frameCount = 1;
  if (cfgToSanitize.frameCount > 8) cfgToSanitize.frameCount = 8;
  if (cfgToSanitize.maxSequenceCount < 1) cfgToSanitize.maxSequenceCount = 1;
  if (cfgToSanitize.maxSequenceCount > 64) cfgToSanitize.maxSequenceCount = 64;

  if (cfgToSanitize.wakeHoldRefreshPolicy > 2) cfgToSanitize.wakeHoldRefreshPolicy = 0;
  if (cfgToSanitize.halfPressDuringBurstPolicy != 0) cfgToSanitize.halfPressDuringBurstPolicy = 0;
  if (cfgToSanitize.fullPressWithoutHpPolicy > 1) cfgToSanitize.fullPressWithoutHpPolicy = 0;
  if (cfgToSanitize.activityHalfPressHoldPolicy != 0) cfgToSanitize.activityHalfPressHoldPolicy = 0;
  if (cfgToSanitize.fpAfterMaxSeqCountPolicy > 1) cfgToSanitize.fpAfterMaxSeqCountPolicy = 0;

  cfgToSanitize.inputActivePolarity = 0;
  cfgToSanitize.outputDriveMode = 0;

  cfgToSanitize.powerSaveIdleMode = cfgToSanitize.powerSaveIdleMode ? 1 : 0;
  if (cfgToSanitize.fullPressIgnoreGapTenths < 5)   cfgToSanitize.fullPressIgnoreGapTenths = 5;
  if (cfgToSanitize.fullPressIgnoreGapTenths > 250) cfgToSanitize.fullPressIgnoreGapTenths = 250;
}

bool cameraConfigHasInvalidValues(const CameraConfig &cfgToCheck) {
  CameraConfig sanitized = cfgToCheck;
  sanitizeCameraConfig(sanitized);
  return memcmp(&cfgToCheck, &sanitized, sizeof(CameraConfig)) != 0;
}

void resetFeederToDefaults() {
  memset(&feederCfg, 0, sizeof(feederCfg));
  feederCfg.version           = FEEDER_SETTINGS_VERSION;
  feederCfg.enabled           = 1;
  feederCfg.pulseStretchMinMs = 100;
  feederCfg.pumpOnMs          = 2000;
  feederCfg.pumpOffMs         = 10000;
}

void loadFeederSettings() {
  resetFeederToDefaults();
  if (!InternalFS.exists(FEEDER_SETTINGS_FILE)) return;
  if (feederFile.open(FEEDER_SETTINGS_FILE, FILE_O_READ)) {
    feederFile.read(&feederCfg, sizeof(feederCfg));
    feederFile.close();
  }
  if (feederCfg.version != FEEDER_SETTINGS_VERSION) {
    resetFeederToDefaults();
  } else {
    sanitizeFeederConfig(feederCfg);
  }
}

void saveFeederSettings() {
  InternalFS.remove(FEEDER_SETTINGS_FILE);
  if (feederFile.open(FEEDER_SETTINGS_FILE, FILE_O_WRITE)) {
    feederFile.write((const uint8_t*)&feederCfg, sizeof(feederCfg));
    feederFile.close();
  }
}

void sanitizeFeederConfig(FeederConfig &cfgToSanitize) {
  cfgToSanitize.version = FEEDER_SETTINGS_VERSION;
  cfgToSanitize.enabled = cfgToSanitize.enabled ? 1 : 0;

  if (cfgToSanitize.pulseStretchMinMs < 10)    cfgToSanitize.pulseStretchMinMs = 10;
  if (cfgToSanitize.pulseStretchMinMs > 60000) cfgToSanitize.pulseStretchMinMs = 60000;

  if (cfgToSanitize.pumpOnMs < 50)      cfgToSanitize.pumpOnMs = 50;
  if (cfgToSanitize.pumpOnMs > 3600000) cfgToSanitize.pumpOnMs = 3600000;

  if (cfgToSanitize.pumpOffMs < 50)      cfgToSanitize.pumpOffMs = 50;
  if (cfgToSanitize.pumpOffMs > 3600000) cfgToSanitize.pumpOffMs = 3600000;
}

bool feederConfigHasInvalidValues(const FeederConfig &cfgToCheck) {
  FeederConfig sanitized = cfgToCheck;
  sanitizeFeederConfig(sanitized);
  return memcmp(&cfgToCheck, &sanitized, sizeof(FeederConfig)) != 0;
}

void resetTelemetryCounters() {
  uint32_t preservedBootCount = telCounters.bootCount;
  memset(&telCounters, 0, sizeof(telCounters));
  telCounters.version = TELEMETRY_VERSION;
  telCounters.bootCount = preservedBootCount;
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
  uint32_t msUntilGapClear = remainingMs(now, fullPressIgnoreUntilMs);
  uint32_t msUntilCapTimeoutClear = remainingMs(now, maxSequenceTimeoutUntilMs);
  telPayload.msUntilFpIgnoreClear = msUntilGapClear > msUntilCapTimeoutClear
                                  ? msUntilGapClear
                                  : msUntilCapTimeoutClear;
  telPayload.msUntilNextFrame = remainingMs(now, nextFrameMs);
  telPayload.msUntilPostHoldEnd = remainingMs(now, postShutterHoldUntilMs);
  noInterrupts();
  memcpy(&telPayload.counters, &telCounters, sizeof(telCounters));
  interrupts();
  telPayload.bootResetRaw = lastBootResetRaw;
  telPayload.bootTempCx100 = lastBootTempCx100;
}

void flushTelemetryIfDue(uint32_t now) {
  if (!telemetryDirty) return;
  if (nextTelemetryFlushMs != 0 && !timeReached(now, nextTelemetryFlushMs)) return;
  telemetryDirty = false;
  nextTelemetryFlushMs = 0;
  saveTelemetry();
}
