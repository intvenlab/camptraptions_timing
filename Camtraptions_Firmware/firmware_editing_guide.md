# Camtraptions Firmware Editing Guide

## Overview
This guide explains how to modify the Camtraptions camera trap firmware, focusing on the camera logic state machine and I/O behavior. The firmware is currently at **Phase 3** with dual battery monitoring and a complete camera I/O framework.

## Source layout
The sketch is split across module files in `Camtraptions_Firmware/`:

| File | Responsibility |
|---|---|
| `Camtraptions_Firmware.ino` | `setup()` / `loop()` orchestration only |
| `config.h` | Pins, constants, shared structs/enums, extern globals |
| `build_info.h` | Compile-time build timestamp (`BUILD_*`) |
| `battery.h` / `battery.cpp` | ADC reads, SoC curves, calibration persistence |
| `storage.h` / `storage.cpp` | LittleFS: device/camera settings + telemetry counters |
| `camera.h` / `camera.cpp` | ISRs, state machine, I/O helpers, runtime pin wiring |
| `gatt.h` / `gatt.cpp` | BLE GATT, advertising beacon, write callbacks, connected scheduler |

**Note:** the BLE module is named `gatt.*` (not `ble.*`) to avoid colliding with Bluefruit's internal `ble.h` on the Arduino include path.

Debug serial pin logging: uncomment `#define DEBUG_CAMERA_LOGIC_PINS` in `config.h`.

## Current Features
- ✅ Dual battery monitoring (CR2032 internal + LiPo external)
- ✅ Camera state machine with 5 states
- ✅ Configurable camera logic via GATT/Android app
- ✅ Pass-through mode (disabled logic, direct I/O mirroring)
- ✅ State-machine mode (burst sequences, timing control)
- ✅ Open-drain outputs with high-Z idle
- ✅ Ultra-low power when idle (~150-200µA)

## Device Types
```cpp
0 = BATTERY_MONITOR  // Simple battery monitoring only
1 = CAMERA           // Full camera trap logic with I/O
2 = STROBE           // Flash/strobe unit  
3 = FOCUS_LIGHT      // Focus assist light
```

**Critical:** Camera logic only runs when `cfg.deviceType == 1`.

## Pin Configuration

### Current Pin Assignments
```cpp
A0 (BATTERY_PIN)         → Internal CR2032 battery voltage
A1 (DEVICE_BATTERY_PIN)  → External camera/device LiPo battery
D2 (FP_IN_PIN)           → Focus/Flash Pulse input (shutter trigger)
D3 (HP_IN_PIN)           → Half-Press input (focus/wake trigger)
D4 (FP_OUT_PIN)          → FP output to camera (open-drain)
D5 (HP_OUT_PIN)          → HP output to camera (open-drain)
```

### Non-Camera Devices
- Only `FP_IN_PIN` (D2) is monitored for shutter counting
- All other camera pins remain as INPUT (high-Z, low power)
- HP input, FP output, HP output are unused

## Camera State Machine

### States (CameraState enum)
```cpp
CAM_IDLE             // Waiting for HP or FP trigger
CAM_WAKE_AF          // HP asserted from wake input, waiting for FP or timeout
CAM_COLD_FP_WAIT     // FP arrived first; HP asserted, waiting for T
CAM_BURST_ACTIVE     // Firing frame sequence (FP pulses)
CAM_POST_SHUTTER_EXT // Z hold / between-sequence window
```

### State Variables
Defined in `camera.cpp` (declared `extern` in `config.h`):
```cpp
static CameraState cameraState        = CAM_IDLE;
static bool        cameraLogicActive  = false;  // Prevents sleep/advertising
static bool        activityActive     = false;  // Activity has accepted FP
static uint8_t     framesFired        = 0;      // Current frame count
static uint8_t     sequencesStartedThisActivity = 0;
static uint32_t    wakeHoldDeadlineMs = 0;      // R2/R15 hold deadline
static uint32_t    nextFrameMs        = 0;      // Next FP_OUT rising edge time
static uint32_t    fpOutReleaseMs     = 0;      // FP_OUT release time
static uint32_t    fullPressIgnoreUntilMs = 0;  // R10 FP ignore window
```

### State Machine Location
The state machine runs in `processCameraLogic()` function, called from `loop()`:

