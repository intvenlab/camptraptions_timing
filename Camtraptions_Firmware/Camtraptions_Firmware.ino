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

#include "battery.h"
#include "gatt.h"
#include "camera.h"
#include "storage.h"

using namespace Adafruit_LittleFS_Namespace;

// Uncomment in config.h to emit state-machine pin transition logs over USB serial.

void setup() {
#ifdef DEBUG_CAMERA_LOGIC_PINS
  Serial.begin(115200);
  uint32_t serialWaitStart = millis();
  while (!Serial && millis() - serialWaitStart < 1000) {
    delay(5);
  }
#endif

  batteryInit();

  pinMode(LED_RED,   OUTPUT); digitalWrite(LED_RED,   HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE,  OUTPUT); digitalWrite(LED_BLUE,  HIGH);

  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

  InternalFS.begin();
  loadSettings();
  loadCameraSettings();
  loadTelemetry();
  telCounters.bootCount++;
  saveTelemetry();
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

  if (cameraModeEnabled && cameraControlWorkPending()) {
    processCameraLogic();
  }

  applyPendingCamCfgIfIdle();
  batteryApplyPending();

  if (cameraModeEnabled && !cameraControlWorkPending()) {
    processCameraLogic();
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
  idleWaitWithCameraWake(ADVERTISING_DURATION_MS);
  Bluefruit.Advertising.stop();

  if (settingsDirty) {
    saveSettings();
    settingsDirty = false;
  }
  flushTelemetryIfDue(millis());

  idleWaitWithCameraWake(SLEEP_INTERVAL_MS - ADVERTISING_DURATION_MS);
}
