#include "gatt.h"

#include "battery.h"
#include "build_info.h"
#include "camera.h"
#include "storage.h"

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

BLEService svc("ca500000-0000-0000-0000-000000000000");

BLECharacteristic chrName     ("ca500001-0000-0000-0000-000000000000");
BLECharacteristic chrGroupId  ("ca500002-0000-0000-0000-000000000000");
BLECharacteristic chrGroupName("ca500003-0000-0000-0000-000000000000");
BLECharacteristic chrDevType  ("ca500004-0000-0000-0000-000000000000");
BLECharacteristic chrChemistry("ca500005-0000-0000-0000-000000000000");
BLECharacteristic chrCellCount("ca500006-0000-0000-0000-000000000000");
BLECharacteristic chrShutter  ("ca500007-0000-0000-0000-000000000000");
BLECharacteristic chrReset    ("ca500008-0000-0000-0000-000000000000");
BLECharacteristic chrFactory  ("ca500009-0000-0000-0000-000000000000");
BLECharacteristic chrCamCfg   ("ca50000a-0000-0000-0000-000000000000");
BLECharacteristic chrTelemetry("ca50000b-0000-0000-0000-000000000000");
BLECharacteristic chrCalSet   ("ca50000c-0000-0000-0000-000000000000");
BLECharacteristic chrIntCalSet("ca50000d-0000-0000-0000-000000000000");
BLECharacteristic chrCamCfgStatus("ca50000e-0000-0000-0000-000000000000");

volatile bool isConnected    = false;
volatile bool settingsDirty  = false;
volatile bool shutterUpdated = false;
volatile uint32_t lastShutterMs = 0;

uint8_t camCfgWriteStatus = CAMCFG_ACK_APPLIED;

static void setupTelemetryGatt();
static void setupCameraGatt();

static bool budgetExpired(uint32_t startedUs, uint32_t budgetUs) {
  if (budgetUs == 0) return false;
  return (uint32_t)(micros() - startedUs) >= budgetUs;
}

static bool inPostSequenceWindow(uint32_t now) {
  if (lastActivityEndMs == 0) return false;
  return !timeReached(now, lastActivityEndMs + CONNECTED_POST_MODE_WINDOW_MS);
}

static uint32_t connectedTelemetryIntervalMs(uint32_t now) {
  if (cameraActivityInProgress()) {
    return CONNECTED_ACTIVE_TELEMETRY_MIN_INTERVAL_MS;
  }
  if (inPostSequenceWindow(now)) {
    return CONNECTED_POST_TELEMETRY_MIN_INTERVAL_MS;
  }
  return CONNECTED_IDLE_TELEMETRY_MIN_INTERVAL_MS;
}

static uint32_t connectedBleBudgetUs(uint32_t now) {
  if (cameraActivityInProgress()) {
    return CONNECTED_ACTIVE_BLE_BUDGET_US;
  }
  if (inPostSequenceWindow(now)) {
    return CONNECTED_POST_BLE_BUDGET_US;
  }
  return CONNECTED_IDLE_BLE_BUDGET_US;
}

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
  chrCamCfgStatus.write8(camCfgWriteStatus);
}

void populateTelemetryCharacteristic() {
  populateTelemetryPayload();
  chrTelemetry.write((const uint8_t*)&telPayload, sizeof(telPayload));
}

static void setupTelemetryGatt() {
  chrTelemetry.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrTelemetry.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  chrTelemetry.setFixedLen(sizeof(CameraTelemetryPayload));
  chrTelemetry.begin();
}

static void setupCameraGatt() {
  chrCamCfg.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrCamCfg.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrCamCfg.setFixedLen(sizeof(CameraConfig));
  chrCamCfg.setWriteCallback(onCamCfgWrite);
  chrCamCfg.begin();

  chrCamCfgStatus.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrCamCfgStatus.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  chrCamCfgStatus.setFixedLen(1);
  chrCamCfgStatus.begin();
}

void setupGatt() {
  svc.begin();

  chrName.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrName.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrName.setMaxLen(20);
  chrName.setWriteCallback(onNameWrite);
  chrName.begin();

  chrGroupId.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrGroupId.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrGroupId.setFixedLen(1);
  chrGroupId.setWriteCallback(onGroupIdWrite);
  chrGroupId.begin();

  chrGroupName.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrGroupName.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrGroupName.setMaxLen(20);
  chrGroupName.setWriteCallback(onGroupNameWrite);
  chrGroupName.begin();

  chrDevType.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrDevType.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrDevType.setFixedLen(1);
  chrDevType.setWriteCallback(onDevTypeWrite);
  chrDevType.begin();

  chrChemistry.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrChemistry.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrChemistry.setFixedLen(1);
  chrChemistry.setWriteCallback(onChemWrite);
  chrChemistry.begin();

  chrCellCount.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrCellCount.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrCellCount.setFixedLen(1);
  chrCellCount.setWriteCallback(onCellWrite);
  chrCellCount.begin();

  chrShutter.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrShutter.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  chrShutter.setFixedLen(4);
  chrShutter.begin();

  chrReset.setProperties(CHR_PROPS_WRITE);
  chrReset.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrReset.setFixedLen(1);
  chrReset.setWriteCallback(onResetWrite);
  chrReset.begin();

  chrFactory.setProperties(CHR_PROPS_WRITE);
  chrFactory.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrFactory.setFixedLen(1);
  chrFactory.setWriteCallback(onFactoryWrite);
  chrFactory.begin();

  setupCameraGatt();
  setupTelemetryGatt();

  chrCalSet.setProperties(CHR_PROPS_WRITE);
  chrCalSet.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrCalSet.setFixedLen(2);
  chrCalSet.setWriteCallback(onCalSetWrite);
  chrCalSet.begin();

  chrIntCalSet.setProperties(CHR_PROPS_WRITE);
  chrIntCalSet.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  chrIntCalSet.setFixedLen(2);
  chrIntCalSet.setWriteCallback(onIntCalSetWrite);
  chrIntCalSet.begin();

  populateCharacteristics();
  populateCameraCharacteristics();
  populateTelemetryCharacteristic();
}

