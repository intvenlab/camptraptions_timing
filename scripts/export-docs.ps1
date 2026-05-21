# Build Word (.docx) exports from repo Markdown + Mermaid via pandoc + mermaid-filter.
# Requires: Pandoc, Node.js, npm global mermaid-filter (mermaid-filter.cmd on Windows), wavedrom-cli.
#
# Usage (from repo root):
#   .\scripts\export-docs.ps1
#   .\scripts\export-docs.ps1 -Target scenarios
#   .\scripts\export-docs.ps1 -Target validation
#   .\scripts\export-docs.ps1 -Sequential   # disable parallel pandoc (mermaid temp conflicts)

param(
    [ValidateSet("all", "manual", "overview", "scenarios", "developer", "parameters", "pir", "diagrams", "validation", "validationreport")]
    [string]$Target = "all",
    [switch]$Sequential
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $RepoRoot

$WaveDromDir = Join-Path $RepoRoot "docs\diagrams\wavedrom"
$WaveDromAssets = Join-Path $RepoRoot "docs\diagrams\assets"
$OutDir = Join-Path $RepoRoot "dist"
$FilterArgs = @("--filter", "mermaid-filter.cmd")
$Meta = @(
    "--standalone"
)
$PandocResourcePath = "docs/diagrams;."

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

function Scale-WaveDromPng([string]$PngPath, [double]$Factor = 2) {
    Add-Type -AssemblyName System.Drawing
    $src = [System.Drawing.Image]::FromFile($PngPath)
    try {
        $w = [int]($src.Width * $Factor)
        $h = [int]($src.Height * $Factor)
        $bmp = New-Object System.Drawing.Bitmap $w, $h
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $g.DrawImage($src, 0, 0, $w, $h)
        $g.Dispose()
        $src.Dispose()
        $bmp.Save($PngPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $bmp.Dispose()
    } finally {
        if ($src) { $src.Dispose() }
    }
}

$WaveDromFileBlock = {
    param([string]$JsonPath, [string]$AssetsDir, [string]$ToolPath)
    $env:Path = $ToolPath
    $base = [System.IO.Path]::GetFileNameWithoutExtension($JsonPath)
    $svg = Join-Path $AssetsDir ($base + ".svg")
    $png = Join-Path $AssetsDir ($base + ".png")
    $jsonTime = (Get-Item $JsonPath).LastWriteTimeUtc
    $render = (-not (Test-Path $svg)) -or (-not (Test-Path $png)) -or
        ($jsonTime -gt (Get-Item $svg).LastWriteTimeUtc)
    if (-not $render) { return }
    Write-Host "WaveDrom: $(Split-Path -Leaf $JsonPath) -> $base.svg, $base.png"
    & wavedrom-cli -i $JsonPath -s $svg -p $png
    if (Test-Path $png) {
        Scale-WaveDromPng -PngPath $png -Factor 2
    }
}

function Render-WaveDromAssets([string]$ToolPath) {
    if (-not (Get-Command wavedrom-cli -ErrorAction SilentlyContinue)) {
        Write-Warning "wavedrom-cli not on PATH; skipping WaveDrom render (use committed PNGs in docs/diagrams/assets)."
        return
    }
    if (-not (Test-Path $WaveDromDir)) { return }
    New-Item -ItemType Directory -Force -Path $WaveDromAssets | Out-Null
    $jsonFiles = @(Get-ChildItem -Path $WaveDromDir -Filter "*.json")
    if ($jsonFiles.Count -eq 0) { return }

    foreach ($f in $jsonFiles) {
        & $WaveDromFileBlock $f.FullName $WaveDromAssets $ToolPath
    }
}

$ExportDocxBlock = {
    param(
        [string]$OutputName,
        [string[]]$Inputs,
        [string]$WorkRoot,
        [string]$DistDir,
        [string]$ToolPath,
        [string[]]$Meta,
        [string[]]$FilterArgs,
        [string]$ResourcePath
    )
    $env:Path = $ToolPath
    $out = Join-Path $DistDir $OutputName
    $missing = $Inputs | Where-Object { -not (Test-Path (Join-Path $WorkRoot $_)) }
    if ($missing) {
        throw "Missing input file(s): $($missing -join ', ')"
    }
    $jobTemp = Join-Path $env:TEMP "camptraptions-export-$([Guid]::NewGuid().ToString('N').Substring(0, 8))"
    New-Item -ItemType Directory -Force -Path $jobTemp | Out-Null
    try {
        $prevTemp = $env:TEMP
        $env:TEMP = $jobTemp
        $env:TMP = $jobTemp
        Set-Location $WorkRoot
        Write-Host "Building $OutputName ..."
        $pandocArgs = $Inputs + @("-o", $out) + $Meta + $FilterArgs + @("--resource-path", $ResourcePath)
        & pandoc @pandocArgs
        if (-not (Test-Path $out)) {
            throw "Pandoc did not create $out"
        }
        $kb = [math]::Round((Get-Item $out).Length / 1KB, 1)
        Write-Host "  -> $out ($kb KB)"
    } finally {
        $env:TEMP = $prevTemp
        $env:TMP = $prevTemp
        Set-Location $WorkRoot
        Remove-Item -Recurse -Force $jobTemp -ErrorAction SilentlyContinue
    }
}

function Export-Docx([string]$OutputName, [string[]]$Inputs, [string]$WorkRoot, [string]$DistDir, [string]$ToolPath) {
    & $ExportDocxBlock $OutputName $Inputs $WorkRoot $DistDir $ToolPath $Meta $FilterArgs $PandocResourcePath
}

Add-ToolPath
$script:EnvPath = $env:Path
Assert-Tool pandoc
Assert-Tool node
if (-not (Get-Command mermaid-filter.cmd -ErrorAction SilentlyContinue)) {
    throw "mermaid-filter.cmd not found. Run: npm install -g mermaid-filter"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

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
        Inputs = @(
            "docs/parameters.md",
            "docs/telemetry.md"
        )
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
    validation = @{
        File = "Camptraptions-Timing-Validation-Test-Plan.docx"
        Inputs = @("docs/validation-test-plan.md")
    }
    validationreport = @{
        File = "Camptraptions-Timing-Test-Report.docx"
        Inputs = @("docs/validation-test-report.md")
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
            "docs/validation-test-plan.md",
            "docs/diagrams/system-swimlane.md",
            "docs/diagrams/mcu-state-flow.md",
            "docs/diagrams/timing-sequences.md"
        )
    }
}

$toRun = if ($Target -eq "all") {
    @("overview", "scenarios", "developer", "parameters", "pir", "validation", "validationreport", "diagrams", "manual")
} else {
    @($Target)
}

if ($toRun | Where-Object { $_ -in @("all", "diagrams", "manual") }) {
    Render-WaveDromAssets -ToolPath $script:EnvPath
}

$buildItems = $toRun | ForEach-Object {
    [PSCustomObject]@{
        Key    = $_
        File   = $Builds[$_].File
        Inputs = $Builds[$_].Inputs
    }
}

$throttle = [Math]::Min(4, $buildItems.Count)
$wantParallel = (-not $Sequential) -and ($buildItems.Count -gt 1)

if ($wantParallel -and $PSVersionTable.PSVersion.Major -ge 7) {
    Write-Host "Parallel export ($($buildItems.Count) targets, throttle $throttle, PS 7) ..."
    $results = $buildItems | ForEach-Object -Parallel -ThrottleLimit $throttle {
        $item = $_
        try {
            & $using:ExportDocxBlock $item.File $item.Inputs $using:RepoRoot $using:OutDir $using:EnvPath $using:Meta $using:FilterArgs $using:PandocResourcePath
            [PSCustomObject]@{ Key = $item.Key; Ok = $true; Error = $null }
        } catch {
            [PSCustomObject]@{ Key = $item.Key; Ok = $false; Error = $_.Exception.Message }
        }
    }
    $failed = @($results | Where-Object { -not $_.Ok })
    if ($failed.Count -gt 0) {
        $msg = ($failed | ForEach-Object { "$($_.Key): $($_.Error)" }) -join "`n"
        throw "Export failed:`n$msg"
    }
} elseif ($wantParallel) {
    Write-Host "Parallel export ($($buildItems.Count) targets, throttle $throttle, Start-Job) ..."
    $init = {
        param($ToolPath)
        $env:Path = $ToolPath
    }
    $jobs = foreach ($item in $buildItems) {
        Start-Job -InitializationScript $init -ScriptBlock $ExportDocxBlock -ArgumentList @(
            $item.File, $item.Inputs, $RepoRoot, $OutDir, $script:EnvPath, $Meta, $FilterArgs, $PandocResourcePath
        )
    }
    $completed = Wait-Job -Job $jobs -Timeout 3600
    $failed = @()
    foreach ($j in $jobs) {
        if ($j.State -eq "Failed") {
            $err = Receive-Job -Job $j -ErrorAction SilentlyContinue 2>&1
            $failed += "job: $err"
        } elseif ($j.State -ne "Completed") {
            $failed += "job did not complete (state=$($j.State))"
        } else {
            Receive-Job -Job $j -ErrorAction Stop | Out-Null
        }
        Remove-Job -Job $j -Force
    }
    if ($failed.Count -gt 0) {
        throw "Export failed:`n$($failed -join "`n")"
    }
} else {
    foreach ($item in $buildItems) {
        Export-Docx -OutputName $item.File -Inputs $item.Inputs -WorkRoot $RepoRoot -DistDir $OutDir -ToolPath $script:EnvPath
    }
}

Write-Host ""
Write-Host "Done. Output folder: $OutDir"
