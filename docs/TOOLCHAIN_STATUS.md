# Toolchain Status

## Last Checked

2026-06-22

## Summary

UE5 is the active production engine. C++ compiles, the procedural arena is playable, and the automated UE iteration loop produces screenshot evidence. Some asset pipeline services remain blocked.

## Passing

- ProductOS repo exists at `D:\TE-Code\ProductOS`.
- image-engine repo exists at `D:\TE-Code\image-engine`.
- video-engine repo exists at `D:\TE-Code\video-engine`.
- audio-engine repo exists at `D:\TE-Code\audio-engine`.
- Blender LTS 4.5.5 is installed at `C:\Program Files\Blender Foundation\Blender 4.5\blender.exe`.
- Epic Games Launcher exists at `C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe`.
- Light Warrior project files exist.
- `GAME_MANIFEST.json`, `ASSET_MANIFEST.json`, and `TASK_GRAPH.json` are valid JSON.
- `quality_bar.target = real_game`.
- `engine_automation` is present and targets `unreal_mcp` plus `blender_mcp`.
- `tools\verify.ps1` exists.
- `npm run build` passes for the Vite/Three.js prototype.
- GamesOS design foundation validator passes.
- image-engine service responds on `http://127.0.0.1:3100`.
- video-engine service responds on `http://127.0.0.1:8200/api/health`.
- audio-engine service responds on `http://127.0.0.1:8300/api/health`.
- Blender MCP is listening on `127.0.0.1:9876`.
- video-engine reports Blender `connected`.
- video-engine reports FFmpeg `available`.

## Production Engine

- Unreal Editor 5.8 is installed at `D:\Games\UE_5.8`.
- `engine\LightWarrior.uproject` exists and compiles.
- Unreal MCP/editor automation is not yet configured.

## Blocking Full Asset Pipeline

- video-engine health reports PrusaSlicer and Whisper not found.
- audio-engine health reports generation providers not configured.
- UE5 asset import evidence for generated FBX files has not been captured yet.

## Current Valid Work

- Build and iterate in UE5 using `tools/ue-loop.ps1`
- Capture first 10 seconds from the UE5 playable slice
- Import generated FBX assets into UE5 and capture evidence
- Refine docs, task graph, and playtest notes
- Create Blender blockout assets and import into UE5

Do not mark gameplay tasks complete without Play-In-Editor screenshot or video evidence.

## Install Attempt Notes

- 2026-06-16: Installed Blender LTS 4.5.5 through `winget`.
- 2026-06-16: `winget` found existing Epic Games Launcher `1.3.93.0`; upgrade to `1.3.189.0` failed with MSI exit code `1603`.
- 2026-06-16: No `UnrealEditor.exe` found under common local paths.
- 2026-06-16: Cloned video-engine and audio-engine into `D:\TE-Code`; pulled image-engine from origin.
- 2026-06-16: Installed base Python dependencies for video-engine/audio-engine in local `.venv` folders and started services on ports `8200` and `8300`.
- 2026-06-16: Installed image-engine npm dependencies and started image-engine on port `3100`.
- 2026-06-16: Installed FFmpeg through winget and restarted video-engine with FFmpeg on PATH.
- 2026-06-16: Cloned `ahujasid/blender-mcp`, enabled the add-on in Blender 4.5, started Blender MCP on `9876`, and patched the local video-engine Blender client for the add-on's raw TCP protocol. video-engine now reports Blender connected.

## Commands

Prototype verifier:

```powershell
.\tools\verify.ps1
```

Require UE5:

```powershell
.\tools\verify.ps1 -RequireProductionEngine
```

GamesOS design foundation:

```powershell
powershell -ExecutionPolicy Bypass -File D:\TE-Code\ProductOS\scripts\validate_gamesos_design_foundation.ps1 -GameProject D:\Phoenix\nix-code\games\light-warrior
```

GamesOS toolchain:

```powershell
powershell -ExecutionPolicy Bypass -File D:\TE-Code\ProductOS\scripts\validate_gamesos_toolchain.ps1 -ProductRoot D:\TE-Code\ProductOS -CodeRoot D:\TE-Code -GameProject D:\Phoenix\nix-code\games\light-warrior
```
