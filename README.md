# Camptraptions timing controller — documentation

This repository documents the **inline timing microcontroller** that sits between a Camtraptions (or compatible) motion trigger system and the camera shutter interface. The MCU translates noisy PIR wake pulses and full-press trigger events into stable half-press (focus/wake) and full-press (shutter) outputs.

## Start here

| If you are… | Read |
|-------------|------|
| **Customer** — “what will it do in the field?” | [Operational scenarios](docs/scenarios.md) (SC-01–SC-07) |
| **Developer** — implementing or integrating firmware | [Scenarios](docs/scenarios.md) → [Behavior spec](docs/behavior-spec.md) → [Parameters](docs/parameters.md) |
| **Integrator** — wiring and PIR setup | [Architecture](docs/architecture.md) → [PIR sensor settings](docs/pir-sensor-settings.md) |

**Audience**

| Document | Customers | Firmware / integration developers |
|----------|-----------|-----------------------------------|
| **[Scenarios](docs/scenarios.md)** | SC-01–SC-07 narratives | All SC-xx acceptance cases |
| [System overview](docs/architecture.md) | Yes | Yes |
| [Behavior specification](docs/behavior-spec.md) | Summary sections | Yes (rules R1–R15) |
| [Parameter registry](docs/parameters.md) | MCU defaults & units | Yes — scenario-derived |
| [PIR v4 settings](docs/pir-sensor-settings.md) | Camtraptions menu values | Integrator checklist |
| [Diagrams](docs/diagrams/) | High-level swim lanes | State flow & timing |

**Status:** See [parameters.md](docs/parameters.md) for MCU defaults.

## Export to Word

From the repo root (requires [Pandoc](https://pandoc.org/), [mermaid-filter](https://www.npmjs.com/package/mermaid-filter), and [wavedrom-cli](https://www.npmjs.com/package/wavedrom-cli)):

```powershell
.\scripts\export-docs.ps1
```

Outputs land in `dist/` as `.docx` (Mermaid and WaveDrom diagrams rendered as images). Bundles: `overview`, `scenarios`, `developer` (behavior spec only), `parameters`, `pir`, `diagrams`, or `manual` (all). Example: `.\scripts\export-docs.ps1 -Target parameters`

**WaveDrom timing diagrams:** JSON sources in `docs/diagrams/wavedrom/`; SVG output in `docs/diagrams/assets/`. The export script runs `wavedrom-cli` when JSON is newer than SVG. GUI editor (optional): `C:\Program Files\wavedrom-editor-v3.5.0-win-x64`.

## Quick physical model

```
[ Wide PIR ] ──HP (wake)──┐
                          ├──► [ Timing MCU ] ──HP/FP──► [ Camera ]
[ Narrow PIR ] ──FP───────┘         ▲
                                    │
                          (inline with trigger cable)
```

## Diagram index

**Default behavior:** start with [Timing sequences — Nominal path](docs/diagrams/timing-sequences.md#nominal-path-defaults) (HP latched whole activity, SC-01).

- [System swim lane (actors & responsibilities)](docs/diagrams/system-swimlane.md)
- [MCU state / decision flow](docs/diagrams/mcu-state-flow.md) — links to SC-xx
- [Timing sequence examples](docs/diagrams/timing-sequences.md) — nominal path first; exceptions labeled

