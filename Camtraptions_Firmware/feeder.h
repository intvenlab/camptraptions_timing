#pragma once

#include "config.h"

void configureFeederIo();
void resetFeederState();
void processFeederLogic();
bool feederModeActive();

void onFeederTrigChange();
