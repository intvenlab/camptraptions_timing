/*
 * TickleBoardBits - Arduino Uno fixture firmware
 *
 * Implements a line-oriented protocol for validation testing:
 * ID?, MAP, ARM, PULSE, LEVEL, RUN, DUMP, RESET
 */

#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t MAX_EVENTS = 24;
static const uint8_t MAX_EDGES = 48;
static const uint8_t MAX_ISR_PENDING = 16;
static const uint16_t RX_BUF_SIZE = 64;

enum PolarityMode : uint8_t {
  POL_ACTIVE_LOW = 0,
  POL_ACTIVE_HIGH = 1
};

enum SignalId : uint8_t {
  SIG_HP = 0,
  SIG_FP = 1
};

struct ScheduledEvent {
  bool used;
  SignalId sig;
  uint32_t atMs;
  bool active;
};

enum EdgeSignalId : uint8_t {
  EDGE_SIG_HP_IN = 0,
  EDGE_SIG_FP_IN = 1,
  EDGE_SIG_HP_OUT = 2,
  EDGE_SIG_FP_OUT = 3
};

struct EdgeLog {
  uint32_t tUs;
  EdgeSignalId sigId;
  bool active;
};

struct PendingIsrEdge {
  uint32_t tUs;
  bool isHp;
  bool active;
};

struct PinMap {
  uint8_t hpStimPin;
  uint8_t fpStimPin;
  uint8_t hpMonPin;
  uint8_t fpMonPin;
};

// Wiring locked to DUT pinout:
// DUT D3 (HP_IN)  <- Uno D5 (HP stimulus)
// DUT D2 (FP_IN)  <- Uno D4 (FP stimulus)
// DUT D5 (HP_OUT) -> Uno D3 (HP monitor)
// DUT D4 (FP_OUT) -> Uno D2 (FP monitor)
static PinMap pinMap = {5, 4, 3, 2};
static PolarityMode polarity = POL_ACTIVE_LOW;

static ScheduledEvent events[MAX_EVENTS];
static EdgeLog edges[MAX_EDGES];
static uint8_t edgeCount = 0;
static bool edgeOverflow = false;

static uint32_t armCaptureMs = 0;
static uint32_t runStartUs = 0;
static bool isArmed = false;
static uint32_t runIdCounter = 0;
static uint32_t currentRunId = 0;

static volatile bool lastHpMonActive = false;
static volatile bool lastFpMonActive = false;
static volatile PendingIsrEdge pendingIsrEdges[MAX_ISR_PENDING];
static volatile uint8_t pendingIsrHead = 0;
static volatile uint8_t pendingIsrTail = 0;
static volatile bool pendingIsrOverflow = false;

static char rxBuf[RX_BUF_SIZE];
static uint16_t rxLen = 0;

static inline bool isActiveLevel(int pinLevel) {
  if (polarity == POL_ACTIVE_LOW) return pinLevel == LOW;
  return pinLevel == HIGH;
}

static inline int levelForActive(bool active) {
  if (polarity == POL_ACTIVE_LOW) return active ? LOW : HIGH;
  return active ? HIGH : LOW;
}

static void logEdge(EdgeSignalId sigId, bool active, uint32_t nowUs) {
  if (edgeCount >= MAX_EDGES) {
    edgeOverflow = true;
    return;
  }
  edges[edgeCount].tUs = nowUs;
  edges[edgeCount].sigId = sigId;
  edges[edgeCount].active = active;
  edgeCount++;
}

static void setStimSignal(SignalId sig, bool active) {
  int level = levelForActive(active);
  if (sig == SIG_HP) {
    digitalWrite(pinMap.hpStimPin, level);
    logEdge(EDGE_SIG_HP_IN, active, micros());
  } else {
    digitalWrite(pinMap.fpStimPin, level);
    logEdge(EDGE_SIG_FP_IN, active, micros());
  }
}

static void clearEvents(void) {
  for (uint8_t i = 0; i < MAX_EVENTS; i++) {
    events[i].used = false;
  }
}

static bool addEvent(SignalId sig, uint32_t atMs, bool active) {
  for (uint8_t i = 0; i < MAX_EVENTS; i++) {
    if (!events[i].used) {
      events[i].used = true;
      events[i].sig = sig;
      events[i].atMs = atMs;
      events[i].active = active;
      return true;
    }
  }
  return false;
}

static void resetLogs(void) {
  edgeCount = 0;
  edgeOverflow = false;
  noInterrupts();
  pendingIsrHead = 0;
  pendingIsrTail = 0;
  pendingIsrOverflow = false;
  interrupts();
}

static void configureStimOutputInactive(uint8_t pin) {
  // Preload output latch before enabling OUTPUT to avoid startup glitches.
  digitalWrite(pin, levelForActive(false));
  pinMode(pin, OUTPUT);
}

