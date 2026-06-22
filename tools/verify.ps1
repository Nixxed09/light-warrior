param(
  [switch]$RequireProductionEngine
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot

function Add-Result {
  param(
    [string]$Name,
    [string]$Status,
    [string]$Detail
  )
  [pscustomobject]@{
    name = $Name
    status = $Status
    detail = $Detail
  }
}

$results = New-Object System.Collections.Generic.List[object]

foreach ($file in @(
  "GAME_MANIFEST.json",
  "ASSET_MANIFEST.json",
  "TASK_GRAPH.json",
  "GAME_SPEC.md",
  "GAME_DESIGN.md",
  "REFERENCE_GAMES.md",
  "STYLE_GUIDE.md",
  "DESIGN_REVIEW.md",
  "PLAYTEST_NOTES.md",
  "PROVEN_BETTER_NEW.md",
  "FIRST_FIVE_MINUTES.md",
  "SIGNAL_TEST.md",
  "MINIMUM_VIABLE_IDEA.md",
  "PLAYER_THANK_YOU.md",
  "DISTRIBUTION_STRATEGY.md",
  "REMIX_PRIMITIVES.md",
  "TECHNICAL_PLAN.md"
)) {
  $path = Join-Path $projectRoot $file
  if (Test-Path $path) {
    $results.Add((Add-Result $file "PASS" "Exists."))
  } else {
    $results.Add((Add-Result $file "FAIL" "Missing."))
  }
}

foreach ($file in @("GAME_MANIFEST.json", "ASSET_MANIFEST.json", "TASK_GRAPH.json")) {
  $path = Join-Path $projectRoot $file
  if (Test-Path $path) {
    try {
      Get-Content -LiteralPath $path -Raw | ConvertFrom-Json | Out-Null
      $results.Add((Add-Result "$file JSON" "PASS" "Valid JSON."))
    } catch {
      $results.Add((Add-Result "$file JSON" "FAIL" $_.Exception.Message))
    }
  }
}

$packagePath = Join-Path $projectRoot "package.json"
if (Test-Path $packagePath) {
  $results.Add((Add-Result "package.json" "PASS" "Prototype package exists."))
  Push-Location $projectRoot
  try {
    npm run build
    $results.Add((Add-Result "npm build" "PASS" "Prototype build passed."))
  } catch {
    $results.Add((Add-Result "npm build" "FAIL" $_.Exception.Message))
  } finally {
    Pop-Location
  }
} else {
  $results.Add((Add-Result "package.json" "WARN" "No prototype package.json found."))
}

$unrealCandidates = @(
  "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
  "D:\Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
  "D:\Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe",
  "UnrealEditor"
)

$unrealFound = $false
foreach ($candidate in $unrealCandidates) {
  if (Test-Path $candidate) {
    $results.Add((Add-Result "Unreal Editor" "PASS" "Found $candidate"))
    $unrealFound = $true
    break
  }
  $command = Get-Command $candidate -ErrorAction SilentlyContinue
  if ($command) {
    $results.Add((Add-Result "Unreal Editor" "PASS" "Found $($command.Source)"))
    $unrealFound = $true
    break
  }
}

if (!$unrealFound) {
  $status = if ($RequireProductionEngine) { "FAIL" } else { "WARN" }
  $results.Add((Add-Result "Unreal Editor" $status "UE5 not found. Prototype verification can continue, but production UE5 tasks remain blocked."))
}

$blenderCandidates = @(
  "C:\Program Files\Blender Foundation\Blender 4.5\blender.exe",
  "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe",
  "blender"
)

$blenderFound = $false
foreach ($candidate in $blenderCandidates) {
  if (Test-Path $candidate) {
    $results.Add((Add-Result "Blender" "PASS" "Found $candidate"))
    $blenderFound = $true
    break
  }
  $command = Get-Command $candidate -ErrorAction SilentlyContinue
  if ($command) {
    $results.Add((Add-Result "Blender" "PASS" "Found $($command.Source)"))
    $blenderFound = $true
    break
  }
}

if (!$blenderFound) {
  $results.Add((Add-Result "Blender" "WARN" "Blender not found. Asset blockout work is limited."))
}

$epicLauncher = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe"
if (Test-Path $epicLauncher) {
  $results.Add((Add-Result "Epic Games Launcher" "PASS" "Found $epicLauncher"))
} else {
  $results.Add((Add-Result "Epic Games Launcher" "WARN" "Launcher not found. UE5 installation likely requires manual setup."))
}

$results | Format-Table -AutoSize

$failed = @($results | Where-Object { $_.status -eq "FAIL" })
if ($failed.Count -gt 0) {
  exit 1
}
