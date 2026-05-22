#include "battery.h"

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

static float             deviceDividerRatio = EXT_BATT_DIVIDER_RATIO;
static volatile uint16_t pendingCalTargetMv = 0;
volatile bool            pendingCalApply    = false;

static float             intScaleFactor       = 1.0f;
static float             lastRawIntVoltage    = 0.0f;
static volatile uint16_t pendingIntCalTargetMv = 0;
volatile bool            pendingIntCalApply    = false;

static File calFile(InternalFS);
static File intCalFile(InternalFS);

static void loadCalibration();
static void saveCalibration();
static void applyPendingCal();
static void loadIntCalibration();
static void saveIntCalibration();
static void applyPendingIntCal();

void batteryInit() {
  pinMode(BATTERY_PIN, INPUT);
  pinMode(DEVICE_BATTERY_PIN, INPUT);
}

void batteryLoadAll() {
  loadCalibration();
  loadIntCalibration();
}

void batteryApplyPending() {
  if (pendingCalApply) applyPendingCal();
  if (pendingIntCalApply) applyPendingIntCal();
}

void batteryResetCalibration() {
  deviceDividerRatio = EXT_BATT_DIVIDER_RATIO;
  intScaleFactor = 1.0f;
  InternalFS.remove(CAL_FILE);
  InternalFS.remove(INT_CAL_FILE);
}

void loadCalibration() {
  deviceDividerRatio = EXT_BATT_DIVIDER_RATIO;
  if (!InternalFS.exists(CAL_FILE)) return;
  CalibrationConfig cal;
  if (calFile.open(CAL_FILE, FILE_O_READ)) {
    calFile.read(&cal, sizeof(cal));
    calFile.close();
    if (cal.version == CALIBRATION_VERSION &&
        cal.dividerRatio >= 1.0f && cal.dividerRatio <= 25.0f) {
      deviceDividerRatio = cal.dividerRatio;
    }
  }
}

void saveCalibration() {
  CalibrationConfig cal;
  cal.version = CALIBRATION_VERSION;
  memset(cal.reserved, 0, sizeof(cal.reserved));
  cal.dividerRatio = deviceDividerRatio;
  InternalFS.remove(CAL_FILE);
  if (calFile.open(CAL_FILE, FILE_O_WRITE)) {
    calFile.write((const uint8_t*)&cal, sizeof(cal));
    calFile.close();
  }
}

void applyPendingCal() {
  pendingCalApply = false;
  uint16_t targetMv = pendingCalTargetMv;

  if (targetMv == 0) {
    deviceDividerRatio = EXT_BATT_DIVIDER_RATIO;
    InternalFS.remove(CAL_FILE);
    return;
  }

  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(10);
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(DEVICE_BATTERY_PIN);
    delayMicroseconds(100);
  }
  float avg = (float)(total / 5);
  if (avg < 1.0f) return;

  float raw_V = (avg / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;
  float target_V = targetMv / 1000.0f;
  float new_ratio = target_V / raw_V;

  if (new_ratio < 1.0f || new_ratio > 25.0f) return;

  deviceDividerRatio = new_ratio;
  saveCalibration();
}

void onCalSetWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 2) return;
  pendingCalTargetMv = (uint16_t)d[0] | ((uint16_t)d[1] << 8);
  pendingCalApply = true;
}

void loadIntCalibration() {
  intScaleFactor = 1.0f;
  if (!InternalFS.exists(INT_CAL_FILE)) return;
  IntCalibrationConfig cal;
  if (intCalFile.open(INT_CAL_FILE, FILE_O_READ)) {
    intCalFile.read(&cal, sizeof(cal));
    intCalFile.close();
    if (cal.version == CALIBRATION_VERSION &&
        cal.scaleFactor >= 0.5f && cal.scaleFactor <= 2.0f) {
      intScaleFactor = cal.scaleFactor;
    }
  }
}

void saveIntCalibration() {
  IntCalibrationConfig cal;
  cal.version = CALIBRATION_VERSION;
  memset(cal.reserved, 0, sizeof(cal.reserved));
  cal.scaleFactor = intScaleFactor;
  InternalFS.remove(INT_CAL_FILE);
  if (intCalFile.open(INT_CAL_FILE, FILE_O_WRITE)) {
    intCalFile.write((const uint8_t*)&cal, sizeof(cal));
    intCalFile.close();
  }
}

void applyPendingIntCal() {
  pendingIntCalApply = false;
  uint16_t targetMv = pendingIntCalTargetMv;

  if (targetMv == 0) {
    intScaleFactor = 1.0f;
    InternalFS.remove(INT_CAL_FILE);
    return;
  }

  float raw_V = lastRawIntVoltage;

  if (raw_V < 0.1f) {
    analogReference(AR_INTERNAL_3_0);
    analogReadResolution(10);
    analogRead(BATTERY_PIN);
    delayMicroseconds(500);
    int total = 0;
    for (int i = 0; i < 5; i++) {
      total += analogRead(BATTERY_PIN);
      delayMicroseconds(100);
    }
    raw_V = ((float)(total / 5) / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;
  }

  if (raw_V < 0.1f) return;

  float target_V = targetMv / 1000.0f;
  float new_scale = target_V / raw_V;

  if (new_scale < 0.5f || new_scale > 2.0f) return;

  intScaleFactor = new_scale;
  saveIntCalibration();
}

void onIntCalSetWrite(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t l) {
  (void)h; (void)c;
  if (l < 2) return;
  pendingIntCalTargetMv = (uint16_t)d[0] | ((uint16_t)d[1] << 8);
  pendingIntCalApply = true;
}

float readBatteryVoltage() {
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(10);
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(BATTERY_PIN);
    delayMicroseconds(100);
  }
  float raw = ((float)(total / 5) / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;
  lastRawIntVoltage = raw;
  return raw * intScaleFactor;
}

bool readDeviceBattery(int &pct, float &voltMv, uint8_t cellCount) {
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(10);
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(DEVICE_BATTERY_PIN);
    delayMicroseconds(100);
  }
  float avg = (float)(total / 5);
  float voltage = (avg / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE * deviceDividerRatio;
  if (voltage < 0.5f) return false;
  voltMv = voltage * 1000.0f;
  uint8_t cells = (cellCount < 1) ? 1 : cellCount;
  float perCellVoltage = voltage / cells;
  pct = readLiPoPercentage(perCellVoltage);
  return true;
}

int readCR2032Percentage(float voltage) {
  if (voltage >= 3.0f) return 100;
  if (voltage <= 2.5f) return 0;

  float pct;
  if      (voltage >= 2.9f) pct = 80.0f + (voltage - 2.9f) / 0.1f * 20.0f;
  else if (voltage >= 2.8f) pct = 50.0f + (voltage - 2.8f) / 0.1f * 30.0f;
  else if (voltage >= 2.7f) pct = 20.0f + (voltage - 2.7f) / 0.1f * 30.0f;
  else                      pct =          (voltage - 2.5f) / 0.2f * 20.0f;

  int result = (int)pct;
  if (result > 100) result = 100;
  if (result < 0)   result = 0;
  return result;
}

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
