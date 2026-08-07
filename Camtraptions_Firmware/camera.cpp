#include "camera.h"

#include "feeder.h"
#include "storage.h"

#ifdef DEBUG_CAMERA_LOGIC_PINS
static uint32_t nextHpSampleLogMs = 0;

static void dbgLogPinTransition(
  const char* sig,
  bool active,
  uint32_t now,
  const char* reason,
  int32_t durationMs
) {
  if (!Serial) return;
  Serial.print("[DBG_PIN] t=");
  Serial.print(now);
  Serial.print(" sig=");
  Serial.print(sig);
  Serial.print(" state=");
  Serial.print(active ? "ACTIVE" : "INACTIVE");
  Serial.print(" reason=");
  Serial.print(reason ? reason : "n/a");
  if (durationMs >= 0) {
    Serial.print(" durMs=");
    Serial.print(durationMs);
  }
  Serial.print(" camState=");
  Serial.print((int)cameraState);
  Serial.print(" hpOutAsserted=");
  Serial.println(hpOutAsserted ? 1 : 0);
}

static void dbgLogRuntimeIo(const char* stage) {
  if (!Serial) return;
  Serial.print("[DBG_IO] t=");
  Serial.print(millis());
  Serial.print(" stage=");
  Serial.print(stage ? stage : "n/a");
  Serial.print(" deviceType=");
  Serial.print((int)cfg.deviceType);
  Serial.print(" camEnabled=");
  Serial.print((int)camCfg.enabled);
  Serial.print(" camState=");
  Serial.print((int)cameraState);
  Serial.print(" logicActive=");
  Serial.print(cameraLogicActive ? 1 : 0);
  Serial.print(" activityActive=");
  Serial.print(activityActive ? 1 : 0);
  Serial.print(" hpOutAsserted=");
  Serial.print(hpOutAsserted ? 1 : 0);
  Serial.print(" hpOutPin=");
  Serial.print(digitalRead(HP_OUT_PIN));
  Serial.print(" fpOutPin=");
  Serial.println(digitalRead(FP_OUT_PIN));
}

static void dbgLogHpSample(uint32_t now) {
  if (!Serial) return;
  Serial.print("[DBG_HP_SAMPLE] t=");
  Serial.print(now);
  Serial.print(" hpOutPin=");
  Serial.print(digitalRead(HP_OUT_PIN));
  Serial.print(" hpOutAsserted=");
  Serial.print(hpOutAsserted ? 1 : 0);
  Serial.print(" camState=");
  Serial.print((int)cameraState);
  Serial.print(" logicActive=");
  Serial.print(cameraLogicActive ? 1 : 0);
  Serial.print(" activityActive=");
  Serial.print(activityActive ? 1 : 0);
  Serial.print(" msUntilWakeDeadline=");
  Serial.println(remainingMs(now, wakeHoldDeadlineMs));
}
#endif

volatile bool fpPulseFlag = false;
volatile bool hpPulseFlag = false;
volatile uint32_t lastHpMs = 0;

CameraState cameraState = CAM_IDLE;
bool cameraLogicActive = false;
bool activityActive = false;
bool hpOutAsserted = false;
bool coldFpAcceptPending = false;
uint8_t framesFired = 0;
uint8_t sequencesStartedThisActivity = 0;
uint32_t hpAssertedMs = 0;
uint32_t wakeHoldDeadlineMs = 0;
uint32_t sequenceStartMs = 0;
uint32_t nextFrameMs = 0;
uint32_t lastFpOutStartMs = 0;
uint32_t fpOutReleaseMs = 0;
uint32_t postShutterHoldUntilMs = 0;
uint32_t fullPressIgnoreUntilMs = 0;
uint32_t maxSequenceTimeoutUntilMs = 0;
uint32_t interFrameHpHoldUntilMs = 0;
uint32_t hpReassertAtMs = 0;
bool interFrameHpRelaxActive = false;
bool interFrameHpReleased = false;
bool fpAcceptedAtGapBoundary = false;
bool pendingCamCfgApply = false;
volatile bool runtimeIoReconfigurePending = false;
CameraConfig pendingCamCfg;
uint32_t lastActivityEndMs = 0;
uint32_t lastTelemetryServiceMs = 0;

void populateCameraCharacteristics();