static void applyPinModes(void) {
  configureStimOutputInactive(pinMap.hpStimPin);
  configureStimOutputInactive(pinMap.fpStimPin);
  // Bias open-drain DUT outputs to a stable inactive-high baseline on the fixture.
  pinMode(pinMap.hpMonPin, INPUT_PULLUP);  // Uno D3 for HP_OUT
  pinMode(pinMap.fpMonPin, INPUT_PULLUP);  // Uno D2 for FP_OUT
}

static bool parseUint32(const char* s, uint32_t* out) {
  if (!s || !*s) return false;
  char* endp = nullptr;
  unsigned long v = strtoul(s, &endp, 10);
  if (*endp != '\0') return false;
  *out = (uint32_t)v;
  return true;
}

static bool parseSignal(const char* s, SignalId* outSig) {
  if (!s) return false;
  if (strcmp(s, "HP") == 0) {
    *outSig = SIG_HP;
    return true;
  }
  if (strcmp(s, "FP") == 0) {
    *outSig = SIG_FP;
    return true;
  }
  return false;
}

static bool parseStateToken(const char* s, bool* outActive) {
  if (!s) return false;
  if (strcmp(s, "ACTIVE") == 0) {
    *outActive = true;
    return true;
  }
  if (strcmp(s, "INACTIVE") == 0) {
    *outActive = false;
    return true;
  }
  return false;
}

static void printOk(void) {
  Serial.println(F("OK"));
}

static void printErr(const __FlashStringHelper* msg) {
  Serial.print(F("ERR "));
  Serial.println(msg);
}

static void printErr(const char* msg) {
  Serial.print(F("ERR "));
  Serial.println(msg);
}

static void pushIsrEdge(bool isHp, bool active) {
  uint8_t nextHead = (uint8_t)((pendingIsrHead + 1) % MAX_ISR_PENDING);
  if (nextHead == pendingIsrTail) {
    pendingIsrOverflow = true;
    return;
  }
  pendingIsrEdges[pendingIsrHead].tUs = micros();
  pendingIsrEdges[pendingIsrHead].isHp = isHp;
  pendingIsrEdges[pendingIsrHead].active = active;
  pendingIsrHead = nextHead;
}

void onHpMonChange(void) {
  bool hpActive = isActiveLevel(digitalRead(pinMap.hpMonPin));
  if (hpActive != lastHpMonActive) {
    lastHpMonActive = hpActive;
    pushIsrEdge(true, hpActive);
  }
}

void onFpMonChange(void) {
  bool fpActive = isActiveLevel(digitalRead(pinMap.fpMonPin));
  if (fpActive != lastFpMonActive) {
    lastFpMonActive = fpActive;
    pushIsrEdge(false, fpActive);
  }
}

static bool monitorPinsSupportIsr(void) {
  return digitalPinToInterrupt(pinMap.hpMonPin) != NOT_AN_INTERRUPT &&
         digitalPinToInterrupt(pinMap.fpMonPin) != NOT_AN_INTERRUPT;
}

static void attachMonitorInterrupts(void) {
  attachInterrupt(digitalPinToInterrupt(pinMap.hpMonPin), onHpMonChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinMap.fpMonPin), onFpMonChange, CHANGE);
}

static void detachMonitorInterrupts(void) {
  detachInterrupt(digitalPinToInterrupt(pinMap.hpMonPin));
  detachInterrupt(digitalPinToInterrupt(pinMap.fpMonPin));
}

static void drainPendingIsrEdges(void) {
  bool overflowed = false;
  noInterrupts();
  while (pendingIsrTail != pendingIsrHead) {
    PendingIsrEdge ev;
    ev.tUs = pendingIsrEdges[pendingIsrTail].tUs;
    ev.isHp = pendingIsrEdges[pendingIsrTail].isHp;
    ev.active = pendingIsrEdges[pendingIsrTail].active;
    pendingIsrTail = (uint8_t)((pendingIsrTail + 1) % MAX_ISR_PENDING);
    interrupts();
    logEdge(ev.isHp ? EDGE_SIG_HP_OUT : EDGE_SIG_FP_OUT, ev.active, ev.tUs);
    noInterrupts();
  }
  if (pendingIsrOverflow) {
    overflowed = true;
    pendingIsrOverflow = false;
  }
  interrupts();
  if (overflowed) {
    edgeOverflow = true;
  }
}

