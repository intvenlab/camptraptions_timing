# End-to-End Project Workflow Audit Report

**Audit date:** 2026-05-23  
**Scope:** Customer requirements → technical specs → verification plan/results → firmware implementation → static analysis  
**Auditor:** Automated workspace audit (plan execution)

---

## Executive Summary

This project has **strong behavioral specification and bench validation infrastructure** for an embedded timing MCU. The documentation stack (behavior spec, scenarios, parameters, validation plan, TickleBoard HIL harness) is unusually complete for a hardware product of this size.

The **primary remaining weaknesses** are process and scope gaps, not core timing coverage:

1. **No original customer requirements artifact** is version-controlled — only a paraphrase.
2. **No CI or C++ static analysis gate** — quality depends on manual compile + ~15-minute HIL runs.
3. **One mandatory add-on test** (`ignoreFP` policy) is documented but not implemented as a vector.
4. **Parameter sweep (47 cases) and SC-15 budget (`run-sc15`)** are outside the published full-suite evidence set.

### Top 5 risks (release impact)

| Rank | Risk | Severity | Mitigation status |
|------|------|----------|-------------------|
| 1 | No signed/original customer requirements in repo — audit trail depends on paraphrased bullets in behavior-spec | **High** | Not mitigated |
| 2 | No automated compile/analyze CI — doc/code drift can recur without a gate | **Medium** | Partial — manual cppcheck script exists |
| 3 | Mandatory add-on `fullPressWithoutPriorHpPolicy=ignoreFP` has no test vector | **Medium** | Not mitigated |
| 4 | Parameter sweep (47 cases) not consolidated into release evidence | **Medium** | Not mitigated |
| 5 | L5 field PIR integration not evidenced in repo | **Low** | Plan defined; no artifacts |

### Release-readiness verdict

**Release-ready on published bench evidence for the 38-case BLE full suite.**

As of **2026-05-23**, the authoritative customer report (`validation-test-report.md` / `dist/Camptraptions-Timing-Test-Report.docx`) reflects a **38/38 pass** run on current firmware, including protocol add-ons and the `rejectCamCfgWrite()` GATT fix for `AO-CAMCFG-NACK-BAD-LENGTH`.

Remaining gaps for a broader release gate: parameter sweep results, SC-15 `<1 ms` budget via `run-sc15`, Android client verification, and L5 field integration.

---

## Workflow Status by Stage

| Stage | Status | Notes |
|-------|--------|-------|
| Customer requirements | **Yellow** | Paraphrased baseline only; no REQ IDs; no original quote |
| Technical requirements (R1–R15) | **Green** | [behavior-spec.md](behavior-spec.md) is coherent and normative |
| Parameters & scenarios | **Green** | Registry + SC-01..20 traceability tables present |
| Validation plan | **Green** | L0–L5 defined; camera/telemetry protocol rows reconciled to v3 |
| Test vectors & harness | **Green** | 38-case full suite + 47-case parameter sweep generator |
| Bench execution | **Green** | 38/38 pass rollup batch `20260523_124553` |
| Published report | **Green** | Detailed 38-case report + docx regenerated 2026-05-23 |
| Firmware implementation | **Green** | Modular, compiles; recent hardening documented |
| Static analysis | **Red** | No cppcheck/CI; harness has no pinned lint in requirements.txt |
| Android client | **Red** | Handoff doc only; no in-repo verification |

---

## 1. Traceability Matrix

### Customer baseline → rules → scenarios