static void assertPin(int pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

static void releasePin(int pin) {
  pinMode(pin, INPUT);
}

static void assertHpOut(uint32_t now) {
  bool transitioningActive = !hpOutAsserted;
  if (transitioningActive) {
    hpAssertedMs = now;
  }
  assertPin(HP_OUT_PIN);
  hpOutAsserted = true;
#ifdef DEBUG_CAMERA_LOGIC_PINS
  if (transitioningActive) {
    dbgLogPinTransition("HP_OUT", true, now, "assertHpOut", -1);
  }
#endif
}

static void releaseHpOut() {
  uint32_t now = millis();
  int32_t holdMs = hpOutAsserted ? (int32_t)(now - hpAssertedMs) : -1;
  releasePin(HP_OUT_PIN);
  hpOutAsserted = false;
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogPinTransition("HP_OUT", false, now, "releaseHpOut", holdMs);
#endif
}

static void assertFpOut(uint32_t now, const char* reason) {
  assertPin(FP_OUT_PIN);
  lastFpOutStartMs = now;
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogPinTransition("FP_OUT", true, now, reason, -1);
#endif
}

static void releaseFpOut(uint32_t now, const char* reason) {
  int32_t pulseMs = (lastFpOutStartMs != 0) ? (int32_t)(now - lastFpOutStartMs) : -1;
  releasePin(FP_OUT_PIN);
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogPinTransition("FP_OUT", false, now, reason, pulseMs);
#endif
}

static uint32_t minHalfPressMs() {
  return (uint32_t)camCfg.minHalfPressBeforeShutter * 100UL;
}

static uint32_t shutterPulseMs() {
  return (uint32_t)camCfg.shutterPulseDuration * 10UL;
}

static uint32_t startFrameSpacingMs() {
  return (uint32_t)camCfg.startFrameSpacingTicks * 10UL;
}

static uint32_t postShutterHoldMs() {
  return (uint32_t)camCfg.postShutterHpHoldTenths * 100UL;
}

// Safety margin so inter-frame HP release only happens when there is clear slack
// for Z hold + T reassert inside Y (see docs/hp-relax-transition-spec.md).
static const uint32_t HP_RELAX_GUARD_MS = 20UL;

static bool interFrameHpRelaxAllowed() {
  uint32_t yMs = startFrameSpacingMs();
  uint32_t tMs = minHalfPressMs();
  uint32_t zMs = postShutterHoldMs();
  if (yMs <= tMs + HP_RELAX_GUARD_MS) return false;
  return zMs < (yMs - tMs - HP_RELAX_GUARD_MS);
}

static void clearInterFrameHpRelax() {
  interFrameHpHoldUntilMs = 0;
  hpReassertAtMs = 0;
  interFrameHpRelaxActive = false;
  interFrameHpReleased = false;
}

static void beginInterFrameHpRelax(uint32_t pulseEndMs) {
  clearInterFrameHpRelax();
  if (!interFrameHpRelaxAllowed()) return;

  interFrameHpRelaxActive = true;
  interFrameHpReleased = false;
  interFrameHpHoldUntilMs = pulseEndMs + postShutterHoldMs();

  uint32_t tMs = minHalfPressMs();
  if (nextFrameMs > tMs) {
    hpReassertAtMs = nextFrameMs - tMs;
  } else {
    hpReassertAtMs = pulseEndMs;
  }
}

static void serviceInterFrameHpRelax(uint32_t now) {
  if (!interFrameHpRelaxActive) return;

  if (!interFrameHpReleased && timeReached(now, interFrameHpHoldUntilMs)) {
    if (hpOutAsserted) {
      releaseHpOut();
    }
    interFrameHpReleased = true;
  }

  if (interFrameHpReleased && !hpOutAsserted) {
    if (hpReassertAtMs == 0 || timeReached(now, hpReassertAtMs)) {
      assertHpOut(now);
    }
  }
}

static uint32_t wakeHalfPressHoldMs() {
  return (uint32_t)camCfg.wakeHalfPressHoldSec * 1000UL;
}

static uint32_t fullPressIgnoreGapMs() {
  return (uint32_t)camCfg.fullPressIgnoreGapTenths * 100UL;
}

static uint32_t maxSequenceTimeoutMs() {
  uint32_t frameCount = (uint32_t)camCfg.frameCount;
  uint32_t pulse = shutterPulseMs();
  uint32_t spacing = startFrameSpacingMs();
  if (frameCount <= 1) return pulse;
  return ((frameCount - 1UL) * (pulse + spacing)) + pulse;
}

static bool maxSequenceTimeoutActive(uint32_t now) {
  // Treat the timeout deadline as inclusive so an FP/HP edge that lands exactly
  // on the boundary is still ignored for this cycle.
  return maxSequenceTimeoutUntilMs != 0
      && (int32_t)(now - maxSequenceTimeoutUntilMs) <= 0;
}

static void beginMaxSequenceTimeout(uint32_t now) {
  maxSequenceTimeoutUntilMs = now + maxSequenceTimeoutMs();
  endActivity();
}

static bool hpLeadSatisfied(uint32_t now) {
  return hpOutAsserted && (now - hpAssertedMs >= minHalfPressMs());
}

static bool underSequenceCap() {
  uint8_t safeMax = camCfg.maxSequenceCount;
  if (safeMax < 1) safeMax = 1;
  return sequencesStartedThisActivity < safeMax;
}

static bool tryAcceptFp(uint32_t now) {
  fpAcceptedAtGapBoundary = false;
  if (!timeReached(now, fullPressIgnoreUntilMs)) {
    telCounters.ignoredFpDuringGapCount++;
    markTelemetryChanged(TEL_EVT_FP_REJECT_GAP, TEL_SC_FP_GAP_IGNORE, false);
    return false;
  }
  if (!underSequenceCap()) {
    telCounters.MaxSequenceExceededCount++;
    markTelemetryChanged(TEL_EVT_FP_REJECT_CAP, TEL_SC_SEQUENCE_CAP, false);
    beginMaxSequenceTimeout(now);
    return false;
  }
  if (fullPressIgnoreUntilMs != 0 && now == fullPressIgnoreUntilMs) {
    fpAcceptedAtGapBoundary = true;
  }
  return true;
}

static void ensureHpOutAsserted(uint32_t now) {
  if (!hpOutAsserted) {
    assertHpOut(now);
  }
}

static void startSequence(uint32_t now) {
  bool wasColdFp = coldFpAcceptPending;

  ensureHpOutAsserted(now);
  activityActive = true;
  coldFpAcceptPending = false;
  sequencesStartedThisActivity++;
  telCounters.acceptedFpCount++;
  if (wasColdFp) {
    telCounters.coldFpSequenceCount++;
  }
  uint8_t fpAcceptedEvent = TEL_EVT_FP_ACCEPTED;
  if (!wasColdFp && fpAcceptedAtGapBoundary) {
    fpAcceptedEvent = TEL_EVT_FP_ACCEPTED_AT_GAP_BOUNDARY;
  }
  markTelemetryChanged(fpAcceptedEvent,
                       wasColdFp ? TEL_SC_COLD_FP : TEL_SC_NONE,
                       false);
  fpAcceptedAtGapBoundary = false;

  sequenceStartMs = now;
  fullPressIgnoreUntilMs = now + fullPressIgnoreGapMs();

  framesFired = 0;
  fpOutReleaseMs = 0;
  lastFpOutStartMs = 0;
  nextFrameMs = now;
  clearInterFrameHpRelax();
  cameraState = CAM_BURST_ACTIVE;
  cameraLogicActive = true;
}

void endActivity() {
  bool hadActivity = activityActive || hpOutAsserted || cameraState != CAM_IDLE;

  releaseFpOut(millis(), "endActivity");
  releaseHpOut();
  activityActive = false;
  coldFpAcceptPending = false;
  cameraLogicActive = false;
  cameraState = CAM_IDLE;
  framesFired = 0;
  sequencesStartedThisActivity = 0;
  fpOutReleaseMs = 0;
  nextFrameMs = 0;
  lastFpOutStartMs = 0;
  postShutterHoldUntilMs = 0;
  fullPressIgnoreUntilMs = 0;
  clearInterFrameHpRelax();
  fpAcceptedAtGapBoundary = false;
  lastActivityEndMs = millis();

  if (hadActivity) {
    telCounters.activityCompletedCount++;
    if (lastTelemetryEvent == TEL_EVT_WAKE_TIMEOUT) {
      markTelemetryChanged(TEL_EVT_WAKE_TIMEOUT, TEL_SC_WAKE_TIMEOUT, true);
    } else {
      markTelemetryChanged(TEL_EVT_ACTIVITY_END, TEL_SC_NONE, true);
    }
  } else {
    markTelemetryEvent(TEL_EVT_ACTIVITY_END, TEL_SC_NONE);
  }
}

static void runBurstScheduler(uint32_t now) {
  if (fpOutReleaseMs != 0 && timeReached(now, fpOutReleaseMs)) {
    releaseFpOut(now, "burstPulseComplete");
    uint32_t pulseEndMs = now;
    fpOutReleaseMs = 0;

    if (framesFired < camCfg.frameCount) {
      // More frames remain: Z is per-pulse hold; release only when Y/T/Z allow.
      beginInterFrameHpRelax(pulseEndMs);
    }
  }

  serviceInterFrameHpRelax(now);

  if (fpOutReleaseMs == 0 && framesFired < camCfg.frameCount && timeReached(now, nextFrameMs)) {
    if (!hpOutAsserted) {
      assertHpOut(now);
      nextFrameMs = now + minHalfPressMs();
      return;
    }

    if (!hpLeadSatisfied(now)) {
      nextFrameMs = hpAssertedMs + minHalfPressMs();
      return;
    }

    clearInterFrameHpRelax();
    assertFpOut(now, "burstFrameFire");
    fpOutReleaseMs = now + shutterPulseMs();
    framesFired++;
    nextFrameMs = fpOutReleaseMs + startFrameSpacingMs();
  }

  if (framesFired >= camCfg.frameCount && fpOutReleaseMs == 0) {
    clearInterFrameHpRelax();
    telCounters.sequenceCompletedCount++;
    markTelemetryChanged(TEL_EVT_BURST_COMPLETE, TEL_SC_NONE, false);
    // Final-frame Z hold uses the same PostShutter parameter semantics.
    postShutterHoldUntilMs = now + postShutterHoldMs();
    cameraState = CAM_POST_SHUTTER_EXT;
  }
}

bool cameraActivityInProgress() {
  return cameraState != CAM_IDLE
      || cameraLogicActive
      || activityActive
      || hpOutAsserted
      || coldFpAcceptPending
      || fpOutReleaseMs != 0;
}

void configureRuntimeIo() {
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogRuntimeIo("configureRuntimeIo.enter");
#endif
  detachInterrupt(digitalPinToInterrupt(FP_IN_PIN));
  detachInterrupt(digitalPinToInterrupt(HP_IN_PIN));
  detachInterrupt(digitalPinToInterrupt(FEEDER_TRIG_IN_PIN));

  pinMode(FP_OUT_PIN, INPUT);
  pinMode(HP_OUT_PIN, INPUT);
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogRuntimeIo("configureRuntimeIo.outputsInput");
#endif

  // Unlike Camera's open-drain FP_OUT/HP_OUT (safe floating because the camera
  // side has pull-ups), a floating MOSFET gate can partially turn on -- always
  // drive these two pins to a defined OFF state regardless of device type.
  pinMode(FEEDER_PULSE_OUT_PIN, OUTPUT);
  digitalWrite(FEEDER_PULSE_OUT_PIN, LOW);
  pinMode(FEEDER_PUMP_OUT_PIN, OUTPUT);
  digitalWrite(FEEDER_PUMP_OUT_PIN, LOW);

  if (cfg.deviceType == DEVICE_TYPE_FEEDER) {
    if (feederCfg.enabled) configureFeederIo();
    return;
  }

  if (cfg.deviceType == 1 /* CAMERA */) {
    pinMode(FP_IN_PIN, INPUT_PULLUP);
    pinMode(HP_IN_PIN, INPUT_PULLUP);

    if (camCfg.enabled) {
      attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onShutterPulse, FALLING);
      attachInterrupt(digitalPinToInterrupt(HP_IN_PIN), onHpPulse,      FALLING);
      pinMode(FP_OUT_PIN, INPUT);
      pinMode(HP_OUT_PIN, INPUT);
#ifdef DEBUG_CAMERA_LOGIC_PINS
      dbgLogRuntimeIo("configureRuntimeIo.cameraStateMachine");
#endif
    } else {
      pinMode(FP_OUT_PIN, OUTPUT);
      digitalWrite(FP_OUT_PIN, digitalRead(FP_IN_PIN));
      pinMode(HP_OUT_PIN, OUTPUT);
      digitalWrite(HP_OUT_PIN, digitalRead(HP_IN_PIN));
      attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onFpPassthrough, CHANGE);
      attachInterrupt(digitalPinToInterrupt(HP_IN_PIN), onHpPassthrough, CHANGE);
#ifdef DEBUG_CAMERA_LOGIC_PINS
      dbgLogRuntimeIo("configureRuntimeIo.cameraPassthrough");
#endif
    }
    return;
  }

  pinMode(FP_IN_PIN, INPUT_PULLUP);
  pinMode(HP_IN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FP_IN_PIN), onShutterPulse, FALLING);
