param(
  [string]$AssetId = "audio.core_sfx",
  [string]$AudioEngineUrl = "http://127.0.0.1:8300",
  [string]$Provider = "stable_audio",
  [switch]$UseAudioEngine,
  [switch]$DryRun,
  [switch]$Force,
  [int]$PollSeconds = 45
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$GeneratedRoot = Join-Path $ProjectRoot "assets\generated\audio"
$PackageName = $AssetId -replace "^audio\.", ""
$PackageRoot = Join-Path $GeneratedRoot $PackageName
$CreatedAt = (Get-Date).ToUniversalTime().ToString("o")

$EventMap = @(
  @{
    id = "dash_shimmer"
    event = "Dash"
    duration_sec = 0.46
    frequency_a = 880.0
    frequency_b = 1760.0
    noise = 0.04
    envelope = "fast shimmer rise, short luminous tail"
    prompt = "Light Warrior dash shimmer SFX, quick holy air rush, bright crystal particles, agile heroic movement, no melody, game-ready one shot."
  },
  @{
    id = "light_strike"
    event = "LightStrike"
    duration_sec = 0.54
    frequency_a = 196.0
    frequency_b = 1480.0
    noise = 0.11
    envelope = "sharp impact, radiant spark decay"
    prompt = "Light Warrior sword impact SFX, sacred light strike, metallic hit plus radiant spark burst, readable combat feedback, no voice."
  },
  @{
    id = "shadow_dissolve"
    event = "ShadowDissolve"
    duration_sec = 0.82
    frequency_a = 120.0
    frequency_b = 520.0
    noise = 0.22
    envelope = "dark burst into upward purification fade"
    prompt = "Enemy shadow dissolve SFX, corrupted smoke collapses then burns away into warm light, satisfying defeat cue, no gore, no voice."
  },
  @{
    id = "circle_expansion"
    event = "CircleExpand"
    duration_sec = 1.05
    frequency_a = 164.81
    frequency_b = 987.77
    noise = 0.03
    envelope = "soft pulse blooms into temple resonance"
    prompt = "Sacred circle expansion SFX, restorative holy pulse, warm choir-like resonance without vocals, magical boundary growing outward."
  },
  @{
    id = "temple_activation"
    event = "TempleActivate"
    duration_sec = 1.18
    frequency_a = 220.0
    frequency_b = 1320.0
    noise = 0.05
    envelope = "ancient chime stack, energy core lights up"
    prompt = "Thunder Hammer temple activation SFX, ancient stone machinery wakes, sacred chimes, blue lightning core charges, heroic reveal."
  },
  @{
    id = "hammer_slam"
    event = "HammerSlam"
    duration_sec = 0.92
    frequency_a = 72.0
    frequency_b = 720.0
    noise = 0.26
    envelope = "heavy transient, thunder crack, low heroic tail"
    prompt = "Thunder Hammer slam SFX, heavy mythic impact, thunder crack, sacred shockwave, powerful but clean gameplay one shot."
  }
)

function Write-AsciiBytes {
  param(
    [System.IO.BinaryWriter]$Writer,
    [string]$Text
  )

  $Writer.Write([System.Text.Encoding]::ASCII.GetBytes($Text))
}

function Write-GameWav {
  param(
    [string]$Path,
    [double]$DurationSec,
    [double]$FrequencyA,
    [double]$FrequencyB,
    [double]$NoiseAmount,
    [int]$Seed
  )

  $SampleRate = 44100
  $Channels = 1
  $BitsPerSample = 16
  $BytesPerSample = [int]($BitsPerSample / 8)
  $SampleCount = [int]($DurationSec * $SampleRate)
  $DataBytes = $SampleCount * $Channels * $BytesPerSample
  $Directory = Split-Path -Parent $Path

  New-Item -ItemType Directory -Force -Path $Directory | Out-Null

  $Stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write)
  $Writer = [System.IO.BinaryWriter]::new($Stream)
  $Random = [System.Random]::new($Seed)

  try {
    Write-AsciiBytes $Writer "RIFF"
    $Writer.Write([int](36 + $DataBytes))
    Write-AsciiBytes $Writer "WAVE"
    Write-AsciiBytes $Writer "fmt "
    $Writer.Write([int]16)
    $Writer.Write([int16]1)
    $Writer.Write([int16]$Channels)
    $Writer.Write([int]$SampleRate)
    $Writer.Write([int]($SampleRate * $Channels * $BytesPerSample))
    $Writer.Write([int16]($Channels * $BytesPerSample))
    $Writer.Write([int16]$BitsPerSample)
    Write-AsciiBytes $Writer "data"
    $Writer.Write([int]$DataBytes)

    for ($Index = 0; $Index -lt $SampleCount; $Index++) {
      $T = $Index / $SampleRate
      $Progress = $Index / [Math]::Max(1, $SampleCount - 1)
      $Attack = [Math]::Min(1.0, $Progress / 0.08)
      $Decay = [Math]::Pow(1.0 - $Progress, 1.8)
      $Envelope = $Attack * $Decay
      $Sweep = $FrequencyA + (($FrequencyB - $FrequencyA) * [Math]::Pow($Progress, 0.7))
      $Tone = [Math]::Sin(2.0 * [Math]::PI * $Sweep * $T)
      $Overtone = 0.32 * [Math]::Sin(2.0 * [Math]::PI * ($Sweep * 1.5) * $T)
      $Sub = 0.18 * [Math]::Sin(2.0 * [Math]::PI * [Math]::Max(38.0, $FrequencyA * 0.5) * $T)
      $Noise = (($Random.NextDouble() * 2.0) - 1.0) * $NoiseAmount
      $Sample = ($Tone + $Overtone + $Sub + $Noise) * $Envelope * 0.58
      $Clamped = [Math]::Max(-1.0, [Math]::Min(1.0, $Sample))
      $Writer.Write([int16]($Clamped * 32767))
    }
  }
  finally {
    $Writer.Dispose()
    $Stream.Dispose()
  }
}