```cpp
void loop() {
  applyPendingCamCfgIfIdle();
  if (cfg.deviceType == 1 && camCfg.enabled) {
    processCameraLogic();  // State machine runs even while BLE is connected
  }

  if (isConnected) {
    serviceBleConnectedWork();  // Do BLE housekeeping without fixed delay padding
  }

  if (cameraLogicActive) {
    delay(1);  // Don't sleep while active
    return;
  }

  // Normal advertising and sleep...
}
```

## Configuration Structures

### DeviceConfig (stored in /settings.bin)
```cpp
struct DeviceConfig {
  uint8_t  version;        // SETTINGS_VERSION (currently 1)
  uint8_t  configured;     // 0=fresh, 1=configured
  char     name[21];       // Device name
  uint8_t  groupId;        // 0=none, 1-255=group
  char     groupName[21];  // Group label
  uint8_t  deviceType;     // 0-3 (see above)
  uint8_t  chemistry;      // 0=LiPo, 1=LiFePO4, 2=NiMH, 3=Alkaline
  uint8_t  cellCount;      // 1-8 cells
  uint32_t shutterCount;   // Total shutter actuations
};
```

### CameraConfig (stored in /camera.bin, 22 bytes)
```cpp
struct CameraConfig {
  uint8_t version;                      // CAMERA_SETTINGS_VERSION (3)
  uint8_t enabled;                      // 0=pass-through, 1=state-machine
  uint8_t wakeHalfPressHoldSec;         // X seconds max HP hold
  uint8_t minHalfPressBeforeShutter;    // T ×100ms min HP before shutter
  uint16_t shutterPulseDuration;        // ×10ms FP pulse width (1..3000 => 10..30000ms)
  uint16_t startFrameSpacingTicks;      // Y ×10ms pulse-end->next-start (1..3000 => 10..30000ms)
  uint8_t postShutterHpHoldTenths;      // Z ×100ms HP hold after burst
  uint8_t hpDebounceMs;                 // HP input debounce
  uint8_t fpDebounceMs;                 // FP input debounce
  uint8_t frameCount;                   // N frames per sequence (1-8)
  uint8_t maxSequenceCount;             // Max sequences before timeout lockout (1-64)
  uint8_t wakeHoldRefreshPolicy;        // legacy encoded field; currently no-op for wake deadline timing
  uint8_t halfPressDuringBurstPolicy;   // Reserved (0)
  uint8_t fullPressWithoutHpPolicy;     // 0=assertHp, 1=ignoreFP
  uint8_t activityHalfPressHoldPolicy;  // Reserved (0)
  uint8_t fpAfterMaxSeqCountPolicy;     // legacy compatibility byte
  uint8_t inputActivePolarity;          // Reserved (coerced to 0=activeLow)
  uint8_t outputDriveMode;              // Reserved (coerced to 0=openDrain)
  uint8_t powerSaveIdleMode;            // 0=disabled, 1=enabled (default)
  uint8_t fullPressIgnoreGapTenths;     // R10 ×100ms, default 31 (3.1s)
};
```

**Default values** are set in `resetCameraToDefaults()`.

## Operating Modes

### Pass-Through Mode (`camCfg.enabled == 0`)
- **Behavior:** Outputs directly mirror inputs in real-time
- **ISRs:** `onFpPassthrough()` and `onHpPassthrough()` (CHANGE trigger)
- **Outputs:** Push-pull, digitalWrite in ISR
- **Use case:** Simple cable extension, no logic processing
- **Shutter counting:** Still works on FP_IN active edge

### State-Machine Mode (`camCfg.enabled == 1`)
- **Behavior:** Full burst sequence logic with timing control
- **ISRs:** `onShutterPulse()` and `onHpPulse()` (FALLING trigger)
- **Outputs:** Open-drain (assertPin/releasePin helpers)
- **Use case:** Camera trap automation with sequences

## State Machine Flow

### CAM_IDLE
**Entry:** Device powered on, or after activity end  
**Exit to CAM_WAKE_AF:** HP trigger received  
**Exit to CAM_COLD_FP_WAIT:** FP trigger + `fullPressWithoutHpPolicy == 0`

**Actions:**
- Set `cameraLogicActive = false` (allows sleep)
- HP trigger: assert HP_OUT, start `wakeHoldDeadlineMs`, wait for FP
- Cold FP: assert HP_OUT, mark accept pending, wait T before sequence

### CAM_WAKE_AF
**Purpose:** HP asserted from wide-PIR wake, waiting for an accepted FP  
**Exit to CAM_BURST_ACTIVE:** FP accepted under R10/R12/cap gates  
**Exit to CAM_IDLE:** Wake timeout with no accepted FP

