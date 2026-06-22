param(
  [string]$Configuration = "Development",
  [int]$MaxParallelActions = 4
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$uproject = Join-Path $projectRoot "engine\LightWarrior.uproject"
$unrealEditor = & (Join-Path $PSScriptRoot "find-unreal.ps1")
$engineRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $unrealEditor))
$buildBat = Join-Path $engineRoot "Build\BatchFiles\Build.bat"

if (!(Test-Path $buildBat)) {
  throw "Unreal Build.bat not found at $buildBat"
}

Push-Location $projectRoot
try {
  & $buildBat LightWarriorEditor Win64 $Configuration -Project="$uproject" -WaitMutex "-MaxParallelActions=$MaxParallelActions"
  if ($LASTEXITCODE -ne 0) {
    $ubtLog = Join-Path $env:LOCALAPPDATA "UnrealBuildTool\Log.txt"
    $logTail = if (Test-Path $ubtLog) { (Get-Content -LiteralPath $ubtLog -Tail 40) -join "`n" } else { "" }
    if ($logTail -notmatch "Result:\s+Succeeded") {
      throw "UnrealBuildTool failed with exit code $LASTEXITCODE"
    }
    Write-Warning "Build.bat returned exit code $LASTEXITCODE, but UnrealBuildTool log reports success."
  }
} finally {
  Pop-Location
}
