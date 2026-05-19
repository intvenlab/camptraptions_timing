# Build Word (.docx) exports from repo Markdown + Mermaid via pandoc + mermaid-filter.
# Requires: Pandoc, Node.js, npm global mermaid-filter (mermaid-filter.cmd on Windows).
#
# Usage (from repo root):
#   .\scripts\export-docs.ps1
#   .\scripts\export-docs.ps1 -Target scenarios

param(
    [ValidateSet("all", "manual", "overview", "scenarios", "developer", "parameters", "pir", "diagrams")]
    [string]$Target = "all"
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $RepoRoot

$WaveDromDir = Join-Path $RepoRoot "docs\diagrams\wavedrom"
$WaveDromAssets = Join-Path $RepoRoot "docs\diagrams\assets"

function Add-ToolPath {
    $paths = @(
        "$env:LOCALAPPDATA\Pandoc",
        "${env:ProgramFiles}\nodejs",
        "$env:APPDATA\npm"
    )
    foreach ($p in $paths) {
        if ((Test-Path $p) -and ($env:Path -notlike "*$p*")) {
            $env:Path = "$p;$env:Path"
        }
    }
}

function Assert-Tool([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required tool not on PATH: $Name. Install it or open a shell where it is available."
    }
}

Add-ToolPath
Assert-Tool pandoc
Assert-Tool node
if (-not (Get-Command mermaid-filter.cmd -ErrorAction SilentlyContinue)) {
    throw "mermaid-filter.cmd not found. Run: npm install -g mermaid-filter"
}

$OutDir = Join-Path $RepoRoot "dist"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

function Render-WaveDromAssets {
    if (-not (Get-Command wavedrom-cli -ErrorAction SilentlyContinue)) {
        Write-Warning "wavedrom-cli not on PATH; skipping WaveDrom render (use committed SVGs in docs/diagrams/assets)."
        return
    }
    if (-not (Test-Path $WaveDromDir)) { return }
    New-Item -ItemType Directory -Force -Path $WaveDromAssets | Out-Null
    Get-ChildItem -Path $WaveDromDir -Filter "*.json" | ForEach-Object {
        $svg = Join-Path $WaveDromAssets ($_.BaseName + ".svg")
        $render = (-not (Test-Path $svg)) -or ($_.LastWriteTimeUtc -gt (Get-Item $svg).LastWriteTimeUtc)
        if ($render) {
            Write-Host "WaveDrom: $($_.Name) -> $(Split-Path -Leaf $svg)"
            & wavedrom-cli -i $_.FullName -s $svg
        }
    }
}

Render-WaveDromAssets

$FilterArgs = @("--filter", "mermaid-filter.cmd")
$Meta = @(
    "--standalone",
    "--metadata", "author=Camptraptions timing documentation"
)

function Export-Docx([string]$OutputName, [string[]]$Inputs) {
    $out = Join-Path $OutDir $OutputName
    $missing = $Inputs | Where-Object { -not (Test-Path $_) }
    if ($missing) {
        throw "Missing input file(s): $($missing -join ', ')"
    }
    Write-Host "Building $OutputName ..."
    $args = $Inputs + @("-o", $out) + $Meta + $FilterArgs
    & pandoc @args
    if (-not (Test-Path $out)) {
        throw "Pandoc did not create $out"
    }
    $kb = [math]::Round((Get-Item $out).Length / 1KB, 1)
    Write-Host "  -> $out ($kb KB)"
}

$Builds = @{
    overview = @{
        File = "Camptraptions-Timing-Overview.docx"
        Inputs = @("README.md", "docs/architecture.md")
    }
    scenarios = @{
        File = "Camptraptions-Timing-Scenarios.docx"
        Inputs = @("docs/scenarios.md")
    }
    developer = @{
        File = "Camptraptions-Timing-Developer.docx"
        Inputs = @("docs/behavior-spec.md")
    }
    parameters = @{
        File = "Camptraptions-Timing-Parameters.docx"
        Inputs = @("docs/parameters.md")
    }
    pir = @{
        File = "Camptraptions-Timing-PIR-Settings.docx"
        Inputs = @("docs/pir-sensor-settings.md")
    }
    diagrams = @{
        File = "Camptraptions-Timing-Diagrams.docx"
        Inputs = @(
            "docs/diagrams/system-swimlane.md",
            "docs/diagrams/mcu-state-flow.md",
            "docs/diagrams/timing-sequences.md"
        )
    }
    manual = @{
        File = "Camptraptions-Timing-Manual.docx"
        Inputs = @(
            "README.md",
            "docs/architecture.md",
            "docs/scenarios.md",
            "docs/behavior-spec.md",
            "docs/parameters.md",
            "docs/pir-sensor-settings.md",
            "docs/diagrams/system-swimlane.md",
            "docs/diagrams/mcu-state-flow.md",
            "docs/diagrams/timing-sequences.md"
        )
    }
}

$toRun = if ($Target -eq "all") {
    @("overview", "scenarios", "developer", "parameters", "pir", "diagrams", "manual")
} else {
    @($Target)
}

foreach ($key in $toRun) {
    $b = $Builds[$key]
    Export-Docx -OutputName $b.File -Inputs $b.Inputs
}

Write-Host ""
Write-Host "Done. Output folder: $OutDir"
