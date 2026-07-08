# FirmwareUf2Flasher Prototype

Simple Windows prototype to flash a `.uf2` firmware file to the XIAO nRF52840 and verify boot output strictly.

## Features

- Browse and select UF2 file
- Select runtime COM port
- Trigger bootloader with 1200-baud serial open/close
- Detect UF2 drive and copy firmware
- Strict verification: requires `[BOOT]` line and exact `build=YYYY-MM-DD HH:MM:SS` token match
- Timestamped debug output pane for operator troubleshooting

## Run (developer)

From repo root:

```powershell
dotnet run --project .\tools\FirmwareUf2Flasher\FirmwareUf2Flasher.csproj
```

## Publish (USB delivery)

From repo root:

```powershell
dotnet publish .\tools\FirmwareUf2Flasher\FirmwareUf2Flasher.csproj -c Release -r win-x64 --self-contained true
```

Published files appear under:

`tools\FirmwareUf2Flasher\bin\Release\net8.0-windows\win-x64\publish\`

## Operator Workflow

1. Plug in target device.
2. Launch app.
3. Select `.uf2` firmware file.
4. Select device COM port.
5. Enter expected build token exactly as firmware boot line prints it (`YYYY-MM-DD HH:MM:SS`).
6. Click **Upload**.
7. Confirm final status is **PASS** and log includes matching `[BOOT] ... build=<token>`.
