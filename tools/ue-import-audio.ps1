param(
  [switch]$GenerateFirst
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$uproject = Join-Path $projectRoot "engine\LightWarrior.uproject"
$unrealEditor = & (Join-Path $PSScriptRoot "find-unreal.ps1")
$unrealEditorCmd = $unrealEditor -replace "UnrealEditor\.exe$", "UnrealEditor-Cmd.exe"
if (Test-Path $unrealEditorCmd) {
  $unrealEditor = $unrealEditorCmd
}
$importScript = Join-Path $PSScriptRoot "ue-import-audio.py"

if ($GenerateFirst) {
  & (Join-Path $PSScriptRoot "generate-audio-assets.ps1") -UseAudioEngine -Force
}

if (!(Test-Path $uproject)) {
  throw "Missing UE project: $uproject"
}

if (!(Test-Path $importScript)) {
  throw "Missing UE audio import script: $importScript"
}

& $unrealEditor $uproject -ExecutePythonScript="$importScript" -unattended -nopause -nosplash
if ($LASTEXITCODE -ne 0) {
  throw "Unreal audio import failed with exit code $LASTEXITCODE"
}
