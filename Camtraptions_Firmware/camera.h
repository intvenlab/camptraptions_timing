#pragma once

#include "config.h"

void configureRuntimeIo();
bool cameraActivityInProgress();
void applyPendingCamCfgIfIdle();
void refreshWakeHoldFromHp(uint32_t now);
bool cameraControlWorkPending();
void processCameraLogic();
void idleWaitWithDeviceWake(uint32_t durationMs);
void endActivity();

void onShutterPulse();
void onHpPulse();
void onFpPassthrough();
void onHpPassthrough();