**Actions:**
- HP trigger records refresh telemetry only; wake deadline remains anchored to initial HP assert
- Timeout check: `timeReached(now, wakeHoldDeadlineMs)` before activity
- FP accept calls `startSequence()`, increments `sequencesStartedThisActivity`
- Sequence accept sets `fullPressIgnoreUntilMs`; accepted FP does not move wake hold deadline (R15)

### CAM_COLD_FP_WAIT
**Purpose:** FP arrived before HP; firmware asserts HP and waits T  
**Exit to CAM_BURST_ACTIVE:** `minHalfPressBeforeShutter` satisfied

**Actions:**
- Keep HP_OUT asserted
- Treat the original FP as the accepted sequence start after T
- Do not require a second FP input

### CAM_BURST_ACTIVE
**Purpose:** Firing `FrameCount` frames with R6 end-to-start spacing  
**Exit to CAM_POST_SHUTTER_EXT:** All frames fired and FP_OUT released

**Actions:**
- Release FP_OUT after pulse duration: `now >= fpOutReleaseMs`
- Fire next frame when: `framesFired < frameCount && now >= nextFrameMs`
- Each frame:
  - `assertPin(FP_OUT)`
  - `fpOutReleaseMs = now + shutterPulseDuration * 10`
  - `nextFrameMs = fpOutReleaseMs + startFrameSpacingTicks * 10`
- HP input during burst is ignored for scheduling (R14)
- FP input during the R10 window is ignored
- Transition when: `framesFired >= frameCount && fpOutReleaseMs == 0`

### CAM_POST_SHUTTER_EXT
**Purpose:** Hold HP through Z and between-sequence wait  
**Exit to CAM_BURST_ACTIVE:** FP accepted after burst/gap under cap  
**Exit to CAM_WAKE_AF:** Z elapsed and still under sequence cap  
**Exit to CAM_IDLE:** Sequence cap reached or explicit end-activity path

**Actions:**
- Keep HP_OUT asserted through the activity (R7/R13)
- Accept FP during Z if the prior sequence is complete, R10 elapsed, and under cap (SC-05b)
- HP pulses in post-burst hold do not move wake hold timing (SC-07b)
- Do not end solely because wake timeout elapsed while post-burst hold is still active

## Interrupt Service Routines

### State-Machine Mode ISRs

#### onShutterPulse() - FP_IN FALLING
```cpp
void onShutterPulse() {
  uint32_t now = millis();
  if (now - lastShutterMs < camCfg.fpDebounceMs) return;  // State-machine debounce
  lastShutterMs  = now;
  cfg.shutterCount++;   // Always count
  settingsDirty  = true;
  shutterUpdated = true;
  fpPulseFlag    = true;  // State machine consumes this
}
```

#### onHpPulse() - HP_IN FALLING
```cpp
void onHpPulse() {
  uint32_t now = millis();
  if (now - lastHpMs < camCfg.hpDebounceMs) return;  // Debounce
  lastHpMs    = now;
  hpPulseFlag = true;  // State machine consumes this
}
```

### Pass-Through Mode ISRs

#### onFpPassthrough() - FP_IN CHANGE
```cpp
void onFpPassthrough() {
  int state = digitalRead(FP_IN_PIN);
  digitalWrite(FP_OUT_PIN, state);  // Mirror immediately
  
  if (state == LOW) {  // Count on active edge
    uint32_t now = millis();
    if (now - lastShutterMs >= SHUTTER_DEBOUNCE_MS) {
      lastShutterMs  = now;
      cfg.shutterCount++;
      settingsDirty  = true;
      shutterUpdated = true;
    }
  }
}
```

#### onHpPassthrough() - HP_IN CHANGE
```cpp
void onHpPassthrough() {
  digitalWrite(HP_OUT_PIN, digitalRead(HP_IN_PIN));  // Mirror immediately
}
```

**ISR Rules:**
- ✅ Set flags only in state-machine mode
- ✅ Mirror pins in pass-through mode (safe, outputs pre-configured)
- ✅ Debounce all inputs
- ❌ No Serial.print()
- ❌ No BLE calls
- ❌ No delay()

## Output Control (Open-Drain)

### Helper Functions
```cpp
void assertPin(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);  // Drive low (active)
}

void releasePin(int pin) {
  pinMode(pin, INPUT);     // High-Z (inactive, pulled high by camera)
}
```

