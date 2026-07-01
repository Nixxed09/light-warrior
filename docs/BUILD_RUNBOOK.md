# Light Warrior Build Runbook

## Start Every Session

```powershell
.\tools\status.ps1
.\tools\verify.ps1
```

Then read the next pending task in `TASK_GRAPH.json`.

## Current Best Next Task

`engine.playable_slice`

Reason: production now has a compiling UE5 module and code-spawned first arena. The next milestone is Play-In-Editor capture evidence, Blueprint polish, authored UI, and a saved arena map.

## Prototype Evidence Task

1. Run the dev server:

   ```powershell
   npm run dev
   ```

   Current verified local URL:

   ```text
   http://127.0.0.1:3001/
   ```

2. Play the first 10 seconds:
   - start in sacred circle
   - dash/strike first demon
   - show circle expansion
   - reach or preview Thunder Hammer temple

3. Capture screenshot/video to `captures/`.

   Smoke screenshot:

   ```powershell
   npm run capture:smoke
   ```

4. Update:
   - `PLAYTEST_NOTES.md`
   - `DESIGN_REVIEW.md`
   - `TASK_GRAPH.json`
   - `PRODUCTION_LOG.md`

## UE5 Migration Task

Current scaffold exists:

- `engine/LightWarrior.uproject`
- `engine/Config/*.ini`
- `engine/Source/LightWarrior`
- first C++ classes for character, sacred circle, Light Wells, Thunder Hammer temple, shadow enemy, and game mode

Read `docs\ENGINE_TRANSITION_THREEJS_TO_UE5.md` before treating browser prototype work as production progress.

## Run UE5 Playable Slice

Compile:

```powershell
.\tools\ue-build.ps1
```

Open:

```powershell
& "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject"
```

Press Play. The game mode currently auto-spawns a playable arena if the loaded map has no sacred circle:

- player pawn near the starting circle
- visible arena floor and directional light
- sacred circle safe zone
- three Light Wells
- Thunder Hammer temple
- eight shadow enemies

Controls:

- `WASD`: move
- mouse/gamepad right stick: look
- `Left Shift` / gamepad face button right: dash
- `Space` or left mouse / gamepad face button bottom: Light Strike

Win condition: purify the required Light Wells by holding position inside their radius.
Failure condition: health reaches zero from enemies or staying outside the circle.

## Automated UE Iteration Loop

Use this to build, launch, capture, quit, and create a review note:

```powershell
.\tools\ue-loop.ps1 -Name first-pass
```

Faster after a successful compile:

```powershell
.\tools\ue-loop.ps1 -Name fast-pass -SkipBuild
```

See `docs\UE_ITERATION_LOOP.md`.

## UE5 Procedural SFX

Use `docs\UE5_PROCEDURAL_SFX_FLOW.md` when adding or tuning first-pass gameplay audio.

Procedural SFX are allowed for immediate playable feedback while `audio-engine` production assets are pending. They must stay tied to reusable gameplay events and must be proven through `tools\ue-loop.ps1` captures before task state changes.

Generate the reusable WAV package with:

```powershell
npm run audio:generate
```

Use `npm run audio:generate:engine` when `audio-engine` is running on `http://127.0.0.1:8300` and provider credentials are configured. See `docs\AUDIO_ENGINE_GAME_AUDIO_FLOW.md`.

Do this after UE5 is installed:

1. Open `engine\LightWarrior.uproject`.
2. Let Unreal generate project files and compile the `LightWarrior` module.
3. Create `Content/Maps/L_LightWarrior_Arena.umap`.
4. Add `ASacredCircle`, `AThunderHammerTemple`, and a test `AShadowEnemy`.
5. Create Blueprint children for tuning and temporary visuals.
6. Run Play-In-Editor.
7. Capture evidence.
8. Update `TASK_GRAPH.json` and `PRODUCTION_LOG.md`.

## Media Engine Setup Task

Use `docs/MEDIA_ENGINE_INTEGRATION.md` before generating assets.

Required local repos:

- `D:\TE-Code\image-engine`
- `D:\TE-Code\video-engine`
- `D:\TE-Code\audio-engine`

Required service ports:

- image-engine: `3100`
- video-engine: `8200`
- audio-engine: `8300`
- Blender MCP: `9876`

Start/restart local media support:

```powershell
.\tools\start-media-services.ps1
.\tools\start-blender-mcp.ps1
```

Do not mark generated assets production-ready until they import into UE5 and have capture evidence.

## Completion Standard

Do not mark gameplay tasks `passed` or `shipped` unless:

- build/engine verification passes
- evidence is in `captures/`
- task state is updated
- production log records what changed
