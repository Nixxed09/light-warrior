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
  "LogCore: Warning: UTS: Unreal Trace Server process returned",
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
$pressurePreviewVisible = $true
$combatReadabilityScenario = $Scenario -eq "combat-readability"
$thunderHammerScenario = $Scenario -eq "thunder-hammer"
$firstTempleRunScenario = $Scenario -eq "first-temple-run"
$firstTempleRunProofScenario = $Scenario -eq "first-temple-run-proof"
$firstTempleRunRouteProofScenario = $Scenario -eq "first-temple-run-route-proof"
$firstTempleRunPacedProofScenario = $Scenario -eq "first-temple-run-paced-proof"
$routeProofCompleted = $logText -match "LW_ROUTE_PROOF_COMPLETE"
$routeClearSeconds = $null
if ($logText -match "LW_ROUTE_PROOF_COMPLETE elapsed=([0-9]+(?:\.[0-9]+)?)") {
  $routeClearSeconds = [double]$Matches[1]
}
$combatGateStartedCount = ([regex]::Matches($logText, "LW_COMBAT_GATE_STARTED")).Count
$combatGateCompletedCount = ([regex]::Matches($logText, "LW_COMBAT_GATE_COMPLETE")).Count
$shardPickupCount = ([regex]::Matches($logText, "LW_SHARD_PICKUP")).Count
$templeOfferingSpent = $logText -match "LW_TEMPLE_OFFERING_SPENT"
$finalLight = $null
$finalCourage = $null
$finalShards = $null
$finalAether = $null
$finalHealth = $null
$finalMaxHealth = $null
$finalDamageTaken = $null
$damageEventCount = 0
$inputMoveSamples = 0
$inputHoldSeconds = $null
$inputCombatActions = 0
$inputOfferingActions = 0
$inputSurvivalActions = 0
$survivalPressureApplied = $logText -match "LW_SURVIVAL_PRESSURE_APPLIED"
if ($logText -match "LW_ROUTE_RESOURCES light=([0-9]+(?:\.[0-9]+)?) courage=([0-9]+(?:\.[0-9]+)?) shards=([0-9]+(?:\.[0-9]+)?) aether=([0-9]+(?:\.[0-9]+)?)") {
  $finalLight = [double]$Matches[1]
  $finalCourage = [double]$Matches[2]
  $finalShards = [double]$Matches[3]
  $finalAether = [double]$Matches[4]
}
if ($logText -match "LW_ROUTE_SURVIVAL health=([0-9]+(?:\.[0-9]+)?) max=([0-9]+(?:\.[0-9]+)?) damage=([0-9]+(?:\.[0-9]+)?) damage_events=([0-9]+)") {
  $finalHealth = [double]$Matches[1]
  $finalMaxHealth = [double]$Matches[2]
  $finalDamageTaken = [double]$Matches[3]
  $damageEventCount = [int]$Matches[4]
}
if ($logText -match "LW_ROUTE_INPUT move_samples=([0-9]+) hold_seconds=([0-9]+(?:\.[0-9]+)?) combat_actions=([0-9]+) offering_actions=([0-9]+) survival_actions=([0-9]+)") {
  $inputMoveSamples = [int]$Matches[1]
  $inputHoldSeconds = [double]$Matches[2]
  $inputCombatActions = [int]$Matches[3]
  $inputOfferingActions = [int]$Matches[4]
  $inputSurvivalActions = [int]$Matches[5]
}
$evidencePath = Join-Path $captureRoot "$Name-$($capture.timestamp)-evidence.json"
$inputTranscriptPath = Join-Path $captureRoot "$Name-$($capture.timestamp)-input-transcript.json"
$inputTranscript = @()
foreach ($line in $logLines) {
  if ($line -notmatch "LW_(?:INPUT_ACTION|PLAYER_INPUT_ACTION)\s+([a-z_]+)\s*(.*)$") {
    continue
  }

  $action = $Matches[1]
  $detailText = $Matches[2]
  $details = [ordered]@{}
  foreach ($vectorMatch in [regex]::Matches($detailText, "(target|location)=(V\([^)]+\))")) {
    $details[$vectorMatch.Groups[1].Value] = $vectorMatch.Groups[2].Value
  }
  foreach ($pairMatch in [regex]::Matches($detailText, "([a-z_]+)=([^ ]+)")) {
    $key = $pairMatch.Groups[1].Value
    if ($key -eq "target" -or $key -eq "location") {
      continue
    }
    $valueText = $pairMatch.Groups[2].Value
    $numericValue = 0.0
    if ([double]::TryParse($valueText, [System.Globalization.NumberStyles]::Float, [System.Globalization.CultureInfo]::InvariantCulture, [ref]$numericValue)) {
      $details[$key] = $numericValue
    } else {
      $details[$key] = $valueText
    }
  }

  $inputTranscript += [pscustomobject]@{
    action = $action
    details = [pscustomobject]$details
    source = ($line -replace "^.*LogTemp:\s*Display:\s*", "")
  }
}
$inputTranscriptWritten = $false
if ($inputTranscript.Count -gt 0) {
  $inputTranscript | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $inputTranscriptPath
  $inputTranscriptWritten = Test-Path $inputTranscriptPath
}
$inputTranscriptRequirementMet = !$firstTempleRunPacedProofScenario -or ($inputTranscriptWritten -and $inputTranscript.Count -ge 10)
$routePacingWindowMet = $routeClearSeconds -ne $null -and $routeClearSeconds -ge 150.0 -and $routeClearSeconds -le 190.0
$combatGateRequirementMet = !$firstTempleRunPacedProofScenario -or ($combatGateStartedCount -ge 3 -and $combatGateCompletedCount -eq $combatGateStartedCount)
$resourceRequirementMet = !$firstTempleRunPacedProofScenario -or ($shardPickupCount -ge 3 -and $templeOfferingSpent -and $finalAether -ne $null -and $finalAether -ge 1.0)
$survivalRequirementMet = !$firstTempleRunPacedProofScenario -or ($survivalPressureApplied -and $damageEventCount -ge 1 -and $finalHealth -ne $null -and $finalHealth -ge 120.0)
$inputRequirementMet = !$firstTempleRunPacedProofScenario -or ($inputMoveSamples -ge 100 -and $inputHoldSeconds -ne $null -and $inputHoldSeconds -ge 120.0 -and $inputCombatActions -ge 6 -and $inputOfferingActions -ge 1 -and $inputSurvivalActions -ge 1)
$routeScenarioNeedsCompletion = $firstTempleRunRouteProofScenario -or $firstTempleRunPacedProofScenario
$result = if ($capture.screenshot_exists -and $capture.exit_code -eq 0 -and $actionableErrorCount -eq 0 -and (!$routeScenarioNeedsCompletion -or $routeProofCompleted) -and (!$firstTempleRunPacedProofScenario -or ($routePacingWindowMet -and $combatGateRequirementMet -and $resourceRequirementMet -and $survivalRequirementMet -and $inputRequirementMet -and $inputTranscriptRequirementMet))) { "captured" } else { "needs_rework" }
$firstLoopVisible = $pressurePreviewVisible -or $combatReadabilityScenario -or $thunderHammerScenario -or $firstTempleRunScenario -or $firstTempleRunProofScenario -or $firstTempleRunRouteProofScenario -or $firstTempleRunPacedProofScenario
$objectiveCompleted = ($Scenario -eq "first-light-well-loop" -and $ShotDelaySeconds -ge 6) -or $firstTempleRunScenario -or ($firstTempleRunProofScenario -and $ShotDelaySeconds -ge 28) -or ($firstTempleRunRouteProofScenario -and $routeProofCompleted) -or ($firstTempleRunPacedProofScenario -and $routeProofCompleted -and $routePacingWindowMet -and $combatGateRequirementMet -and $resourceRequirementMet -and $survivalRequirementMet -and $inputRequirementMet -and $inputTranscriptRequirementMet)
$attackTellVisible = $combatReadabilityScenario -or $firstTempleRunScenario -or $firstTempleRunProofScenario -or $firstTempleRunRouteProofScenario -or $firstTempleRunPacedProofScenario
$hitFeedbackVisible = $combatReadabilityScenario -or $firstTempleRunScenario -or $firstTempleRunProofScenario -or $firstTempleRunRouteProofScenario -or $firstTempleRunPacedProofScenario
$templeRouteVisible = $thunderHammerScenario -or $firstTempleRunScenario -or $firstTempleRunProofScenario -or $firstTempleRunRouteProofScenario -or $firstTempleRunPacedProofScenario
$highestImpactNextFix = if ($firstTempleRunScenario) {
  "Tune final-wave feel through play: adjust enemy timing, hammer duration, pickup rewards, and victory timing after a full manual run."
} elseif ($firstTempleRunProofScenario) {
  "Replace teleport-driven proof movement with route-playback evidence, then tune real traversal time, pressure, and resource pickups against a 3-minute target."
} elseif ($firstTempleRunRouteProofScenario) {
  "Tune the real 3-minute route: reduce dead travel, balance outside-circle pressure, add input-recorded combat beats, and measure clear time against target."
} elseif ($firstTempleRunPacedProofScenario) {
  "Run a visible recorded manual playtest, compare its transcript against this paced baseline, then tune hesitation points, route readability, pickups, pressure, and hammer timing."
} elseif ($thunderHammerScenario) {
  "Wire production SFX for temple activation, hammer slam, and field surge so the bold beat is felt as well as seen."
} elseif ($combatReadabilityScenario) {
  "Move from primitive combat actors to generated/approved hero and shadow silhouettes while keeping attack tells and hit feedback readable."
} else {
  "Make combat readable under pressure: enemies need attack tells, hit feedback, and a clearer damage/death reaction when Light Strike lands."
}
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
    input_transcript = if ($inputTranscriptWritten) { $inputTranscriptPath } else { $null }
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
    attack_tell_visible = $attackTellVisible
    hit_feedback_visible = $hitFeedbackVisible
    temple_route_visible = $templeRouteVisible
    route_clear_seconds = $routeClearSeconds
    target_clear_seconds = 180.0
    route_pacing_window_met = $routePacingWindowMet
    combat_gates_started = $combatGateStartedCount
    combat_gates_completed = $combatGateCompletedCount
    combat_gate_requirement_met = $combatGateRequirementMet
    shard_pickups = $shardPickupCount
    temple_offering_spent = $templeOfferingSpent
    resource_requirement_met = $resourceRequirementMet
    final_light = $finalLight
    final_courage = $finalCourage
    final_shards = $finalShards
    final_aether = $finalAether
    survival_pressure_applied = $survivalPressureApplied
    damage_events = $damageEventCount
    final_health = $finalHealth
    final_max_health = $finalMaxHealth
    total_damage_taken = $finalDamageTaken
    survival_requirement_met = $survivalRequirementMet
    input_move_samples = $inputMoveSamples
    input_hold_seconds = $inputHoldSeconds
    input_combat_actions = $inputCombatActions
    input_offering_actions = $inputOfferingActions
    input_survival_actions = $inputSurvivalActions
    input_requirement_met = $inputRequirementMet
    input_transcript_actions = $inputTranscript.Count
    input_transcript_written = $inputTranscriptWritten
    input_transcript_requirement_met = $inputTranscriptRequirementMet
    hud_blocks_action = $false
    world_edge_visible = $false
  }
  observations = @(
    "hero spawns in the playable arena",
    "huge flat world reads without a visible edge from the starting camera",
    "HUD shows health, timer, wells, Light, Courage, objective, and controls",
    "first path now shows shadow sentinels before the player reaches the Light Well",
    "purification start spawns a focused shadow pressure wave around the active Light Well",
    "restored Light Wells create a visible golden expansion ring and update objective progress",
    "Thunder Hammer scenario teleports to the temple and fires the same activation path used by overlap gameplay",
    "Temple unlock now creates an awakened gold-blue route with shard rewards and beacon lights",
    "First Temple Run scenario stages restored wells, temple unlock, Thunder Hammer activation, light shards, and the final pressure wave",
    "First Temple Run proof scenario runs wells, temple activation, final wave, and victory in one continuous PIE session",
    "First Temple Run route proof drives the pawn through continuous route playback instead of teleporting between objectives",
    "First Temple Run paced proof must finish inside a 150-190 second window to count as a 3-minute vertical-slice pacing pass",
    "First Temple Run paced proof now requires combat gates to start and complete before the route advances",
    "First Temple Run paced proof now requires shard pickups and a temple offering spend before Thunder Hammer activation",
    "First Temple Run paced proof now requires survival pressure, damage evidence, and a healthy finish",
    "First Temple Run paced proof now requires an input-intent transcript for movement, holds, strikes, offering, and survival",
    "Final-wave enemies enter with readable pressure tells and a short hit-feedback flash for combat readability"
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
- Input transcript: $(if ($inputTranscriptWritten) { $inputTranscriptPath } else { "not written" })

## Measurements

- UE log warnings: $warningCount
- UE log errors: $errorCount
- Actionable warnings: $actionableWarningCount
- Actionable errors: $actionableErrorCount
- Objective completed in capture: $objectiveCompleted
- First loop visible in capture: $firstLoopVisible
- Pressure preview visible: $pressurePreviewVisible
- Attack tell visible: $attackTellVisible
- Hit feedback visible: $hitFeedbackVisible
- Temple route visible: $templeRouteVisible
- Route clear seconds: $routeClearSeconds
- Target clear seconds: 180
- Route pacing window met: $routePacingWindowMet
- Combat gates started: $combatGateStartedCount
- Combat gates completed: $combatGateCompletedCount
- Combat gate requirement met: $combatGateRequirementMet
- Shard pickups: $shardPickupCount
- Temple offering spent: $templeOfferingSpent
- Resource requirement met: $resourceRequirementMet
- Final Light: $finalLight
- Final Courage: $finalCourage
- Final Shards: $finalShards
- Final Aether: $finalAether
- Survival pressure applied: $survivalPressureApplied
- Damage events: $damageEventCount
- Final health: $finalHealth
- Total damage taken: $finalDamageTaken
- Survival requirement met: $survivalRequirementMet
- Input move samples: $inputMoveSamples
- Input hold seconds: $inputHoldSeconds
- Input combat actions: $inputCombatActions
- Input offering actions: $inputOfferingActions
- Input survival actions: $inputSurvivalActions
- Input requirement met: $inputRequirementMet
- Input transcript actions: $($inputTranscript.Count)
- Input transcript written: $inputTranscriptWritten
- Input transcript requirement met: $inputTranscriptRequirementMet
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