| REQ | Customer baseline (behavior-spec) | Rules | Primary scenarios |
|-----|-----------------------------------|-------|-------------------|
| REQ-001 | HP input latches camera HP for `wakeHalfPressHoldTime` (X) | R1, R13, R15 | SC-01, SC-04, SC-04b, SC-12 |
| REQ-002 | FP with HP lead < T delays until T satisfied, then fires | R4 | SC-06, SC-08, SC-17, SC-20 |
| REQ-003 | One accepted FP → N frames at Y spacing, pulse width | R5, R6, R10 | SC-01, SC-11 |
| REQ-004 | After sequence, keep HP for Z; wake rules still apply | R11, R15 | SC-01, SC-05b |
| REQ-005 | Retrigger while HP latched when gates clear | R10b, R12 | SC-05, SC-05b |
| REQ-006 | (Extension) MaxSequenceCount timeout lockout | TimeOut | SC-09, SC-10 |

**Gap:** REQ-006 is implemented and tested but labeled "optional extension" in behavior-spec — customer sign-off not recorded.

### Scenario → vector → latest artifact → firmware surface

| Scenario | Vector | Latest rollup artifact | Status | Firmware surface |
|----------|--------|------------------------|--------|------------------|
| SC-01 | `TickleBoard/vectors/scenarios/SC-01.yaml` | `20260523_104412_SC-01-NOMINAL` | pass | `CAM_WAKE_AF`, `runBurstScheduler()`, R15 release |
| SC-02 | SC-02.yaml | `20260523_104434_SC-02-FP-DURING-SEQUENCE` | pass | `tryAcceptFp()`, `fullPressIgnoreUntilMs` |
| SC-03 | SC-03.yaml | `20260523_104456_SC-03-FP-FLOOD` | pass | R10 ignore during burst |
| SC-04 | SC-04.yaml | `20260523_104517_SC-04-WAKE-TIMEOUT` | pass | `CAM_WAKE_AF`, R2 timeout |
| SC-04b | SC-04b.yaml | `20260523_104539_SC-04B-REPEATED-HP` | pass | R1 no wake refresh |
| SC-05 | SC-05.yaml | `20260523_104607_SC-05-BACK-TO-BACK` | pass | `startSequence()`, R12 |
| SC-05b | SC-05b.yaml | `20260523_104633_SC-05B-FP-DURING-POST-HOLD` | pass | `CAM_POST_SHUTTER_EXT`, R11 |
| SC-06 | SC-06.yaml | `20260523_104658_SC-06-COLD-FP` | pass | `CAM_COLD_FP_WAIT`, R3/R4 |
| SC-07 | SC-07.yaml | `20260523_104719_SC-07-HP-DURING-BURST` | pass | R14 in `CAM_BURST_ACTIVE` |
| SC-07b | SC-07b.yaml | `20260523_104740_SC-07B-HP-DURING-POST-HOLD` | pass | R1 no refresh in post-hold |
| SC-08 | SC-08.yaml | `20260523_104803_SC-08-FP-BEFORE-HP` | pass | cold path, `fullPressWithoutPriorHpPolicy=0` |
| SC-09 | SC-09.yaml | `20260523_104824_SC-09-SEQUENCE-CAP` | pass | `tryAcceptFp()`, `beginMaxSequenceTimeout()` |
| SC-10 | SC-10.yaml | `20260523_104851_SC-10-RECOVERY-AFTER-CAP` | pass | timeout expiry, R12 recovery |
| SC-11 | SC-11.yaml | `20260523_104922_SC-11-SPACING-VS-T` | pass | R6 end-to-start spacing |
| SC-12 | SC-12.yaml | `20260523_104943_SC-12-HP-ONLY-MIN-GAP` | pass | R2 wake-only timeout |
| SC-13 | SC-13.yaml | `20260523_105005_SC-13-BOUNCE-DEBOUNCE` | pass | `onShutterPulse()` debounce |
| SC-14 | SC-14.yaml | `20260523_105026_SC-14-HELD-VS-PULSED-FP` | pass | FP debounce edge semantics |
| SC-15 | SC-15.yaml | `20260523_105047_SC-15-POWER-SAVE-BUDGET` | pass* | `idleWaitWithCameraWake()` — *vector only; full budget needs `run-sc15` |
| SC-16 | SC-16.yaml | `20260523_105108_SC-16-HP-RELEASE-AFTER-FP` | pass | R7 latched HP; `onHpPulse()` ISR assert |
| SC-17 | SC-17.yaml | `20260523_105130_SC-17-SHORT-HP-LEAD` | pass | R4 first-frame gate |
| SC-18 | SC-18.yaml | `20260523_105151_SC-18-HP-CHATTER-BURST` | pass | R14 during burst |
| SC-19 | SC-19.yaml | `20260523_105212_SC-19-NEW-EVENT-AFTER-RELEASE` | pass | R12 new activity after release |
| SC-20 | SC-20.yaml | `20260523_105244_SC-20-T-GREATER-THAN-Y` | pass | R4/R6 T vs Y |
| AO-TELEMETRY | AO-TELEMETRY-FIELD-COVERAGE.yaml | `20260523_105305` | pass | telemetry v3 payload |
| AO-BLE-SC04 | AO-BLE-CONNECTED-SC04.yaml | `20260523_105327` | pass | connected scheduling |
| AO-BLE-SC01 | AO-BLE-CONNECTED-SC01.yaml | `20260523_105349` | pass | connected scheduling |
| AO-LATENCY-HP | AO-LATENCY-HP-IN-TO-HP-OUT.yaml | `20260523_105410` | pass | `onHpPulse()` → HP OUT |
| AO-LATENCY-FP | AO-LATENCY-FP-IN-TO-FP-OUT.yaml | `20260523_105424` | pass | FP path latency |
| AO-GAP-TRIAD | AO-GAP-BOUNDARY-TRIAD.yaml | `20260523_105445` | pass | gap boundary telemetry |
| AO-GAP-CADENCE | AO-GAP-CADENCE.yaml | `20260523_105510` | pass | frame cadence |
| AO-MAX-SEQ-64 | AO-MAX-SEQUENCE-RANGE-64.yaml | `20260523_105536` | pass | `maxSequenceCount` up to 64 |
| AO-DEFERRED | AO-DEFERRED-CONFIG-WRITES.yaml | `20260523_105558` | pass | `CAMCFG_NACK_BUSY` |
| AO-FACTORY | AO-FACTORY-RESET-AND-COERCION.yaml | `20260523_105634` | pass | factory reset path |
| AO-BOUNDS-MIN | AO-BOUNDS-COERCE-MIN.yaml | `20260523_105659` | pass | write validation |
| AO-BOUNDS-MAX | AO-BOUNDS-COERCE-MAX.yaml | `20260523_105720` | pass | write validation |
| AO-NACK-VER | AO-CAMCFG-NACK-BAD-VERSION.yaml | `20260523_105749` | pass | `rejectCamCfgWrite()` |
| AO-NACK-LEN | AO-CAMCFG-NACK-BAD-LENGTH.yaml | `20260523_125948` | pass | `rejectCamCfgWrite()` GATT readback restore |
| AO-NACK-RANGE | AO-CAMCFG-NACK-OUT-OF-RANGE.yaml | `20260523_105819` | pass | range validation |

