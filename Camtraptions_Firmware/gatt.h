#pragma once

#include "config.h"

void setupGatt();
void populateCharacteristics();
void populateCameraCharacteristics();
void populateTelemetryCharacteristic();
void advertiseData(int intPct, float intVoltage, uint8_t extPct, uint16_t extVoltMv);
void serviceConnectedManagementPlane(uint32_t now);

void onConnect(uint16_t connHdl);
void onDisconnect(uint16_t connHdl, uint8_t reason);

void publishCamCfgWriteStatus(uint8_t statusCode);

void onNameWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onGroupIdWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onGroupNameWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onDevTypeWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onChemWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onCellWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onResetWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onFactoryWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
void onCamCfgWrite(uint16_t h, class BLECharacteristic* c, uint8_t* d, uint16_t l);
