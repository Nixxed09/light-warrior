# Audio Engine Game Audio Flow

## Purpose

Light Warrior uses one stable audio contract for gameplay feedback:

```text
gameplay event -> audio package cue -> UE5 imported sound -> UE loop evidence
```

The same pattern should work for future GamesOS games. The game owns event names and timing. `audio-engine` owns generated sound candidates, provenance, and eventual mastering.

## Commands

From the Light Warrior repo:

```powershell
npm run audio:dry-run
npm run audio:generate
npm run audio:generate:engine
npm run audio:import:ue
npm run audio:import:ue:fresh
```

`audio:generate` creates a local deterministic WAV package. It works even when `audio-engine` is offline or provider credentials are missing.

`audio:generate:engine` checks `http://127.0.0.1:8300/api/health`, submits SFX jobs to `POST /api/generate/sfx`, and uses `AUDIO_ENGINE_API_TOKEN` if that environment variable is set. If the service is down, unconfigured, or returns no downloadable asset, the command still writes the local WAV fallback package.

`audio:import:ue` imports the generated WAV package into `/Game/LightWarrior/Audio/CoreSfx`.

`audio:import:ue:fresh` regenerates the package first, then imports it into UE5.

## Output Package

The first package is:

```text
assets/generated/audio/core_sfx/
  dash_shimmer.wav
  light_strike.wav
  shadow_dissolve.wav
  circle_expansion.wav
  temple_activation.wav
  hammer_slam.wav
  request.json
  prompt-package.json
  audio_metadata.json
  review-notes.md
  ue-import-result.json
  playable-evidence.json
```

The metadata maps each file to the UE5 gameplay event vocabulary:

```text
Dash
LightStrike
ShadowDissolve
CircleExpand
TempleActivate
HammerSlam
```

Keep those names stable. New games should define their event vocabulary first, then generate cue packages against that vocabulary.

## Use Cases

- First playable feedback: generate local WAVs immediately so a prototype is not silent.
- Production SFX pass: run `audio-engine` with configured providers and keep the same event names.
- Theme alignment: encode the game theme into `prompt-package.json`, then review each cue against the theme before import.
- Cross-game reuse: copy the script shape into another game, replace the event map, and keep the same output files.
- UE5 import: import WAVs into `/Game/LightWarrior/Audio/CoreSfx`, then map imported `USoundBase` assets to the same events.
- Runtime playback: `ULightWarriorAudio` tries imported `USoundBase` assets first and falls back to procedural tones if a sound asset is missing.

## Current Engine Reality

`audio-engine` is installed at `D:\TE-Code\audio-engine` and exposes:

```text
GET  /api/health
POST /api/generate/sfx
POST /api/generate/music
GET  /api/jobs/{job_id}
GET  /api/assets?job_id={job_id}
```

The game-side flow is ready now. The current SFX route in `audio-engine` may still depend on provider credentials or scaffolded handlers, so Light Warrior keeps local WAV generation as a fallback until production provider output is reliable.

## Graduation Checklist

1. Run `npm run audio:generate` or `npm run audio:generate:engine`.
2. Review `assets/generated/audio/core_sfx/review-notes.md`.
3. Import WAVs into UE5:

   ```powershell
   npm run audio:import:ue
   ```

4. Confirm `assets/generated/audio/core_sfx/ue-import-result.json` reports imported assets.
5. Build UE5 with `.\tools\ue-build.ps1`.
6. Capture audio scenarios:

   ```powershell
   .\tools\ue-loop.ps1 -Name audio-combat -Scenario combat-readability -SkipBuild
   .\tools\ue-loop.ps1 -Name audio-hammer -Scenario thunder-hammer -SkipBuild
   ```

7. Update `ue-import-result.json` and `playable-evidence.json` with the import and capture results.