**Vector gap:** YAML vectors reference `scenario: SC-xx` but not `R-xx` — rule linkage is markdown-only.

---

## 2. Documentation Reconciliation

### Protocol version truth (authoritative: firmware + harness)

| Item | config.h / constants.py | android-client-handoff.md | validation-test-plan.md | telemetry.md |
|------|-------------------------|---------------------------|---------------------------|--------------|
| Camera config version | **3** | **3** | **3** | — |
| Camera payload length | **22 bytes** | **22 bytes** | **22 bytes** | — |
| Telemetry version | **3** | **3** | — | **3** |
| Telemetry payload length | **84 bytes** | **84 bytes** | — | **84 bytes** |

### Other doc inconsistencies

| Topic | Issue | Location |
|-------|-------|----------|
| SC-09 procedure | Describes legacy `fpAfterMaxSeqCountPolicy` variants | validation-test-plan.md |
| TimeOut rule | Implemented + tested but marked optional | behavior-spec.md vs SC-09/10 |
| Reserved policy bytes | R13/R14 name `activityHalfPressHoldPolicy`, `halfPressDuringBurstPolicy` as if selectable; firmware coerces to 0 | storage.cpp, parameters.md |
| `wakeHoldRefreshPolicy` | Retained for wire compat; behavior is fixed (no refresh) | firmware + docs mostly aligned post May 21 |
| Counter name | Legacy artifacts may reference `rejectedFpAtSequenceCapCount`; current name is `MaxSequenceExceededCount` | archived artifacts only |
| firmware_editing_guide | Mixed CAMERA_SETTINGS_VERSION 1/2/3 references in checklist | firmware_editing_guide.md |

