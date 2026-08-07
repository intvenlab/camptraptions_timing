# HP Relax Transition Spec

This document defines adaptive half-press (HP) handling: preserve continuous-HP behavior when settings imply it, and allow HP deassert/reassert between frames when settings permit it. This is the normative specification and validation contract for the implemented firmware revision.

## Goal

1. Keep backward-compatible behavior for existing "stock" settings that already produce continuous HP.
2. Enable settings-driven HP deassert between frames without adding a new mode byte.
3. Guarantee `minHalfPressBeforeShutter` before every FP output pulse.
4. Eliminate trigger-loss edge case for simultaneous HP+FP arrival.

## Parameter semantics (redefined)

`PostShutterHalfPressHoldTimeExtension` (`Z`) is redefined as:

- HP hold duration after **each** FP output pulse end, not only after the last pulse.
- Applies to inter-frame gaps and post-final-frame gap with identical semantics.
- Planned valid range for new behavior: `0..200` ticks (`0..20.0 s` in 100 ms units).

`StartFrameSpacingMin` (`Y`) remains the minimum time from FP pulse end to next FP pulse start.
`minHalfPressBeforeShutter` (`T`) remains a hard precondition before each FP output pulse.

## Normative timing model

For frame `n+1` after frame `n`:

- `frame_n_release = t_release[n]`
- `scheduled_next_start = frame_n_release + Y`
- `required_hp_assert_time = scheduled_next_start - T`
- `hp_hold_until = frame_n_release + Z`

Rules:

1. HP must be asserted at or before `required_hp_assert_time`.
2. If HP has already been asserted for at least `T` by `scheduled_next_start`, fire on `scheduled_next_start`.
3. Otherwise delay the FP start until `T` is satisfied.

### Continuous vs relaxed HP condition

Inter-frame HP can deassert only if:

`Z < (Y - T - guard)`

Where `guard` is an implementation safety margin (recommended 20 ms initial value, tune by fixture evidence).

Equivalent:

- If `Z >= (Y - T - guard)`: HP stays continuously asserted between those frames.
- If `Z < (Y - T - guard)`: HP deassert between frames is allowed; firmware must reassert HP before next frame to satisfy `T`.

### Examples

| `T` | `Y` | `Z` | Expected HP between frames |
|---|---|---|---|
| 500 ms | 1000 ms | 0 ms | Deassert allowed; reassert before next frame |
| 700 ms | 1000 ms | 500 ms | Continuous HP (no inter-frame drop) |
| 500 ms | 1000 ms | 1200 ms | Continuous HP (Z exceeds Y) |
| 500 ms | 1000 ms | 400 ms | Boundary region; depends on guard and measured jitter |

## Trigger intent handling

### Simultaneous HP+FP at idle

If HP and FP arrive in the same scheduler cycle, treat as valid shoot intent:

1. Assert HP immediately.
2. Start sequence.
3. Gate first FP output by `T` as usual.

No dropped sequence due to branch-ordering race is allowed.

### Short wake hold with FP intent

If `wakeHalfPressHoldTime` is short, accepted FP intent still proceeds under sequence rules. Wake timeout must not terminate an accepted sequence path.

## Backward-compatibility expectation

With existing stock profile (`X=10 s, T=0.5 s, Y=1.0 s, Z=2.0 s`), behavior remains effectively continuous HP, matching current field expectations.

## Validation contract

This change is complete only when:

1. Full existing suite still passes where expectations are unchanged.
2. New HP-relax vectors pass for both "relax allowed" and "relax blocked" regimes.
3. Simultaneous HP+FP vector passes (sequence starts; first frame timing obeys `T`).

See `docs/validation-test-plan.md` (HP relax transition section) and `TickleBoard/vectors/suites/hp_relax_transition_suite.yaml`.