### Usage Pattern
```cpp
// Activate output
assertPin(FP_OUT_PIN);
fpOutReleaseMs = millis() + pulseDuration;

// Later, in state machine
if (millis() >= fpOutReleaseMs) {
  releasePin(FP_OUT_PIN);
  fpOutReleaseMs = 0;  // Mark as released
}
```

**Why open-drain:**
- Camera trap inputs have internal pull-ups
- Open-drain = "open collector" = safe for multi-device wiring
- Idle state is high-Z (no power draw)

## Common Editing Tasks

### Changing Timing Values

**Example: Increase FP pulse duration from 100ms to 200ms**

1. **Via Android app:** Change `shutterPulseDuration` from 10 to 20 (×10ms)
2. **Via code default:**
```cpp
void resetCameraToDefaults() {
  // ...
  camCfg.shutterPulseDuration = 20;  // Changed from 10
  // ...
}
```

### Adding a New State

1. **Add to enum:**
```cpp
enum CameraState {
  CAM_IDLE,
  CAM_WAKE_AF,
  CAM_COLD_FP_WAIT,
  CAM_BURST_ACTIVE,
  CAM_POST_SHUTTER_EXT,
  CAM_MY_NEW_STATE      // Add here
};
```

2. **Add case in `processCameraLogic()`:**
```cpp
case CAM_MY_NEW_STATE:
  cameraLogicActive = true;
  
  // Your logic here
  
  // Transition condition
  if (timeReached(now, myStateDeadlineMs)) {
    cameraState = CAM_IDLE;
  }
  break;
```

3. **Add transition from existing state:**
```cpp
case CAM_WAKE_AF:
  // ...existing code...
  
  if (someCondition) {
    wakeHoldDeadlineMs = now + wakeHalfPressHoldMs();
    cameraState = CAM_MY_NEW_STATE;
  }
  break;
```

### Adding a Configuration Parameter

1. **Add to CameraConfig struct:**
```cpp
struct CameraConfig {
  // ...existing fields...
  uint8_t myNewParameter;  // Add at end
};
// Update sizeof comment if needed
```

2. **Increment CAMERA_SETTINGS_VERSION:**
```cpp
#define CAMERA_SETTINGS_VERSION 2  // Was 1
```

3. **Set default in `resetCameraToDefaults()`:**
```cpp
void resetCameraToDefaults() {
  // ...existing defaults...
  camCfg.myNewParameter = 42;
}
```

4. **Use in state machine:**
```cpp
if (timeReached(now, myDeadlineMs + (uint32_t)camCfg.myNewParameter * 100UL)) {
  // Transition
}
```

5. **Update Android app** to read/write new parameter via GATT

### Modifying Burst Behavior

**Example: Fire frames on both rising and falling edges**

Current code fires on FP pulse (FALLING only). To fire on both:

```cpp
case CAM_BURST_ACTIVE:
  // ...existing frame firing logic...
  
  // Fire on both edges
  if (framesFired < camCfg.frameCount && now >= nextFrameMs) {
    if (framesFired % 2 == 0) {
      assertPin(FP_OUT_PIN);  // Fire on even frames
    } else {
      releasePin(FP_OUT_PIN); // Release on odd frames
    }
    // Schedule next edge
    nextFrameMs = fpOutReleaseMs + (camCfg.startFrameSpacingTicks * 10) / 2;
    framesFired++;
  }
  break;
```

### Adding Conditional Behavior

**Example: Different timing for first frame**

```cpp
case CAM_BURST_ACTIVE:
  // Release FP_OUT after pulse
  if (fpOutReleaseMs != 0 && now >= fpOutReleaseMs) {
    releasePin(FP_OUT_PIN);
    fpOutReleaseMs = 0;
  }
  
  // Fire next frame
  if (framesFired < camCfg.frameCount && now >= nextFrameMs) {
    assertPin(FP_OUT_PIN);
    
    // First frame uses different pulse duration
    uint32_t pulseMs = (framesFired == 0) 
      ? camCfg.shutterPulseDuration * 20    // 2× for first frame
      : camCfg.shutterPulseDuration * 10;   // Normal for others
    
    fpOutReleaseMs = now + pulseMs;
    nextFrameMs = fpOutReleaseMs + (camCfg.startFrameSpacingTicks * 10);
    framesFired++;
  }
  break;
```

## Battery Monitoring

