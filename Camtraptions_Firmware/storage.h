#pragma once

#include "config.h"

void loadSettings();
void saveSettings();
void resetToDefaults();

void loadCameraSettings();
void saveCameraSettings();
void resetCameraToDefaults();
void sanitizeCameraConfig(CameraConfig &cfgToSanitize);
bool cameraConfigHasInvalidValues(const CameraConfig &cfgToCheck);

void loadFeederSettings();
void saveFeederSettings();
void resetFeederToDefaults();
void sanitizeFeederConfig(FeederConfig &cfgToSanitize);
bool feederConfigHasInvalidValues(const FeederConfig &cfgToCheck);

void resetTelemetryCounters();
void loadTelemetry();
void saveTelemetry();
void markTelemetryChanged(uint8_t eventCode, uint8_t scenarioHint, bool flushSoon);
void markTelemetryEvent(uint8_t eventCode, uint8_t scenarioHint);
uint32_t remainingMs(uint32_t now, uint32_t target);
void populateTelemetryPayload();
void flushTelemetryIfDue(uint32_t now);
