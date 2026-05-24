# Run cppcheck on Camtraptions_Firmware C++ sources (no Arduino headers required).
# Requires cppcheck on PATH or at default install location.

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$FwDir = Join-Path $RepoRoot "Camtraptions_Firmware"

$Cppcheck = $null
if (Get-Command cppcheck -ErrorAction SilentlyContinue) {
    $Cppcheck = "cppcheck"
} elseif (Test-Path "C:\Program Files\Cppcheck\cppcheck.exe") {
    $Cppcheck = "C:\Program Files\Cppcheck\cppcheck.exe"
} else {
    Write-Error "cppcheck not found. Install from https://cppcheck.sourceforge.io/ or add to PATH."
}

$Sources = @(
    (Join-Path $FwDir "camera.cpp"),
    (Join-Path $FwDir "gatt.cpp"),
    (Join-Path $FwDir "storage.cpp"),
    (Join-Path $FwDir "battery.cpp")
)

& $Cppcheck --version
& $Cppcheck `
    --enable=warning,style,performance,portability `
    --inconclusive `
    --std=c++11 `
    --force `
    --suppress=missingIncludeSystem `
    @Sources

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