### Dual Battery System

**Internal (CR2032 coin cell on A0):**
- Powers the XIAO board itself
- Read via `readBatteryVoltage()` → `readCR2032Percentage()`
- Nominal: 3.0V full, 2.5V empty
- Expected life: 40-50 days at 150µA average

**External (Camera LiPo on A1):**
- Powers the camera being controlled
- Read via `readDeviceBattery()` → `readLiPoPercentage()`
- Returns false if not present (voltage < 0.5V)
- Nominal: 4.2V full, 3.0V empty

### Advertising Both Batteries

Advertising packet includes both (13 bytes total):
```cpp
[0-1]  Company ID (0xFFFF)
[2]    Internal battery %
[3-4]  Internal voltage mV
[5]    External battery % (0xFF if not present)
[6-7]  External voltage mV (0xFFFF if not present)
[8]    Flags (configured | deviceType | chemistry)
[9]    Group ID
[10]   Cell count
[11-12] Shutter count (16-bit)
```

### Modifying Battery Curves

**CR2032 curve** (very flat discharge):
```cpp
int readCR2032Percentage(float voltage) {
  // Edit these thresholds:
  if (voltage >= 3.0f) return 100;  // Full
  if (voltage <= 2.5f) return 0;    // Empty
  
  // Edit piecewise segments here
  if (voltage >= 2.9f) return 80 + (voltage - 2.9f) * 200;
  // ...
}
```

**LiPo curve** (steeper discharge):
```cpp
int readLiPoPercentage(float voltage) {
  // Edit these thresholds:
  if (voltage > 4.2f) voltage = 4.2f;
  if (voltage < 3.0f) voltage = 3.0f;
  
  // Edit piecewise segments here
  if (voltage >= 4.1f) return 90 + (voltage - 4.1f) * 100;
  // ...
}
```

## Power Management

### Sleep Behavior

**Non-camera or idle camera:**
- `cameraLogicActive = false`
- Main loop executes normal advertising cycle
- Idle wait uses `idleWaitWithCameraWake()`; in camera power-save mode this uses interrupt-driven wake (`__WFE()`) instead of blind 950ms sleeps
- Average: ~150-200µA

**Active camera logic:**
- `cameraLogicActive = true`
- Main loop skips sleep: `delay(1)` only
- State machine processes continuously
- Average: ~10-15mA while active

### Power Save Idle Mode

When `camCfg.powerSaveIdleMode == 1`:
```cpp
void idleWaitWithCameraWake(uint32_t durationMs) {
  if (!(cfg.deviceType == 1 && camCfg.enabled && camCfg.powerSaveIdleMode)) {
    delay(durationMs);
    return;
  }
  // GPIO interrupts wake __WFE() immediately and processCameraLogic() runs on flags.
}
```

This mode still processes triggers promptly; it avoids fixed blind delays while idle.

## Debugging Techniques

### Serial Debug Output

Add at top of file:
```cpp
#define DEBUG_SERIAL
```

In `processCameraLogic()`:
```cpp
#ifdef DEBUG_SERIAL
  static CameraState lastState = CAM_IDLE;
  if (cameraState != lastState) {
    Serial.print("State: ");
    Serial.println(cameraState);
    lastState = cameraState;
  }
#endif
```

Initialize in `setup()`:
```cpp
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for USB
  Serial.println("Boot OK");
#endif
```

### LED State Indicators

Blink LED on state transitions:
```cpp
void enterState(CameraState newState) {
  cameraState = newState;
  myStateDeadlineMs = millis() + 1000UL;
  
  // Blink blue LED
  digitalWrite(LED_BLUE, LOW);   // ON
  delay(50);
  digitalWrite(LED_BLUE, HIGH);  // OFF
}
```

### Pin Monitoring

Print pin states periodically:
```cpp
#ifdef DEBUG_SERIAL
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("FP_IN:");  Serial.print(digitalRead(FP_IN_PIN));
    Serial.print(" HP_IN:"); Serial.print(digitalRead(HP_IN_PIN));
    Serial.print(" State:"); Serial.println(cameraState);
    lastPrint = millis();
  }
#endif
```

## Testing Checklist

### Before Uploading
- [ ] Increment firmware version (if releasing)
- [ ] Comment changes at top of file
- [ ] Verify `CAMERA_SETTINGS_VERSION` bumped if struct changed
- [ ] Verify `SETTINGS_VERSION` bumped if DeviceConfig changed
- [ ] Compile without errors/warnings