function Get-AudioEngineHeaders {
  $Headers = @{}
  if ($env:AUDIO_ENGINE_API_TOKEN) {
    $Headers["Authorization"] = "Bearer $($env:AUDIO_ENGINE_API_TOKEN)"
  }
  return $Headers
}

function Test-AudioEngine {
  param([string]$BaseUrl)

  try {
    return Invoke-RestMethod -Uri "$BaseUrl/api/health" -TimeoutSec 5
  }
  catch {
    return $null
  }
}

function Invoke-AudioEngineSfx {
  param(
    [hashtable]$Cue,
    [string]$BaseUrl,
    [string]$ProviderName,
    [int]$TimeoutSeconds
  )

  $Headers = Get-AudioEngineHeaders
  $Body = @{
    provider = $ProviderName
    prompt = $Cue.prompt
    duration_sec = $Cue.duration_sec
    client_request_id = "light-warrior-$($Cue.id)-$([Guid]::NewGuid().ToString("n"))"
  } | ConvertTo-Json -Depth 6

  $Job = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/generate/sfx" -Headers $Headers -ContentType "application/json" -Body $Body -TimeoutSec 15
  $JobId = $Job.job_id
  if (-not $JobId) {
    throw "audio-engine did not return a job_id for $($Cue.id)"
  }

  $Deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  do {
    Start-Sleep -Seconds 2
    $Status = Invoke-RestMethod -Uri "$BaseUrl/api/jobs/$JobId" -Headers $Headers -TimeoutSec 10
    if ($Status.status -in @("completed", "succeeded", "success")) {
      $Assets = Invoke-RestMethod -Uri "$BaseUrl/api/assets?job_id=$JobId" -Headers $Headers -TimeoutSec 10
      return @{
        job_id = $JobId
        status = $Status.status
        assets = $Assets
      }
    }
    if ($Status.status -in @("failed", "error", "cancelled")) {
      throw "audio-engine job $JobId ended with status $($Status.status)"
    }
  } while ((Get-Date) -lt $Deadline)

  throw "audio-engine job $JobId did not complete within $TimeoutSeconds seconds"
}

if ($AssetId -ne "audio.core_sfx") {
  throw "Only audio.core_sfx is wired right now. Received: $AssetId"
}

$Request = @{
  asset_id = $AssetId
  created_at = $CreatedAt
  requested_by = "Light Warrior GamesOS audio asset flow"
  audio_engine_url = $AudioEngineUrl
  provider = $Provider
  use_audio_engine = [bool]$UseAudioEngine
  fallback = "local deterministic WAV synthesis when audio-engine is offline, unconfigured, or returns no downloadable asset"
  event_vocabulary = $EventMap | ForEach-Object { $_.event }
}

$PromptPackage = @{
  asset_id = $AssetId
  theme = "Light Warrior: brave warmth and sacred restoration pushing back readable shadow corruption"
  constraints = @(
    "One-shot WAV cues for UE5 gameplay events.",
    "No vocals or spoken words.",
    "Readable transient first, stylized heroic fantasy color second.",
    "Keep event names stable so procedural UE5 fallbacks and production assets share one vocabulary."
  )
  sfx = $EventMap | ForEach-Object {
    @{
      id = $_.id
      event = $_.event
      duration_sec = $_.duration_sec
      envelope = $_.envelope
      prompt = $_.prompt
    }
  }
}

if ($DryRun) {
  $Request | ConvertTo-Json -Depth 8
  $PromptPackage | ConvertTo-Json -Depth 8
  Write-Host "Dry run only. No files generated."
  exit 0
}

if ((Test-Path $PackageRoot) -and -not $Force) {
  $ExistingFiles = Get-ChildItem -Path $PackageRoot -Filter *.wav -ErrorAction SilentlyContinue
  if ($ExistingFiles.Count -gt 0) {
    throw "Audio package already has WAV files at $PackageRoot. Re-run with -Force to regenerate."
  }
}

New-Item -ItemType Directory -Force -Path $PackageRoot | Out-Null

$Health = Test-AudioEngine $AudioEngineUrl
$Files = @()
$Notes = New-Object System.Collections.Generic.List[string]

