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
 *
 * Source modules: config.h, build_info.h, battery.*, storage.*, camera.*, gatt.*
 */

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <stdio.h>

#include "battery.h"
#include "build_info.h"
#include "gatt.h"
#include "camera.h"
#include "feeder.h"
#include "storage.h"

using namespace Adafruit_LittleFS_Namespace;

static void appendResetReasons(uint32_t rawReason, char* out, size_t outLen) {
  bool first = true;
  auto append = [&](const char* token) {
    if (outLen == 0) return;
    if (!first) {
      strncat(out, "|", outLen - strlen(out) - 1);
    }
    strncat(out, token, outLen - strlen(out) - 1);
    first = false;
  };

  if (rawReason & POWER_RESETREAS_RESETPIN_Msk) append("pin");
  if (rawReason & POWER_RESETREAS_DOG_Msk)      append("watchdog");
  if (rawReason & POWER_RESETREAS_SREQ_Msk)     append("soft");
  if (rawReason & POWER_RESETREAS_LOCKUP_Msk)   append("lockup");
  if (rawReason & POWER_RESETREAS_OFF_Msk)      append("off");
  if (rawReason & POWER_RESETREAS_LPCOMP_Msk)   append("lpcomp");
  if (rawReason & POWER_RESETREAS_DIF_Msk)      append("debug");
  if (rawReason & POWER_RESETREAS_NFC_Msk)      append("nfc");
  if (first) {
    strncat(out, "power_on_or_unknown", outLen - strlen(out) - 1);
  }
}

static float readInternalTempC() {
  NRF_TEMP->TASKS_START = 1;
  while (NRF_TEMP->EVENTS_DATARDY == 0) {
    // Wait for hardware sample.
  }
  NRF_TEMP->EVENTS_DATARDY = 0;
  int32_t rawQuarterC = NRF_TEMP->TEMP;
  NRF_TEMP->TASKS_STOP = 1;
  return ((float)rawQuarterC) / 4.0f;
}

static int16_t tempCToCentiDegrees(float tempC) {
  float scaled = tempC * 100.0f;
  if (scaled >= 0.0f) {
    return (int16_t)(scaled + 0.5f);
  }
  return (int16_t)(scaled - 0.5f);
}

static void emitBootLine() {
  const uint32_t devId0 = NRF_FICR->DEVICEID[0];
  const uint32_t devId1 = NRF_FICR->DEVICEID[1];
  const uint32_t rawResetReason = NRF_POWER->RESETREAS;
  NRF_POWER->RESETREAS = rawResetReason;

  char resetReasons[96];
  resetReasons[0] = '\0';
  appendResetReasons(rawResetReason, resetReasons, sizeof(resetReasons));

  const float tempC = readInternalTempC();
  lastBootResetRaw = (uint16_t)(rawResetReason & 0xFFFFu);
  lastBootTempCx100 = tempCToCentiDegrees(tempC);
  Serial.print("[BOOT] bootCount=");
  Serial.print((unsigned long)telCounters.bootCount);
  Serial.print(" devId=");
  Serial.print((unsigned long)devId1, HEX);
  Serial.print((unsigned long)devId0, HEX);
  Serial.print(" tempC=");
  Serial.print(tempC, 2);
  Serial.print(" resetRaw=0x");
  Serial.print((unsigned long)rawResetReason, HEX);
  Serial.print(" resetReason=");
  Serial.print(resetReasons);
  Serial.print(" build=");
  Serial.print((unsigned int)BUILD_YEAR);
  Serial.print("-");
  if (BUILD_MONTH < 10) Serial.print("0");
  Serial.print((unsigned int)BUILD_MONTH);
  Serial.print("-");
  if (BUILD_DAY < 10) Serial.print("0");
  Serial.print((unsigned int)BUILD_DAY);
  Serial.print(" ");
  if (BUILD_HOUR < 10) Serial.print("0");
  Serial.print((unsigned int)BUILD_HOUR);
  Serial.print(":");
  if (BUILD_MIN < 10) Serial.print("0");
  Serial.print((unsigned int)BUILD_MIN);
  Serial.print(":");
  if (BUILD_SEC < 10) Serial.print("0");
  Serial.println((unsigned int)BUILD_SEC);
}

void setup() {
  Serial.begin(115200);
  uint32_t serialWaitStart = millis();
  while (!Serial && millis() - serialWaitStart < 250) {
    delay(5);
  }

  batteryInit();

  pinMode(LED_RED,   OUTPUT); digitalWrite(LED_RED,   HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE,  OUTPUT); digitalWrite(LED_BLUE,  HIGH);

  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

  InternalFS.begin();
  loadSettings();
  loadCameraSettings();
  loadFeederSettings();
  loadTelemetry();
  telCounters.bootCount++;
  saveTelemetry();
  emitBootLine();
  batteryLoadAll();

  configureRuntimeIo();

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.autoConnLed(false);
  Bluefruit.setConnLedInterval(0);

  Bluefruit.setName(cfg.configured && cfg.name[0] ? cfg.name : "Camtraptions Device");

  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);

  setupGatt();
}

void loop() {
  bool cameraModeEnabled = (cfg.deviceType == 1 /* CAMERA */ && camCfg.enabled);
  bool feederModeEnabled = feederModeActive();

  if (cameraModeEnabled && cameraControlWorkPending()) {
    processCameraLogic();
  }

  applyPendingCamCfgIfIdle();
  batteryApplyPending();

  if (cameraModeEnabled && !cameraControlWorkPending()) {
    processCameraLogic();
  }

  if (feederModeEnabled) {
    processFeederLogic();
  }

  if (isConnected) {
    serviceConnectedManagementPlane(millis());

    if (!cameraControlWorkPending()) {
      __WFE();
    }
    return;
  }

  if (cameraLogicActive) {
    delay(1);
    return;
  }

  float intVoltage = readBatteryVoltage();
  int   intPct     = readCR2032Percentage(intVoltage);

  int   extPctInt = -1;
  float extVoltMvF = 0.0f;
  bool  extPresent = readDeviceBattery(extPctInt, extVoltMvF, cfg.cellCount);
  uint8_t  extBatPct = extPresent ? (uint8_t)extPctInt    : 0xFF;
  uint16_t extBatMv  = extPresent ? (uint16_t)extVoltMvF  : 0xFFFF;

  advertiseData(intPct, intVoltage, extBatPct, extBatMv);
  idleWaitWithDeviceWake(ADVERTISING_DURATION_MS);
  Bluefruit.Advertising.stop();

  if (settingsDirty) {
    saveSettings();
    settingsDirty = false;
  }
  flushTelemetryIfDue(millis());

  idleWaitWithDeviceWake(SLEEP_INTERVAL_MS - ADVERTISING_DURATION_MS);
}
