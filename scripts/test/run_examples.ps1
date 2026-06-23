#Requires -Version 5.1
<#
.SYNOPSIS
  Corre todos los .rho bajo examples\ y reporta pass/fail.
.DESCRIPTION
  Equivalente Windows (PowerShell) de scripts/test/run_examples.sh.
  Misma interfaz: modos `all` / `fast`, y los flags -Runner, -Timeout,
  -IncludeInteractive, -FailFast, -h / -Help.
.NOTES
  Funciona con Windows PowerShell 5.1 (incluido en Windows 10/11).
  Usa System.Diagnostics.Process directo (no Start-Process) porque este
  último devuelve ExitCode = $null con binarios nativos cuando hay
  redirección de stdout/stderr a archivo.
#>

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet('all', 'fast')]
    [string]$Mode = 'all',

    [string]$Runner,
    [int]$Timeout = 20,
    [switch]$IncludeInteractive,
    [switch]$FailFast,
    [switch]$Help
)

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Localización del repo y runner por defecto
# ---------------------------------------------------------------------------
$ScriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir     = (Resolve-Path (Join-Path $ScriptDir '..\..')).Path
$ExamplesDir = Join-Path $RootDir 'examples'

if (-not $Runner) {
    $Runner = Join-Path $RootDir 'rhodesia.exe'
}

# ---------------------------------------------------------------------------
# Ayuda
# ---------------------------------------------------------------------------
function Print-Help {
    @'
Uso:
  powershell -ExecutionPolicy Bypass -File scripts\test\run_examples.ps1 [all|fast] [opciones]

Modos:
  all   Corre todos los .rho dentro de examples\ (por defecto)
  fast  Excluye examples\10_applications\ para una corrida más rápida

Opciones:
  -Runner <path>             Ruta al ejecutable de rhodesia (default: .\rhodesia.exe)
  -Timeout <segundos>        Timeout por archivo (default: 20)
  -IncludeInteractive        Incluye ejemplos interactivos (por defecto se excluyen)
  -FailFast                  Corta en el primer error
  -h, -Help                  Muestra esta ayuda

Ejemplos:
  powershell -ExecutionPolicy Bypass -File scripts\test\run_examples.ps1
  powershell -ExecutionPolicy Bypass -File scripts\test\run_examples.ps1 fast
  powershell -ExecutionPolicy Bypass -File scripts\test\run_examples.ps1 all -Timeout 10
'@
}

if ($Help) {
    Print-Help
    exit 0
}

# ---------------------------------------------------------------------------
# Validaciones tempranas
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $Runner)) {
    Write-Host "[error] No se encontró ejecutable: $Runner"
    Write-Host "        Compilá primero (Strawberry + cmake):"
    Write-Host "          cmake -S . -B . -G Ninja ; ninja"
    exit 2
}

if ($Timeout -le 0) {
    Write-Host "[error] -Timeout debe ser un entero positivo"
    exit 2
}

# ---------------------------------------------------------------------------
# Helper: lanza rhodesia.exe con timeout, devuelve ok/exitCode/timeout/stdout/stderr
# ---------------------------------------------------------------------------
function Invoke-Rhodesia {
    param(
        [string]$ExePath,
        [string]$ScriptPath,
        [int]$TimeoutSec
    )

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName               = $ExePath
    $psi.Arguments              = '"' + $ScriptPath + '"'
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError  = $true
    $psi.RedirectStandardInput  = $true   # close stdin immediately → EOF for io.input
    $psi.UseShellExecute        = $false
    $psi.CreateNoWindow         = $true
    # Rhodesia resuelve paths relativos (include, libs/) desde el cwd.
    $psi.WorkingDirectory       = (Get-Location).Path

    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi

    try {
        $proc.Start() | Out-Null
    }
    catch {
        return @{ Ok = $false; ExitCode = $null; TimedOut = $false; Stdout = ''; Stderr = "No se pudo lanzar: $($_.Exception.Message)" }
    }

    # Close stdin right away so the child sees EOF instead of blocking
    # on a (non-existent) keyboard. io.input already returns "" on EOF.
    try { $proc.StandardInput.Close() } catch { }

    # Leer stdout/stderr asincrónicamente antes de WaitForExit para no
    # bloquear si el proceso llena los pipes.
    $stdoutTask = $proc.StandardOutput.ReadToEndAsync()
    $stderrTask = $proc.StandardError.ReadToEndAsync()

    $exited = $proc.WaitForExit($TimeoutSec * 1000)

    if (-not $exited) {
        try { $proc.Kill() } catch { }
        # Dar tiempo a que los ReadToEndAsync terminen tras el kill.
        $null = $stdoutTask.Wait(2000)
        $null = $stderrTask.Wait(2000)
        return @{
            Ok       = $false
            ExitCode = $null
            TimedOut = $true
            Stdout   = if ($stdoutTask.IsCompleted) { $stdoutTask.Result } else { '' }
            Stderr   = if ($stderrTask.IsCompleted) { $stderrTask.Result } else { '' }
        }
    }

    # Proceso terminó por sí solo: leer lo que quede en los pipes.
    $null = $stdoutTask.Wait(2000)
    $null = $stderrTask.Wait(2000)

    return @{
        Ok       = ($proc.ExitCode -eq 0)
        ExitCode = $proc.ExitCode
        TimedOut = $false
        Stdout   = $stdoutTask.Result
        Stderr   = $stderrTask.Result
    }
}

