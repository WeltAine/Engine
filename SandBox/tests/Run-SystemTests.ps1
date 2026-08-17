param(
    [ValidateSet("Debug", "Release", "Dist")]
    [string]$Configuration = "Debug",
    [int]$TimeoutSeconds = 20,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
$solution = Join-Path $repoRoot "Ayin.slnx"
$outputDir = Join-Path $PSScriptRoot "results"
$stdoutLog = Join-Path $outputDir "system-tests.stdout.log"
$stderrLog = Join-Path $outputDir "system-tests.stderr.log"

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

if (-not $SkipBuild) {
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw "vswhere.exe not found: $vswhere"
    }

    $msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" |
        Select-Object -First 1
    if ([string]::IsNullOrWhiteSpace($msbuild)) {
        throw "MSBuild.exe was not found by vswhere.exe"
    }

    & $msbuild $solution /m /t:SandBox /p:Configuration=$Configuration /p:Platform=x64 /v:minimal
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$executable = Join-Path $repoRoot "bin/$Configuration-windows-x86_64/SandBox/SandBox.exe"
if (-not (Test-Path -LiteralPath $executable)) {
    throw "SandBox executable not found: $executable"
}

[System.IO.File]::WriteAllText($stdoutLog, "")
[System.IO.File]::WriteAllText($stderrLog, "")

$processArguments = @{
    FilePath = $executable
    WorkingDirectory = $repoRoot
    WindowStyle = "Hidden"
    PassThru = $true
    RedirectStandardOutput = $stdoutLog
    RedirectStandardError = $stderrLog
    Environment = @{
        AYIN_SANDBOX_TEST_LAYER = "system"
        AYIN_SANDBOX_TEST_AUTO_EXIT = "1"
    }
}
$process = Start-Process @processArguments

if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $process.Id -Force
    Write-Error "System tests timed out after $TimeoutSeconds seconds. Logs: $outputDir"
    exit 124
}

$stdout = Get-Content -LiteralPath $stdoutLog -Raw
$stderr = Get-Content -LiteralPath $stderrLog -Raw
if (-not [string]::IsNullOrWhiteSpace($stdout)) { Write-Host $stdout.TrimEnd() }
if (-not [string]::IsNullOrWhiteSpace($stderr)) { Write-Error $stderr.TrimEnd() }

if ($process.ExitCode -ne 0) {
    Write-Error "SandBox exited with code $($process.ExitCode). Logs: $outputDir"
    exit $process.ExitCode
}
if ($stdout -notmatch "SYSTEM_WORLD_TEST: PASS") {
    Write-Error "PASS marker was not found. Logs: $outputDir"
    exit 1
}

Write-Host "System regression tests passed. Logs: $outputDir"
