param(
  [string]$Name = "ue-loop",
  [int]$ShotDelaySeconds = 6,
  [int]$QuitDelaySeconds = 2,
  [int]$ResX = 1280,
  [int]$ResY = 720,
  [string]$Scenario = "first-playable",
  [switch]$RecordInput,
  [switch]$Visible,
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
  "-LWQuitDelay=$QuitDelaySeconds",
  "-LWScenario=$Scenario"
)

if ($RecordInput) {
  $arguments += "-LWRecordInput"
}

$windowStyle = if ($Visible) { "Normal" } else { "Hidden" }
$process = Start-Process -FilePath $unrealEditor -ArgumentList $arguments -Wait -PassThru -WindowStyle $windowStyle

$inputTranscriptPath = Join-Path $captureRoot "$Name-$timestamp-input-transcript.json"
$inputTranscriptExists = $false
$inputTranscriptActionCount = 0
if ($RecordInput -and (Test-Path -LiteralPath $logPath)) {
  $inputTranscript = & (Join-Path $PSScriptRoot "extract-input-transcript.ps1") -LogPath $logPath -OutPath $inputTranscriptPath
  $inputTranscriptExists = [bool]$inputTranscript.transcript_exists
  $inputTranscriptActionCount = [int]$inputTranscript.action_count
}

$summary = [pscustomobject]@{
  name = $Name
  timestamp = $timestamp
  scenario = $Scenario
  record_input = [bool]$RecordInput
  exit_code = $process.ExitCode
  screenshot = $shotPath
  screenshot_exists = Test-Path $shotPath
  log = $logPath
  input_transcript = if ($inputTranscriptExists) { $inputTranscriptPath } else { $null }
  input_transcript_actions = $inputTranscriptActionCount
  summary = $summaryPath
}

$summary | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $summaryPath

if (!(Test-Path $shotPath)) {
  Write-Warning "Screenshot was not found at $shotPath. Check the Unreal log window/output."
}

$summary