#ifdef DEBUG_CAMERA_LOGIC_PINS
  dbgLogRuntimeIo("configureRuntimeIo.nonCamera");
#endif
}

void applyPendingCamCfgIfIdle() {
  if (cameraActivityInProgress()) return;

  if (pendingCamCfgApply) {
    camCfg = pendingCamCfg;
    sanitizeCameraConfig(camCfg);
    saveCameraSettings();
    populateCameraCharacteristics();
    pendingCamCfgApply = false;
    runtimeIoReconfigurePending = true;
  }

  if (runtimeIoReconfigurePending) {
    configureRuntimeIo();
    runtimeIoReconfigurePending = false;
  }
}

void refreshWakeHoldFromHp(uint32_t now) {
  (void)now;
  telCounters.hpRefreshCount++;
  markTelemetryChanged(TEL_EVT_HP_REFRESH, TEL_SC_NONE, false);
}

bool cameraControlWorkPending() {
  if (!(cfg.deviceType == 1 /* CAMERA */ && camCfg.enabled)) return false;
  return cameraLogicActive || hpPulseFlag || fpPulseFlag;
}

void onShutterPulse() {
  uint32_t now = millis();
  uint32_t debounceMs = (cfg.deviceType == 1 && camCfg.enabled)
                      ? camCfg.fpDebounceMs
                      : SHUTTER_DEBOUNCE_MS;
  if (now - lastShutterMs < debounceMs) {
    if (cfg.deviceType == 1 && camCfg.enabled) {
      telCounters.fpDebounceRejectCount++;
      markTelemetryChanged(TEL_EVT_FP_DEBOUNCE_REJECT, TEL_SC_DEBOUNCE, false);
    }
    return;
  }
  lastShutterMs  = now;
  cfg.shutterCount++;
  settingsDirty  = true;
  shutterUpdated = true;
  fpPulseFlag    = true;
}

