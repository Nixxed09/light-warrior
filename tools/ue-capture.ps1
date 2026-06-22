param(
  [string]$Name = "ue-loop",
  [int]$ShotDelaySeconds = 6,
  [int]$QuitDelaySeconds = 2,
  [int]$ResX = 1280,
  [int]$ResY = 720,
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$uproject = Join-Path $projectRoot "engine\LightWarrior.uproject"
$captureRoot = Join-Path $projectRoot "captures\ue-loop"
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$shotPath = Join-Path $captureRoot "$Name-$timestamp.png"
$logPath = Join-Path $captureRoot "$Name-$timestamp.log"
$summaryPath = Join-Path $captureRoot "$Name-$timestamp.json"

New-Item -ItemType Directory -Force -Path $captureRoot | Out-Null

if (!$SkipBuild) {
  & (Join-Path $PSScriptRoot "ue-build.ps1")
}

$unrealEditor = & (Join-Path $PSScriptRoot "find-unreal.ps1")

$arguments = @(
  "`"$uproject`"",
  "-game",
  "-windowed",
  "-ResX=$ResX",
  "-ResY=$ResY",
  "-log",
  "-abslog=`"$logPath`"",
  "-NoSplash",
  "-LWAutoCapture",
  "-LWShot=`"$shotPath`"",
  "-LWShotDelay=$ShotDelaySeconds",
  "-LWQuitDelay=$QuitDelaySeconds"
)

$process = Start-Process -FilePath $unrealEditor -ArgumentList $arguments -Wait -PassThru -WindowStyle Hidden

$summary = [pscustomobject]@{
  name = $Name
  timestamp = $timestamp
  exit_code = $process.ExitCode
  screenshot = $shotPath
  screenshot_exists = Test-Path $shotPath
  log = $logPath
  summary = $summaryPath
}

$summary | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $summaryPath

if (!(Test-Path $shotPath)) {
  Write-Warning "Screenshot was not found at $shotPath. Check the Unreal log window/output."
}

$summary