void advertiseData(int intPct, float intVoltage, uint8_t extPct, uint16_t extVoltMv) {
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);

  uint8_t flags = 0;
  if (cfg.configured)           flags |= 0x01;
  flags |= (cfg.deviceType & 0x03) << 1;
  flags |= (cfg.chemistry  & 0x03) << 3;

  uint16_t intVoltMv = (uint16_t)(intVoltage * 1000.0f);

  uint8_t mfgData[23];
  mfgData[0]  = 0xFF;
  mfgData[1]  = 0xFF;
  mfgData[2]  = (uint8_t)intPct;
  mfgData[3]  =  intVoltMv       & 0xFF;
  mfgData[4]  = (intVoltMv >> 8) & 0xFF;
  mfgData[5]  = extPct;
  mfgData[6]  =  extVoltMv       & 0xFF;
  mfgData[7]  = (extVoltMv >> 8) & 0xFF;
  mfgData[8]  = flags;
  mfgData[9]  = cfg.groupId;
  mfgData[10] = cfg.cellCount;
  mfgData[11] =  cfg.shutterCount        & 0xFF;
  mfgData[12] = (cfg.shutterCount >> 8)  & 0xFF;
  mfgData[13] = BEACON_LAYOUT_VERSION;
  mfgData[14] = (uint8_t)cameraState;
  mfgData[15] = (activityActive ? 0x01 : 0x00)
              | (hpOutAsserted  ? 0x02 : 0x00);
  mfgData[16] = BUILD_YEAR & 0xFF;
  mfgData[17] = (BUILD_YEAR >> 8) & 0xFF;
  mfgData[18] = BUILD_MONTH;
  mfgData[19] = BUILD_DAY;
  mfgData[20] = BUILD_HOUR;
  mfgData[21] = BUILD_MIN;
  mfgData[22] = BUILD_SEC;

  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, mfgData, 23);

  Bluefruit.ScanResponse.clearData();
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.setInterval(32, 32);
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.start(0);
}

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

void serviceConnectedManagementPlane(uint32_t now) {
  uint32_t serviceStartedUs = micros();
  uint32_t bleBudgetUs = connectedBleBudgetUs(now);

  if (shutterUpdated) {
    if (!cameraActivityInProgress() || timeReached(now, lastTelemetryServiceMs + connectedTelemetryIntervalMs(now))) {
      shutterUpdated = false;
      chrShutter.notify32(cfg.shutterCount);
      lastTelemetryServiceMs = now;
    }
  }

  if (budgetExpired(serviceStartedUs, bleBudgetUs)) return;

  bool telemetryDue = timeReached(now, lastTelemetryServiceMs + connectedTelemetryIntervalMs(now));
  if (telemetryDue) {
    bool telemetryWasUpdated = telemetryUpdated;
    populateTelemetryCharacteristic();
    if (telemetryWasUpdated) {
      telemetryUpdated = false;
      chrTelemetry.notify((uint8_t*)&telPayload, sizeof(telPayload));
    }
    lastTelemetryServiceMs = now;
  }

  if (budgetExpired(serviceStartedUs, bleBudgetUs)) return;

  if (!cameraActivityInProgress()) {
    flushTelemetryIfDue(now);
  }
}

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
  cfg.deviceType = d[0] & 0x03;
  if (cameraActivityInProgress()) {
    endActivity();
  }
  runtimeIoReconfigurePending = true;
  markConfigured();
  saveSettings();
  populateCharacteristics();
  telemetryUpdated = true;
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
  if (cameraActivityInProgress()) {
    endActivity();
  }
  resetToDefaults();
  resetCameraToDefaults();
  resetTelemetryCounters();
  pendingCamCfgApply = false;
  runtimeIoReconfigurePending = true;
  batteryResetCalibration();
  InternalFS.remove(CAMERA_SETTINGS_FILE);
  populateCharacteristics();
  populateCameraCharacteristics();
  populateTelemetryCharacteristic();
  Bluefruit.setName("Camtraptions Device");
  saveSettings();
  saveCameraSettings();
  saveTelemetry();
}

void publishCamCfgWriteStatus(uint8_t statusCode) {
  camCfgWriteStatus = statusCode;
  chrCamCfgStatus.write8(statusCode);
  if (isConnected) {
    chrCamCfgStatus.notify(&camCfgWriteStatus, 1);
  }
}

void onCamCfgWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l != sizeof(camCfg)) {
    publishCamCfgWriteStatus(CAMCFG_NACK_BAD_FORMAT);
    return;
  }
  if (d[0] != CAMERA_SETTINGS_VERSION) {
    publishCamCfgWriteStatus(CAMCFG_NACK_BAD_FORMAT);
    return;
  }
  CameraConfig incoming;
  memcpy(&incoming, d, sizeof(incoming));
  if (cameraConfigHasInvalidValues(incoming)) {
    publishCamCfgWriteStatus(CAMCFG_NACK_OUT_OF_RANGE);
    return;
  }

  if (cameraActivityInProgress()) {
    publishCamCfgWriteStatus(CAMCFG_NACK_BUSY);
    return;
  }

  camCfg = incoming;
  saveCameraSettings();
  populateCameraCharacteristics();
  runtimeIoReconfigurePending = true;
  publishCamCfgWriteStatus(CAMCFG_ACK_APPLIED);
}