### After Uploading
- [ ] Verify battery monitoring works (both internal and external)
- [ ] Test with device type = Battery Monitor (ensure no camera logic)
- [ ] Test with device type = Camera, enabled = 0 (pass-through works)
- [ ] Test with device type = Camera, enabled = 1 (state machine works)
- [ ] Verify Android app can read/write camera config
- [ ] Check flash persistence (settings survive power cycle)
- [ ] Measure power consumption if critical

### Camera Logic Tests
- [ ] HP trigger enters `CAM_WAKE_AF` and does not shoot without FP
- [ ] Cold FP enters `CAM_COLD_FP_WAIT`, asserts HP, waits T, then fires burst
- [ ] FP trigger in wake state starts one sequence and does not move wake hold deadline
- [ ] Burst fires correct number of frames
- [ ] Frame spacing is pulse-end-to-next-start (`StartFrameSpacingMin`)
- [ ] Post-shutter HP hold accepts another FP after R10 if under cap
- [ ] Timeout returns to IDLE
- [ ] Sequence limit prevents additional bursts
- [ ] HP input during burst does not change schedule
- [ ] Shutter count increments correctly

### Firmware traceability quick check
| Rule / scenario | Firmware surface |
|-----------------|------------------|
| R1/R2, SC-04 | `CAM_WAKE_AF`, `wakeHoldDeadlineMs`, `wakeHoldRefreshPolicy` |
| R3/R4, SC-06 | `CAM_COLD_FP_WAIT`, `hpLeadSatisfied()` |
| R5/R6, SC-11 | `runBurstScheduler()`, `fpOutReleaseMs`, `nextFrameMs` |
| R10/R10b, SC-02/SC-03 | `fullPressIgnoreUntilMs`, `tryAcceptFp()` |
| R12/R15, SC-05/SC-05b | `startSequence()`, `sequencesStartedThisActivity` |
| R13/R14, SC-07 | `activityActive`, `CAM_BURST_ACTIVE` ignores HP input |
| SC-13/SC-14 | `hpDebounceMs`, `fpDebounceMs`, FALLING-edge FP ISR |
| SC-15 | `idleWaitWithCameraWake()` |

## Common Pitfalls

### ❌ Modifying struct without version bump
```cpp
// BAD - forgot to increment version
struct CameraConfig {
  uint8_t version;  // Still says CAMERA_SETTINGS_VERSION 1
  uint8_t newField; // Added field but didn't bump version!
```

**Fix:** Always increment version when changing struct layout.

### ❌ Forgetting device type check
```cpp
// BAD - runs for all devices
assertPin(FP_OUT_PIN);

// GOOD - only for camera
if (cfg.deviceType == 1) {
  assertPin(FP_OUT_PIN);
}
```

### ❌ Using delay() in state machine
```cpp
// BAD - blocks everything
delay(1000);

// GOOD - non-blocking
if (timeReached(millis(), myStateDeadlineMs)) {
  // Proceed
}
```

### ❌ Not checking cameraLogicActive
```cpp
// BAD - allows sleep during burst
advertiseData(...);
delay(950);  // Device sleeps mid-burst!

// GOOD - guarded in loop()
if (cameraLogicActive) {
  delay(1);
  return;  // Don't sleep
}
```

### ❌ Complex logic in ISR
```cpp
// BAD
void onHpPulse() {
  assertPin(HP_OUT_PIN);  // DON'T do state logic in ISR!
  cameraState = CAM_WAKE_AF;
}

// GOOD
void onHpPulse() {
  hpPulseFlag = true;  // Just set flag
}
```

## Summary

**Key Architecture Points:**
1. State machine in `processCameraLogic()`, called from `loop()` even when BLE is connected
2. ISRs only set flags (state-machine) or mirror pins (pass-through)
3. `cameraLogicActive` prevents sleep during processing
4. Open-drain outputs (assertPin/releasePin pattern)
5. All timing uses `millis()`, never blocking `delay()`
6. Camera code only runs when `deviceType == 1`

**Editing Workflow:**
```
1. Edit code
2. Increment version if releasing
3. Compile in Arduino IDE
4. Upload via USB
5. Test basic functions
6. Test camera-specific logic
7. Verify power consumption
8. Document changes
```

**State Machine Pattern:**
```
Consume ISR flags → Check current state → Execute state logic → 
Check transitions → Update outputs → Set next state
```