void onHpPulse() {
  uint32_t now = millis();
  if (!hpOutAsserted) {
    assertPin(HP_OUT_PIN);
    hpAssertedMs = now;
    hpOutAsserted = true;
  }
  lastHpMs    = now;
  hpPulseFlag = true;
}

void onFpPassthrough() {
  int state = digitalRead(FP_IN_PIN);
  digitalWrite(FP_OUT_PIN, state);

  if (state == LOW) {
    uint32_t now = millis();
    if (now - lastShutterMs >= SHUTTER_DEBOUNCE_MS) {
      lastShutterMs  = now;
      cfg.shutterCount++;
      settingsDirty  = true;
      shutterUpdated = true;
    }
  }
}

void onHpPassthrough() {
  digitalWrite(HP_OUT_PIN, digitalRead(HP_IN_PIN));
}

void processCameraLogic() {
  uint32_t now = millis();

#ifdef DEBUG_CAMERA_LOGIC_PINS
  if (timeReached(now, nextHpSampleLogMs)) {
    dbgLogHpSample(now);
    nextHpSampleLogMs = now + 100;
  }
#endif

  bool hpTrig = false, fpTrig = false;
  noInterrupts();
  if (hpPulseFlag) { hpPulseFlag = false; hpTrig = true; }
  if (fpPulseFlag) { fpPulseFlag = false; fpTrig = true; }
  interrupts();

  if (maxSequenceTimeoutActive(now)) {
    hpTrig = false;
    fpTrig = false;
  } else if (maxSequenceTimeoutUntilMs != 0) {
    maxSequenceTimeoutUntilMs = 0;
  }

  switch (cameraState) {

    case CAM_IDLE:
      cameraLogicActive = false;

      if (hpTrig && fpTrig && camCfg.fullPressWithoutHpPolicy == 0) {
        // Simultaneous HP+FP is valid shoot intent: latch HP and start sequence.
        assertHpOut(now);
        activityActive = true;
        sequencesStartedThisActivity = 0;
        wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
        fullPressIgnoreUntilMs = 0;
        cameraLogicActive = true;
        markTelemetryEvent(TEL_EVT_HP_WAKE, TEL_SC_NONE);
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        } else {
          cameraState = CAM_WAKE_AF;
        }
      } else if (hpTrig) {
        assertHpOut(now);
        activityActive = false;
        sequencesStartedThisActivity = 0;
        wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
        fullPressIgnoreUntilMs = 0;
        cameraState = CAM_WAKE_AF;
        cameraLogicActive = true;
        markTelemetryEvent(TEL_EVT_HP_WAKE, TEL_SC_NONE);
      } else if (fpTrig && camCfg.fullPressWithoutHpPolicy == 0) {
        assertHpOut(now);
        activityActive = true;
        sequencesStartedThisActivity = 0;
        wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
        fullPressIgnoreUntilMs = 0;
        coldFpAcceptPending = true;
        cameraState = CAM_COLD_FP_WAIT;
        cameraLogicActive = true;
        markTelemetryEvent(TEL_EVT_COLD_FP, TEL_SC_COLD_FP);
      }
      break;

    case CAM_WAKE_AF:
      cameraLogicActive = true;

      if (hpTrig) {
        refreshWakeHoldFromHp(now);
      }

      if (fpTrig) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        }
        break;
      }

      if (!activityActive && timeReached(now, wakeHoldDeadlineMs)) {
        telCounters.wakeTimeoutCount++;
        markTelemetryChanged(TEL_EVT_WAKE_TIMEOUT, TEL_SC_WAKE_TIMEOUT, true);
        endActivity();
      }
      break;

    case CAM_COLD_FP_WAIT:
      cameraLogicActive = true;
      ensureHpOutAsserted(now);

      if (hpTrig) {
        refreshWakeHoldFromHp(now);
      }

      if (timeReached(now, wakeHoldDeadlineMs)) {
        telCounters.wakeTimeoutCount++;
        markTelemetryChanged(TEL_EVT_WAKE_TIMEOUT, TEL_SC_WAKE_TIMEOUT, true);
        endActivity();
        break;
      }

      if (!coldFpAcceptPending) {
        endActivity();
        break;
      }

      if (hpLeadSatisfied(now)) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        } else {
          endActivity();
        }
      }
      break;

    case CAM_BURST_ACTIVE:
      cameraLogicActive = true;
      // Do not force HP asserted here: inter-frame relax may intentionally
      // release HP between frames when Z/Y/T permit it.

      if (hpTrig) {
        telCounters.hpIgnoredDuringBurstCount++;
        markTelemetryChanged(TEL_EVT_HP_IGNORED_BURST, TEL_SC_HP_DURING_BURST, false);
      }
      if (fpTrig) {
        telCounters.ignoredFpDuringBurstCount++;
        if (!timeReached(now, fullPressIgnoreUntilMs)) {
          telCounters.ignoredFpDuringGapCount++;
        }
        markTelemetryChanged(TEL_EVT_FP_REJECT_GAP, TEL_SC_FP_GAP_IGNORE, false);
      }
      runBurstScheduler(now);
      break;

    case CAM_POST_SHUTTER_EXT:
      cameraLogicActive = true;
      ensureHpOutAsserted(now);

      if (hpTrig) {
        refreshWakeHoldFromHp(now);
      }

      if (fpTrig) {
        if (tryAcceptFp(now)) {
          startSequence(now);
          runBurstScheduler(now);
        }
        break;
      }

      if (timeReached(now, postShutterHoldUntilMs)) {
        activityActive = false;
        cameraState = CAM_WAKE_AF;
      }
      break;
  }
}

void idleWaitWithDeviceWake(uint32_t durationMs) {
  bool cameraIdleWakeEligible = (cfg.deviceType == 1 && camCfg.enabled && camCfg.powerSaveIdleMode);
  bool feederActive = feederModeActive();
  if (!(cameraIdleWakeEligible || feederActive)) {
    delay(durationMs);
    return;
  }

  uint32_t startMs = millis();
  while (millis() - startMs < durationMs) {
    if (cameraLogicActive || hpPulseFlag || fpPulseFlag) {
      processCameraLogic();
      if (cameraLogicActive) return;
      if (hpPulseFlag || fpPulseFlag) continue;
    }
    if (feederActive) processFeederLogic();
    __WFE();
  }
}