static void runSchedule(void) {
  if (!isArmed) {
    printErr(F("NOT_ARMED"));
    return;
  }
  if (!monitorPinsSupportIsr()) {
    printErr(F("MON_NO_ISR"));
    return;
  }

  runStartUs = micros();
  uint32_t runEndUs = runStartUs + (armCaptureMs * 1000UL);
  Serial.print(F("RUN_START "));
  Serial.println(currentRunId);

  // Prime monitor state to avoid false edge at t=0.
  noInterrupts();
  pendingIsrHead = 0;
  pendingIsrTail = 0;
  pendingIsrOverflow = false;
  lastHpMonActive = isActiveLevel(digitalRead(pinMap.hpMonPin));
  lastFpMonActive = isActiveLevel(digitalRead(pinMap.fpMonPin));
  interrupts();
  attachMonitorInterrupts();

  while ((int32_t)(micros() - runEndUs) < 0) {
    uint32_t elapsedMs = (micros() - runStartUs) / 1000UL;

    for (uint8_t i = 0; i < MAX_EVENTS; i++) {
      if (events[i].used && elapsedMs >= events[i].atMs) {
        setStimSignal(events[i].sig, events[i].active);
        events[i].used = false;
      }
    }

    drainPendingIsrEdges();
  }

  // Return stimulus lines inactive after run.
  setStimSignal(SIG_HP, false);
  setStimSignal(SIG_FP, false);
  delayMicroseconds(200);
  drainPendingIsrEdges();
  detachMonitorInterrupts();

  Serial.print(F("RUN_OK "));
  Serial.println(currentRunId);
}

static void printEdgeSignalName(EdgeSignalId sigId) {
  switch (sigId) {
    case EDGE_SIG_HP_IN:
      Serial.print(F("HP_IN"));
      break;
    case EDGE_SIG_FP_IN:
      Serial.print(F("FP_IN"));
      break;
    case EDGE_SIG_HP_OUT:
      Serial.print(F("HP_OUT"));
      break;
    case EDGE_SIG_FP_OUT:
      Serial.print(F("FP_OUT"));
      break;
    default:
      Serial.print(F("UNKNOWN"));
      break;
  }
}

static void dumpEdges(void) {
  Serial.print(F("BEGIN LOG RUNID="));
  Serial.println(currentRunId);
  Serial.print(F("SNAPSHOT HP_IN="));
  Serial.print(isActiveLevel(digitalRead(pinMap.hpStimPin)) ? F("ACTIVE") : F("INACTIVE"));
  Serial.print(F(" FP_IN="));
  Serial.print(isActiveLevel(digitalRead(pinMap.fpStimPin)) ? F("ACTIVE") : F("INACTIVE"));
  Serial.print(F(" HP_OUT="));
  Serial.print(isActiveLevel(digitalRead(pinMap.hpMonPin)) ? F("ACTIVE") : F("INACTIVE"));
  Serial.print(F(" FP_OUT="));
  Serial.println(isActiveLevel(digitalRead(pinMap.fpMonPin)) ? F("ACTIVE") : F("INACTIVE"));
  for (uint8_t i = 0; i < edgeCount; i++) {
    uint32_t relUs = edges[i].tUs - runStartUs;
    Serial.print(F("EDGE "));
    Serial.print(relUs / 1000UL);
    Serial.print(' ');
    printEdgeSignalName(edges[i].sigId);
    Serial.print(' ');
    Serial.println(edges[i].active ? F("ACTIVE") : F("INACTIVE"));
  }
  if (edgeOverflow) {
    Serial.println(F("WARN EDGE_OVERFLOW"));
  }
  Serial.print(F("END OK RUNID="));
  Serial.println(currentRunId);
}

static void handleMapToken(const char* tok) {
  if (!tok) return;
  if (strncmp(tok, "HP_IN=", 6) == 0) {
    pinMap.hpStimPin = (uint8_t)atoi(tok + 6);
  } else if (strncmp(tok, "FP_IN=", 6) == 0) {
    pinMap.fpStimPin = (uint8_t)atoi(tok + 6);
  } else if (strncmp(tok, "HP_OUT=", 7) == 0) {
    pinMap.hpMonPin = (uint8_t)atoi(tok + 7);
  } else if (strncmp(tok, "FP_OUT=", 7) == 0) {
    pinMap.fpMonPin = (uint8_t)atoi(tok + 7);
  } else if (strncmp(tok, "POL=", 4) == 0) {
    if (strcmp(tok + 4, "ACTIVE_LOW") == 0) {
      polarity = POL_ACTIVE_LOW;
    } else if (strcmp(tok + 4, "ACTIVE_HIGH") == 0) {
      polarity = POL_ACTIVE_HIGH;
    }
  }
}

