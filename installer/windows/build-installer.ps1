# installer/build-installer.ps1
# Builds rhodesia-setup-0.1.0.exe from the pre-compiled binary on the
# latest GitHub release, then packages it via Inno Setup.
#
# Pipeline: download rhodesia_win64.exe from releases/latest -> ISCC package
# Output:   installer/Output/rhodesia-setup-0.1.0.exe
#
# Edit $RepoSlug / $AssetName below to point at a different release.

[CmdletBinding()]
param(
    [string]$InnoSetupPath,                 # autodetect if empty
    [switch]$Clean                          # wipe staging/ first
)

$ErrorActionPreference = "Stop"

# Release asset configuration
$RepoSlug  = "giulicrenna/rhodesia-lang"
$AssetName = "rhodesia_win64.exe"

# Paths (absolute, computed once)
$ScriptDir  = (Resolve-Path $PSScriptRoot).Path
$RepoRoot   = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path
$StagingDir = Join-Path $ScriptDir "staging"
$SrcDir     = Join-Path $StagingDir "src"
$BuildDir   = Join-Path $SrcDir "build"
$OutputDir  = Join-Path $ScriptDir "Output"
$IssPath    = Join-Path $ScriptDir "rhodesia.iss"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "=== $Message ===" -ForegroundColor Cyan
}

function Find-InnoSetup {
    param([string]$Hint)
    if ($Hint) {
        if (Test-Path $Hint) { return $Hint }
        throw "ISCC.exe not found at '$Hint'."
    }
    $candidates = @(
        (Join-Path $env:ProgramFiles "Inno Setup 6\ISCC.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe"),
        (Join-Path $env:ProgramFiles "Inno Setup 5\ISCC.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 5\ISCC.exe")
    )
    foreach ($p in $candidates) {
        if ($p -and (Test-Path $p)) { return $p }
    }
    $fromPath = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue
    if ($fromPath) { return $fromPath.Source }
    return $null
}

try {
    Write-Step "Checking prerequisites"
    $iscc = Find-InnoSetup -Hint $InnoSetupPath
    if (-not $iscc) {
        throw "ISCC.exe not found. Install Inno Setup 6 from https://jrsoftware.org/isinfo.php or pass -InnoSetupPath."
    }
    Write-Host "  Inno Setup: $iscc"

    if ($Clean -and (Test-Path $StagingDir)) {
        Write-Step "Cleaning staging directory"
        Remove-Item -Recurse -Force $StagingDir
    }

    Write-Step "Downloading $AssetName from latest release"
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
    $assetUrl = "https://github.com/$RepoSlug/releases/latest/download/$AssetName"
    $exeDest  = Join-Path $BuildDir "rhodesia.exe"
    Write-Host "  $assetUrl"
    try {
        Invoke-WebRequest -Uri $assetUrl -OutFile $exeDest -UseBasicParsing
    } catch {
        throw "Could not download $AssetName from $assetUrl. Has a Windows binary been published to the latest release? ($($_.Exception.Message))"
    }
    $downloadedBytes = (Get-Item $exeDest).Length
    if ($downloadedBytes -lt 1MB) {
        throw "Downloaded $AssetName is only $downloadedBytes bytes - looks like an error page, not a binary."
    }
    Write-Host ("  Downloaded {0:N1} MB" -f ($downloadedBytes / 1MB))

    Write-Step "Staging support files from local checkout"
    New-Item -ItemType Directory -Force -Path $SrcDir | Out-Null
    Copy-Item -Path (Join-Path $RepoRoot "LICENSE")  -Destination (Join-Path $SrcDir "LICENSE")  -Force
    Copy-Item -Path (Join-Path $RepoRoot "README.md") -Destination (Join-Path $SrcDir "README.md") -Force
    $libsSrc = Join-Path $RepoRoot "libs"
    $libsDst = Join-Path $SrcDir "libs"
    if (Test-Path $libsSrc) {
        New-Item -ItemType Directory -Force -Path $libsDst | Out-Null
        Copy-Item -Path "$libsSrc\*" -Destination $libsDst -Recurse -Force
    }

    $exePath = Join-Path $BuildDir "rhodesia.exe"
    if (-not (Test-Path $exePath)) {
        throw "Downloaded asset not found at $exePath."
    }

    Write-Step "Running Inno Setup Compiler"
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    Push-Location $ScriptDir
    try {
        & $iscc $IssPath
        if ($LASTEXITCODE -ne 0) { throw "ISCC failed." }
    } finally {
        Pop-Location
    }

    $setupExe = Join-Path $OutputDir "rhodesia-setup-0.1.0.exe"
    Write-Host ""
    if (Test-Path $setupExe) {
        $sizeMb = (Get-Item $setupExe).Length / 1MB
        Write-Host ("Done. Installer: {0}  ({1:N1} MB)" -f $setupExe, $sizeMb) -ForegroundColor Green
    } else {
        Write-Host "Done. Check $OutputDir for output files." -ForegroundColor Green
    }
} catch {
    Write-Host ""
    Write-Host "ERROR: $_" -ForegroundColor Red
    exit 1
}