### Reconciliation checklist

- [x] Update validation-test-plan.md camera config row to v3 / 22-byte
- [x] Update telemetry.md to TELEMETRY_VERSION=3, 84-byte payload, boot diagnostics fields
- [ ] Update SC-09 procedure to timeout-on-cap semantics
- [ ] Mark reserved policy bytes as wire-compat-only in parameters.md
- [x] Regenerate validation-test-report.md from latest full suite
- [ ] Classify TimeOut rule with customer sign-off note

---

## 3. Evidence Freshness

### Published vs bench state

| Artifact | Declared state | Actual state (audit) | Fresh? |
|----------|----------------|------------------------|--------|
| validation-test-report.md | 38 pass, May 23 BLE run | Detailed per-case assertions; batch `20260523_124553` | **Yes** |
| dist/Camptraptions-Timing-Test-Report.docx | Exported from above | Regenerated 2026-05-23 | **Yes** |
| suite_rollup.md / .json | 38 pass, 0 fail, 38 total | Batch `20260523_124553`; timing 94/94, functional 167/167, protocol 24/24 | **Yes** |
| full_run_ble_20260521_1719/ | 31/31 pass | Authoritative for its generation only | **Archived** |

### Assertion counts (latest rollup vs published report)

| Metric | Published report (May 21 archive) | Current report (May 23) |
|--------|-----------------------------------|-------------------------|
| Cases | 31 | 38 |
| Timing assertions | 86/86 | 94/94 |
| Functional assertions | 109/109 | 167/167 |
| Protocol assertions | — | 24/24 |
| Failures | 0 | 0 |

---

## 4. Verification Coverage Gaps

### Mandatory add-ons (validation-test-plan.md §552–603)

| Required add-on | Vector exists? | In full suite? | Last status |
|-----------------|----------------|----------------|-------------|
| BLE-connected SC-04 + SC-01 | Yes | Yes | pass |
| Gap boundary triad | Yes | Yes | pass |
| Gap cadence | Yes | Yes | pass |
| Mid-activity config writes (NACK_BUSY) | Yes | Yes | pass |
| Factory reset idle + active | Yes | Yes | pass |
| Reserved field coercion | Yes (AO-BOUNDS, AO-FACTORY) | Yes | pass |
| **`fullPressWithoutPriorHpPolicy=ignoreFP`** | **No** | **No** | **Not tested** |
| Camcfg NACK protocol | Yes (3 vectors) | Yes | pass (incl. BAD-LENGTH post `rejectCamCfgWrite()`) |

### Validation levels

| Level | Description | Evidenced? |
|-------|-------------|------------|
| L0 | Doc consistency / traceability matrix | Partial — matrices exist; version drift fails gate |
| L1 | Fixture functional (SC-xx pass/fail) | Yes — 38/38 in latest rollup |
| L2 | Timing accuracy | Yes — 94/94 timing assertions in rollup |
| L3 | Parameter sweeps | **No** — 47 SW-* cases exist but not in full suite or report |
| L4 | Latency / power-save | Partial — latency AO pass; SC-15 vector passes but `<1 ms` budget needs `run-sc15` |
| L5 | Field PIR integration | **No** — plan defined, no artifacts |

