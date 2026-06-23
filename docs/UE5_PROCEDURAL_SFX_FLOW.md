# UE5 Procedural SFX Flow

## Purpose

Light Warrior currently uses procedural UE5 SFX as a playable placeholder layer. This lets the game prove timing, feedback, and event coverage before production audio assets are generated.

This does not replace `audio-engine`. The production path remains:

```text
audio-engine generation -> asset package metadata -> UE5 import/wiring -> UE loop evidence
```

## Current State

Implemented now:

- UE5 procedural SFX helper: `ULightWarriorAudio`
- Runtime generated tones through `USoundWaveProcedural`
- Gameplay event wiring for dash, light strike, shadow dissolve, circle expansion, Light Well completion, Thunder Hammer activation, and hammer slam
- UE loop evidence showing the game runs with the SFX layer active

Not implemented yet:

- Generated `audio-engine` WAV/OGG assets
- `assets/generated/audio/core_sfx/` production package
- UE5 imported production sound assets
- Data-driven replacement of procedural tones with imported `USoundBase` assets

## Files

```text
engine/Source/LightWarrior/Public/LightWarriorAudio.h
engine/Source/LightWarrior/Private/LightWarriorAudio.cpp
engine/Source/LightWarrior/LightWarrior.Build.cs
```

`LightWarrior.Build.cs` includes `AudioMixer` so procedural sounds can run in UE5.

## Event Vocabulary

Keep these names stable. Final audio assets should replace these events, not invent parallel event names.

```text
Dash
LightStrike
ShadowDissolve
CircleExpand
TempleActivate
HammerSlam
```

Current wiring:

- `ALightWarriorCharacter::StartDash` -> `Dash`
- `ALightWarriorCharacter::LightStrike` -> `LightStrike` or `HammerSlam`
- `AShadowEnemy::TakeDamage` on death -> `ShadowDissolve`
- `ASacredCircle::ExpandFromCombat` -> `CircleExpand`
- `ALightWell::CompletePurification` -> `CircleExpand`
- `AThunderHammerTemple::ActivateTemple` -> `TempleActivate`

## Use Cases

Use procedural SFX when:

- A gameplay event is silent and needs immediate feedback.
- The team needs to prove whether sound improves readability.
- `audio-engine` providers are unavailable.
- A new mechanic needs stable event naming before final sound design.
- UE loop evidence needs to prove the playable slice is no longer silent.

Use `audio-engine` when:

- The event vocabulary is stable.
- The sound direction is known.
- The asset needs provenance, metadata, review notes, and import evidence.
- The team is replacing placeholders with production candidates.

## Repeatable Workflow

1. Add or reuse an `ELightWarriorSfx` event.
2. Add/tune the procedural tone in `GetTone(...)`.
3. Wire the event from the gameplay actor that owns the cause.
4. Build UE5:

   ```powershell
   .\tools\ue-build.ps1
   ```

5. Capture the scenario:

   ```powershell
   .\tools\ue-loop.ps1 -Name audio-combat -Scenario combat-readability -SkipBuild
   .\tools\ue-loop.ps1 -Name audio-hammer -Scenario thunder-hammer -SkipBuild
   ```

6. Check `captures\ue-loop\latest-review.md`.
7. Update `TASK_GRAPH.json` and `PRODUCTION_LOG.md`.

## Production Audio Graduation

When `audio-engine` is ready, create:

```text
assets/generated/audio/core_sfx/
  request.json
  prompt-package.json
  audio_metadata.json
  review-notes.md
  ue-import-result.json
  playable-evidence.json
```

Then replace procedural tones by mapping the same event names to imported UE5 sound assets. Keep procedural tones as fallbacks until the production package has capture evidence.

## Acceptance

Procedural SFX pass when:

- UE5 compiles.
- The relevant UE loop scenario captures successfully.
- `latest-review.md` has zero actionable errors.
- The sound helps the player read the action.
- Docs/task state clearly call it procedural, not final production audio.

Production SFX pass only when:

- `audio-engine` provenance exists.
- Sound files import into UE5.
- UE5 gameplay captures prove the sound in context.
- The procedural fallback is intentionally retained or replaced.
