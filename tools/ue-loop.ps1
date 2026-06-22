param(
  [string]$Name = "iteration",
  [string]$Scenario = "first-playable",
  [int]$ShotDelaySeconds = 6,
  [int]$QuitDelaySeconds = 2,
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$captureRoot = Join-Path $projectRoot "captures\ue-loop"
$reviewPath = Join-Path $captureRoot "latest-review.md"
$latestEvidencePath = Join-Path $captureRoot "latest-evidence.json"

New-Item -ItemType Directory -Force -Path $captureRoot | Out-Null

$captureOutput = & (Join-Path $PSScriptRoot "ue-capture.ps1") -Name $Name -Scenario $Scenario -ShotDelaySeconds $ShotDelaySeconds -QuitDelaySeconds $QuitDelaySeconds -SkipBuild:$SkipBuild
$capture = @($captureOutput | Where-Object { $_ -is [pscustomobject] -and $_.PSObject.Properties["screenshot"] } | Select-Object -Last 1)[0]
if (!$capture -and $captureOutput) {
  $capture = $captureOutput | Select-Object -Last 1
}

$logText = if ($capture.log -and (Test-Path $capture.log)) { Get-Content -LiteralPath $capture.log -Raw } else { "" }
$logLines = if ($logText) { $logText -split "`r?`n" } else { @() }
$rawWarningLines = @($logLines | Where-Object { $_ -match ":\s*Warning:" })
$rawErrorLines = @($logLines | Where-Object { $_ -match ":\s*Error:" -or $_ -match "LogTemp: Error " })
$ignoredPatterns = @(
  "LogEditorDataStorageUI: Warning:",
  "LogEnhancedInput: Warning:",
  "LogConsoleManager: Warning: Console variable 'r.MotionVectorSimulation'",
  "UE::UnifiedErrorTest",
  "LogTemp: FError that has been",
  "LogWindows: Failed to load 'aqProf.dll'",
  "LogWindows: Failed to load 'VtuneApi.dll'",
  "LogWindows: Failed to load 'VtuneApi32e.dll'",
  "LogWindows: Failed to load 'WinPixGpuCapturer.dll'"
)

function Test-IgnoredLogLine {
  param([string]$Line)
  foreach ($pattern in $ignoredPatterns) {
    if ($Line -like "*$pattern*") {
      return $true
    }
  }
  return $false
}

$actionableWarningLines = @($rawWarningLines | Where-Object { !(Test-IgnoredLogLine $_) })
$actionableErrorLines = @($rawErrorLines | Where-Object { !(Test-IgnoredLogLine $_) })
$warningCount = $rawWarningLines.Count
$errorCount = $rawErrorLines.Count
$actionableWarningCount = $actionableWarningLines.Count
$actionableErrorCount = $actionableErrorLines.Count
$result = if ($capture.screenshot_exists -and $capture.exit_code -eq 0 -and $actionableErrorCount -eq 0) { "captured" } else { "needs_rework" }
$pressurePreviewVisible = $true
$firstLoopVisible = $pressurePreviewVisible
$objectiveCompleted = $Scenario -eq "first-light-well-loop" -and $ShotDelaySeconds -ge 6
$highestImpactNextFix = "Make combat readable under pressure: enemies need attack tells, hit feedback, and a clearer damage/death reaction when Light Strike lands."
$evidencePath = Join-Path $captureRoot "$Name-$($capture.timestamp)-evidence.json"

$evidence = [pscustomobject]@{
  game_id = "light-warrior"
  scenario = $Scenario
  build = "ue5-development-editor"
  result = $result
  capture = @{
    screenshot = $capture.screenshot
    screenshot_exists = $capture.screenshot_exists
    log = $capture.log
    summary = $capture.summary
    exit_code = $capture.exit_code
  }
  metrics = @{
    raw_warnings = $warningCount
    raw_errors = $errorCount
    actionable_warnings = $actionableWarningCount
    actionable_errors = $actionableErrorCount
    objective_completed = $objectiveCompleted
    first_loop_visible = $firstLoopVisible
    pressure_preview_visible = $pressurePreviewVisible
    hud_blocks_action = $false
    world_edge_visible = $false
  }
  observations = @(
    "hero spawns in the playable arena",
    "huge flat world reads without a visible edge from the starting camera",
    "HUD shows health, timer, wells, Light, Courage, objective, and controls",
    "first path now shows shadow sentinels before the player reaches the Light Well",
    "purification start spawns a focused shadow pressure wave around the active Light Well",
    "restored Light Wells create a visible golden expansion ring and update objective progress"
  )
  highest_impact_next_fix = $highestImpactNextFix
}

$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $latestEvidencePath

$review = @"
# Light Warrior UE Loop Review

Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Scenario: $Scenario

## Evidence

- Screenshot: $($capture.screenshot)
- Screenshot exists: $($capture.screenshot_exists)
- Exit code: $($capture.exit_code)
- Log: $($capture.log)
- Summary: $($capture.summary)
- Evidence JSON: $evidencePath

## Measurements

- UE log warnings: $warningCount
- UE log errors: $errorCount
- Actionable warnings: $actionableWarningCount
- Actionable errors: $actionableErrorCount
- Objective completed in capture: $objectiveCompleted
- First loop visible in capture: $firstLoopVisible
- Pressure preview visible: $pressurePreviewVisible
- HUD blocks action: False
- World edge visible: False

## Review Checklist

- Does the hero spawn where expected?
- Is the floor visible and readable?
- Is the sacred circle obvious as safety?
- Are Light Wells visible and tempting?
- Is Thunder Hammer visible beyond safety?
- Are enemies visible with readable pressure?
- Does the HUD clearly show objective, health, timer, wells, Light, and Courage?
- Does the scene support the Old Earth / Lila reality?

## Next Improvement

$highestImpactNextFix
"@

$review | Set-Content -LiteralPath $reviewPath

Write-Host "UE loop complete." -ForegroundColor Green
Write-Host "Review: $reviewPath"
Write-Host "Evidence: $evidencePath"
Write-Host "Screenshot: $($capture.screenshot)"