### Infrastructure gaps

- **No CI/CD** (no `.github/workflows/`)
- **No Android client testing** in this repo
- **Fixture resolution** ~1 ms (Arduino Uno); no automated logic-analyzer cross-check
- **BLE required** for authoritative runs; no-BLE skips ~10 cases

---

## 5. Firmware Implementation Review

**Compile check (2026-05-23):** PASS — 129828 bytes flash (16%), 15928 bytes RAM (6%).

### Rule alignment (sample)

| Rule | Implementation | Concern |
|------|----------------|---------|
| R1 | `onHpPulse()` asserts HP OUT in ISR when not latched | ISR does pin work, not flag-only |
| R2 | `CAM_WAKE_AF` timeout → release | Verified SC-04 |
| R4 | `hpLeadSatisfied()` gates first frame | Verified SC-06/17/20 |
| R6 | `runBurstScheduler()` end-to-start spacing | Verified SC-11 |
| R10/R10b | `tryAcceptFp()`, `fullPressIgnoreUntilMs` | Verified SC-02/03 |
| R13 | Activity holds HP past wake deadline | Verified SC-04/07 |
| R15 | `max(hpAssert+X, lastFrame+Z)` release | Verified SC-01 after May 21 fix |
| TimeOut | `beginMaxSequenceTimeout()` on cap exceed | Verified SC-09/10 |

### Issues found

| Severity | Issue | Location |
|----------|-------|----------|
| Medium | **ISR telemetry from FP debounce reject** — `markTelemetryChanged()` called from `onShutterPulse()` ISR | camera.cpp:418–422 |
| Medium | **ISR HP OUT assert** — `onHpPulse()` calls `assertPin()` in ISR | camera.cpp:432–440 |
| Medium | **Dead `pendingCamCfgApply` path** — never set true; immediate apply or NACK_BUSY is actual policy | camera.cpp:387–392, gatt.cpp |
| Medium | **Open BLE security** — camera config, factory reset, calibration writable with `SECMODE_OPEN` | gatt.cpp |
| Low | **Non-atomic flash writes** — delete-then-write for settings/telemetry | storage.cpp |
| Low | **Stray `Untitled` file** in firmware folder | Camtraptions_Firmware/Untitled |
| Low | **No watchdog feed** visible in main loop | Camtraptions_Firmware.ino |
| Info | `sanitizeCameraConfig()` clamps `frameCount` to 8 on load; write path rejects out-of-range via `cameraConfigHasInvalidValues()` | storage.cpp — consistent with docs |

### GATT fix (verified)

`rejectCamCfgWrite()` republishes authoritative `camCfg` after NACK, fixing ATT partial-write readback corruption for short-length writes. Isolated rerun `20260523_112115_AO-CAMCFG-NACK-BAD-LENGTH` passes.

---

## 6. Static Analysis Results

### Firmware C++

| Tool | Result |
|------|--------|
| arduino-cli compile | **PASS** (Seeeduino:nrf52:xiaonRF52840) |
| cppcheck 2.20.0 | **PASS (0 errors, 9 warnings, 0 errors)** — see below |

**Cppcheck run (2026-05-23):** `C:\Program Files\Cppcheck\cppcheck.exe` on `camera.cpp`, `gatt.cpp`, `storage.cpp`, `battery.cpp`. Repeat via `.\scripts\run-cppcheck.ps1`.

| Severity | Count | Category |
|----------|-------|----------|
| error | 0 | — |
| warning | 9 | `dangerousTypeCast` (struct pointer C-casts for BLE/LittleFS writes) |
| style | 20+ | mostly BLE callback signatures, redundant conditionals, unused vars |

**Actionable cppcheck findings (prioritized):**

