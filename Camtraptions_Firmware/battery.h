#pragma once

#include "config.h"

void batteryInit();
void batteryLoadAll();
void batteryApplyPending();
void batteryResetCalibration();

float readBatteryVoltage();
bool  readDeviceBattery(int &pct, float &voltMv, uint8_t cellCount);
int   readCR2032Percentage(float voltage);
int   readLiPoPercentage(float voltage);

void onCalSetWrite(uint16_t conn_hdl, class BLECharacteristic* chr, uint8_t* data, uint16_t len);
void onIntCalSetWrite(uint16_t conn_hdl, class BLECharacteristic* chr, uint8_t* data, uint16_t len);