# ---------------------------------------------------------------------------
# Recolectar archivos .rho
# ---------------------------------------------------------------------------
$allFiles = Get-ChildItem -LiteralPath $ExamplesDir -Recurse -File -Filter '*.rho' |
            Sort-Object FullName |
            ForEach-Object { $_.FullName }

if ($allFiles.Count -eq 0) {
    Write-Host "[error] No se encontraron archivos .rho en $ExamplesDir"
    exit 2
}

$files = foreach ($f in $allFiles) {
    $rel = $f.Substring($RootDir.Length).TrimStart('\', '/')

    if ($Mode -eq 'fast' -and $rel -like 'examples\10_applications\*') { continue }
    if (-not $IncludeInteractive -and ($rel -like '*\interactive.rho')) { continue }
    # Long-running services: blocked on accept()/listen() loops, never exit
    # on their own. Skip unless the user explicitly opts in.
    if (-not $IncludeInteractive -and ($rel -like '*\tcp_server.rho')) { continue }

    [pscustomobject]@{ Full = $f; Rel = $rel }
}

if (-not $files -or $files.Count -eq 0) {
    Write-Host "[warn] No hay archivos para ejecutar con los filtros actuales"
    exit 0
}

# ---------------------------------------------------------------------------
# Cabecera
# ---------------------------------------------------------------------------
$width = '=' * 60
Write-Host $width
Write-Host "Rhodesia examples runner"
Write-Host ("Mode:        {0}" -f $Mode)
Write-Host ("Runner:      {0}" -f $Runner)
Write-Host ("Timeout:     {0}s por archivo" -f $Timeout)
Write-Host ("Files:       {0}" -f $files.Count)
Write-Host ("Interactive: {0}" -f ($(if ($IncludeInteractive) { 'incluidos' } else { 'excluidos' })))
Write-Host $width

# ---------------------------------------------------------------------------
# Loop principal
# ---------------------------------------------------------------------------
$total  = 0
$passed = 0
$failed = 0
$sw     = [System.Diagnostics.Stopwatch]::StartNew()

foreach ($entry in $files) {
    $total++
    $rel  = $entry.Rel
    $path = $entry.Full

    $r = Invoke-Rhodesia -ExePath $Runner -ScriptPath $path -TimeoutSec $Timeout

    if ($r.Ok) {
        $passed++
        Write-Host ("[{0,3}/{1,3}] [ok]   {2}" -f $total, $files.Count, $rel)
    }
    else {
        $failed++
        $reason = if ($r.TimedOut) { 'timeout' } elseif ($null -eq $r.ExitCode) { 'no exit' } else { "exit $($r.ExitCode)" }
        Write-Host ("[{0,3}/{1,3}] [fail] {2} ({3})" -f $total, $files.Count, $rel, $reason)

        $stderrText = if ($r.Stderr) { $r.Stderr.Trim() } else { '' }
        $stdoutText = if ($r.Stdout) { $r.Stdout.Trim() } else { '' }

        if ($stderrText) {
            Write-Host '  --- stderr ---'
            ($stderrText -split "`r?`n" | Select-Object -First 10) | ForEach-Object { Write-Host ("  {0}" -f $_) }
        }
        elseif ($stdoutText) {
            Write-Host '  --- output ---'
            ($stdoutText -split "`r?`n" | Select-Object -First 10) | ForEach-Object { Write-Host ("  {0}" -f $_) }
        }

        if ($FailFast) {
            Write-Host '[info] -FailFast activado, deteniendo ejecución.'
            break
        }
    }
}

$sw.Stop()

# ---------------------------------------------------------------------------
# Resumen
# ---------------------------------------------------------------------------
Write-Host ''
Write-Host '==================== Summary ===================='
Write-Host ("Total:   {0}" -f $total)
Write-Host ("Passed:  {0}" -f $passed)
Write-Host ("Failed:  {0}" -f $failed)
Write-Host ("Elapsed: {0}s" -f [int]$sw.Elapsed.TotalSeconds)
Write-Host '================================================='

if ($failed -gt 0) { exit 1 } else { exit 0 }