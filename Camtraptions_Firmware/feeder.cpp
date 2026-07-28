#include "feeder.h"

#include "storage.h"

volatile bool feederInputActive = false;
static bool feederOutputAsserted = false;
static uint32_t feederAssertedMs = 0;

static bool pumpOn = false;
static uint32_t pumpNextToggleMs = 0;

bool feederModeActive() {
  return cfg.deviceType == DEVICE_TYPE_FEEDER && feederCfg.enabled;
}

void resetFeederState() {
  digitalWrite(FEEDER_PULSE_OUT_PIN, LOW);
  digitalWrite(FEEDER_PUMP_OUT_PIN, LOW);
  feederOutputAsserted = false;
  feederInputActive = false;
  pumpOn = false;
  // Start idle: don't switch the pump on immediately when Feeder mode is (re)entered.
  pumpNextToggleMs = millis() + feederCfg.pumpOffMs;
}

void configureFeederIo() {
  pinMode(FEEDER_PULSE_OUT_PIN, OUTPUT);
  pinMode(FEEDER_PUMP_OUT_PIN, OUTPUT);
  resetFeederState();

  pinMode(FEEDER_TRIG_IN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FEEDER_TRIG_IN_PIN), onFeederTrigChange, CHANGE);
}

// Active-low input, same convention as FP_IN/HP_IN. Only asserts the output;
// never releases it here -- release is handled by processFeederLogic() once the
// configured minimum hold time has elapsed, per the "no decision logic in ISR" rule.
void onFeederTrigChange() {
  bool active = (digitalRead(FEEDER_TRIG_IN_PIN) == LOW);
  feederInputActive = active;
  if (active && !feederOutputAsserted) {
    digitalWrite(FEEDER_PULSE_OUT_PIN, HIGH);
    feederAssertedMs = millis();
    feederOutputAsserted = true;
  }
}

void processFeederLogic() {
  uint32_t now = millis();

  // Pulse-stretcher: release once the input has gone inactive AND the configured
  // minimum hold time has elapsed -- output width = max(input width, minimum).
  if (feederOutputAsserted && !feederInputActive &&
      timeReached(now, feederAssertedMs + feederCfg.pulseStretchMinMs)) {
    digitalWrite(FEEDER_PULSE_OUT_PIN, LOW);
    feederOutputAsserted = false;
  }

  // Free-running pump timer, independent of the trigger input.
  if (timeReached(now, pumpNextToggleMs)) {
    pumpOn = !pumpOn;
    digitalWrite(FEEDER_PUMP_OUT_PIN, pumpOn ? HIGH : LOW);
    pumpNextToggleMs = now + (pumpOn ? feederCfg.pumpOnMs : feederCfg.pumpOffMs);
  }
}