| Priority | Finding | File:line | Notes |
|----------|---------|-----------|-------|
| Medium | Redundant conditional before assign | camera.cpp:481 | `else if (maxSequenceTimeoutUntilMs != 0) maxSequenceTimeoutUntilMs = 0` → simplify to unconditional clear |
| Low | Unused variables | camera.cpp:131, 148 | `holdMs`, `pulseMs` assigned but never read (likely debug scaffolding) |
| Low | Redundant policy coercion | storage.cpp:132, 134 | `if (x != 0) x = 0` → assign directly |
| Info | `dangerousTypeCast` | gatt.cpp, storage.cpp, battery.cpp | `(const uint8_t*)&struct` casts — common/acceptable for embedded APIs; use `reinterpret_cast` if cleaning up |
| Info | `constParameterCallback` | gatt.cpp (9 BLE write handlers) | Bluefruit callback typedef requires non-const `uint8_t*` — suppress or leave |
| False positive | `knownConditionTrueFalse` | build_info.h:18–21 | Month parsing from `__DATE__`; cppcheck cannot evaluate compile-time macro |

**Manual review findings (not detected by cppcheck without Arduino headers):**

- ISR calls non-ISR-safe functions (`assertPin`, `markTelemetryChanged`)
- Global mutable state across modules (testability risk)
- `memcpy` from BLE write buffer (mitigated by fixed-length validation)
- `strncat` in boot reset-reason builder (boot-only, bounded)

### TickleBoard Python harness

| Tool | Result |
|------|--------|
| `python -m compileall` | **PASS** |
| ruff (installed for audit) | **PASS** — all checks passed |
| mypy (installed for audit) | **11 errors in 1 file** — mostly `camera_config_protocol.py` type narrowing; not in requirements.txt |

**Recommendation:** Add `ruff` to dev tooling; cppcheck config for firmware; compile-only GitHub Action.

---

## 7. Remediation Backlog

| Priority | Action | Owner | Effort |
|----------|--------|-------|--------|
| ~~P0~~ | ~~Re-run full BLE suite on current firmware~~ | Validation | **Done** — `20260523_124553`, 38/38 |
| ~~P0~~ | ~~Regenerate validation-test-report.md + docx~~ | Docs | **Done** — 2026-05-23 |
| ~~P0~~ | ~~Update suite_rollup to include post-fix AO-NACK-LEN~~ | Harness/process | **Done** |
| P1 | Ingest/link original customer requirements with revision metadata | Product | External |
| ~~P1~~ | ~~Reconcile telemetry.md + validation-test-plan.md to v3 protocol~~ | Docs | **Done** |
| P1 | Add AO vector for `ignoreFP` or remove from mandatory add-ons | Validation | 1 hr |
| P2 | Add GitHub Actions compile + vector YAML parse gate | DevOps | 2–4 hrs |
| P2 | Wire cppcheck into CI (`scripts/run-cppcheck.ps1` exists; add to GitHub Actions) | DevOps | 1 hr |
| P2 | Run parameter_sweep_suite and document results | Validation | ~hours |
| P2 | Run `run-sc15` budget check and attach to SC-15 evidence | Validation | 30 min |
| P3 | Remove dead `pendingCamCfgApply` code or document legacy | Firmware | 30 min |
| P3 | L5 field test plan or explicit out-of-scope statement | Docs | 30 min |
| P3 | Delete stray `Untitled` firmware file | Firmware | 1 min |

---

## Appendix A: Workflow Diagram

See plan workflow map: requirements layer → verification layer → implementation layer, with missing static analysis / CI / L5 field layer.

## Appendix B: Audit execution log

- Traceability matrix built from behavior-spec, scenarios.md, full_validation_suite.yaml, suite_rollup.json
- Doc version grep across docs/ and constants.py
- Evidence compared: validation-test-report.md, suite_rollup, May 22/23 artifacts
- Firmware reviewed: camera.cpp, gatt.cpp, storage.cpp, config.h
- Static analysis: arduino-cli compile, cppcheck 2.20.0 (0 errors, 9 warnings), compileall, ruff, mypy