if ($Health) {
  $Notes.Add("audio-engine health check succeeded at $AudioEngineUrl.")
}
else {
  $Notes.Add("audio-engine health check failed at $AudioEngineUrl; generated local deterministic WAV fallbacks.")
}

foreach ($Cue in $EventMap) {
  $OutputPath = Join-Path $PackageRoot "$($Cue.id).wav"
  $Source = "local_fallback"
  $EngineResult = $null

  if ($UseAudioEngine -and $Health) {
    try {
      $EngineResult = Invoke-AudioEngineSfx -Cue $Cue -BaseUrl $AudioEngineUrl -ProviderName $Provider -TimeoutSeconds $PollSeconds
      $Source = "local_fallback_after_audio_engine_job"
      $Notes.Add("audio-engine accepted $($Cue.id) as job $($EngineResult.job_id); generated local fallback WAV because no provider-backed downloadable SFX file is imported yet.")
    }
    catch {
      $Notes.Add("audio-engine generation failed for $($Cue.id): $($_.Exception.Message). Generated fallback WAV.")
    }
  }

  Write-GameWav -Path $OutputPath -DurationSec $Cue.duration_sec -FrequencyA $Cue.frequency_a -FrequencyB $Cue.frequency_b -NoiseAmount $Cue.noise -Seed ([Math]::Abs($Cue.id.GetHashCode()))

  $AudioEngineJobId = $null
  if ($EngineResult) {
    $AudioEngineJobId = $EngineResult.job_id
  }

  $Files += @{
    id = $Cue.id
    event = $Cue.event
    file = "assets/generated/audio/$PackageName/$($Cue.id).wav"
    duration_sec = $Cue.duration_sec
    format = "wav"
    sample_rate = 44100
    channels = 1
    source = $Source
    audio_engine_job_id = $AudioEngineJobId
    prompt = $Cue.prompt
  }
}

$ProviderSystem = "local-deterministic-wav-fallback"
if ($UseAudioEngine -and $Health) {
  $ProviderSystem = "audio-engine-with-local-wav-fallback"
}

$Metadata = @{
  asset_id = $AssetId
  status = "generated"
  created_at = $CreatedAt
  provider_system = $ProviderSystem
  audio_engine = @{
    url = $AudioEngineUrl
    health_available = [bool]$Health
    provider = $Provider
    token_env = "AUDIO_ENGINE_API_TOKEN"
  }
  ue5_event_map = @{}
  files = $Files
  notes = $Notes
}

foreach ($File in $Files) {
  $Metadata.ue5_event_map[$File.event] = $File.file
}

$ImportResult = @{
  asset_id = $AssetId
  status = "pending_ue5_import"
  created_at = $CreatedAt
  target = "/Game/LightWarrior/Audio/CoreSfx"
  note = "Import these WAV files into UE5, map them to the same event names, and keep ULightWarriorAudio procedural tones as fallback until capture evidence passes."
}

$PlayableEvidence = @{
  asset_id = $AssetId
  status = "pending_capture"
  required_commands = @(
    ".\tools\ue-build.ps1",
    ".\tools\ue-loop.ps1 -Name audio-combat -Scenario combat-readability -SkipBuild",
    ".\tools\ue-loop.ps1 -Name audio-hammer -Scenario thunder-hammer -SkipBuild"
  )
}

$ReviewEventLines = ($Files | ForEach-Object { "- $($_.event): ``$($_.file)``" }) -join [Environment]::NewLine

$ReviewNotes = @"
# audio.core_sfx Review Notes

Generated: $CreatedAt

This package wires the Light Warrior gameplay event vocabulary to actual WAV files under ``assets/generated/audio/core_sfx``.

Current source: $($Metadata.provider_system)

Events:

$ReviewEventLines

Production gate:

- Import the WAVs into ``/Game/LightWarrior/Audio/CoreSfx``.
- Replace procedural tones only after UE5 capture evidence proves the generated cues work in context.
- Keep the event names stable: Dash, LightStrike, ShadowDissolve, CircleExpand, TempleActivate, HammerSlam.
"@

$Request | ConvertTo-Json -Depth 8 | Set-Content -Path (Join-Path $PackageRoot "request.json") -Encoding UTF8
$PromptPackage | ConvertTo-Json -Depth 8 | Set-Content -Path (Join-Path $PackageRoot "prompt-package.json") -Encoding UTF8
$Metadata | ConvertTo-Json -Depth 10 | Set-Content -Path (Join-Path $PackageRoot "audio_metadata.json") -Encoding UTF8
$ImportResult | ConvertTo-Json -Depth 8 | Set-Content -Path (Join-Path $PackageRoot "ue-import-result.json") -Encoding UTF8
$PlayableEvidence | ConvertTo-Json -Depth 8 | Set-Content -Path (Join-Path $PackageRoot "playable-evidence.json") -Encoding UTF8
$ReviewNotes | Set-Content -Path (Join-Path $PackageRoot "review-notes.md") -Encoding UTF8

Write-Host "Generated $AssetId at $PackageRoot"
Write-Host "WAV files:"
$Files | ForEach-Object { Write-Host " - $($_.event): $($_.file)" }