static void processCommand(char* line) {
  // Uppercase in-place for predictable token matching.
  for (uint16_t i = 0; line[i] != '\0'; i++) {
    line[i] = (char)toupper((unsigned char)line[i]);
  }

  char* cmd = strtok(line, " ");
  if (!cmd) return;

  if (strcmp(cmd, "ID?") == 0) {
    Serial.println(F("ID TickleBoardBits UNO_R3 PROTO=1 CMDS=ID?,MAP,ARM,PULSE,LEVEL,RUN,DUMP,RESET"));
    return;
  }

  if (strcmp(cmd, "RESET") == 0) {
    clearEvents();
    resetLogs();
    isArmed = false;
    setStimSignal(SIG_HP, false);
    setStimSignal(SIG_FP, false);
    printOk();
    return;
  }

  if (strcmp(cmd, "MAP") == 0) {
    char* tok = nullptr;
    while ((tok = strtok(nullptr, " ")) != nullptr) {
      handleMapToken(tok);
    }
    applyPinModes();
    if (!monitorPinsSupportIsr()) {
      printErr(F("MON_NO_ISR"));
      return;
    }
    printOk();
    return;
  }

  if (strcmp(cmd, "ARM") == 0) {
    char* msTok = strtok(nullptr, " ");
    uint32_t captureMs = 0;
    if (!parseUint32(msTok, &captureMs) || captureMs == 0) {
      printErr(F("BAD_ARM_MS"));
      return;
    }
    armCaptureMs = captureMs;
    clearEvents();
    resetLogs();
    isArmed = true;
    currentRunId = ++runIdCounter;
    Serial.print(F("ARMED RUNID="));
    Serial.print(currentRunId);
    Serial.print(F(" CAPTURE_MS="));
    Serial.println(armCaptureMs);
    Serial.print(F("SNAPSHOT HP_IN="));
    Serial.print(isActiveLevel(digitalRead(pinMap.hpStimPin)) ? F("ACTIVE") : F("INACTIVE"));
    Serial.print(F(" FP_IN="));
    Serial.print(isActiveLevel(digitalRead(pinMap.fpStimPin)) ? F("ACTIVE") : F("INACTIVE"));
    Serial.print(F(" HP_OUT="));
    Serial.print(isActiveLevel(digitalRead(pinMap.hpMonPin)) ? F("ACTIVE") : F("INACTIVE"));
    Serial.print(F(" FP_OUT="));
    Serial.println(isActiveLevel(digitalRead(pinMap.fpMonPin)) ? F("ACTIVE") : F("INACTIVE"));
    printOk();
    return;
  }

  if (strcmp(cmd, "PULSE") == 0) {
    SignalId sig;
    uint32_t atMs = 0;
    uint32_t durMs = 0;
    char* sigTok = strtok(nullptr, " ");
    char* atTok = strtok(nullptr, " ");
    char* durTok = strtok(nullptr, " ");
    if (!isArmed) {
      printErr(F("NOT_ARMED"));
      return;
    }
    if (!parseSignal(sigTok, &sig) || !parseUint32(atTok, &atMs) || !parseUint32(durTok, &durMs)) {
      printErr(F("BAD_PULSE"));
      return;
    }
    if (!addEvent(sig, atMs, true) || !addEvent(sig, atMs + durMs, false)) {
      printErr(F("EVENT_OVERFLOW"));
      return;
    }
    printOk();
    return;
  }

  if (strcmp(cmd, "LEVEL") == 0) {
    SignalId sig;
    uint32_t atMs = 0;
    bool active = false;
    char* sigTok = strtok(nullptr, " ");
    char* atTok = strtok(nullptr, " ");
    char* stTok = strtok(nullptr, " ");
    if (!isArmed) {
      printErr(F("NOT_ARMED"));
      return;
    }
    if (!parseSignal(sigTok, &sig) || !parseUint32(atTok, &atMs) || !parseStateToken(stTok, &active)) {
      printErr(F("BAD_LEVEL"));
      return;
    }
    if (!addEvent(sig, atMs, active)) {
      printErr(F("EVENT_OVERFLOW"));
      return;
    }
    printOk();
    return;
  }

  if (strcmp(cmd, "RUN") == 0) {
    runSchedule();
    return;
  }

  if (strcmp(cmd, "DUMP") == 0) {
    dumpEdges();
    return;
  }

  printErr(F("UNKNOWN_CMD"));
}

void setup() {
  Serial.begin(115200);
  applyPinModes();
  clearEvents();
  resetLogs();
  Serial.println(F("TickleBoardBits ready"));
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      rxBuf[rxLen] = '\0';
      if (rxLen > 0) processCommand(rxBuf);
      rxLen = 0;
    } else if (rxLen < RX_BUF_SIZE - 1) {
      rxBuf[rxLen++] = c;
    } else {
      // Overflow-safe command reset.
      rxLen = 0;
      printErr(F("RX_OVERFLOW"));
    }
  }
}
