# Toolchain Status

## Last Checked

2026-06-25

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
- Internal OpenAI image-generation key is saved in the Windows user environment as `OPENAI_API_KEY`.
- `npm run assets:generate` produced the first internal-image-gen visual batch.
- `npm run assets:validate-routing` passes and maps concepts to Blender, UE VFX/materials, UE UI, or audio.
- `npm run audio:generate` produces the first reusable WAV package for `audio.core_sfx`.
- image-engine service responds on `http://127.0.0.1:3100`.
- video-engine service responds on `http://127.0.0.1:8200/api/health`.
- audio-engine service responds on `http://127.0.0.1:8300/api/health`.
- Blender MCP is listening on `127.0.0.1:9876`.
- video-engine reports Blender `connected`.
- video-engine reports FFmpeg `available`.

## Production Engine

- Unreal Editor 5.8 is installed at `D:\Games\UE_5.8`.
- `engine\LightWarrior.uproject` exists and compiles.
- First-pass procedural UE5 SFX are wired for gameplay feedback.
- Game-side audio package generation is wired through `tools\generate-audio-assets.ps1`; production provider output still belongs to `audio-engine`.
- Unreal MCP/editor automation is configured with UE 5.8 native `ModelContextProtocol`.
- Project-local MCP client config exists at `.mcp.json` for Claude Code-compatible clients.
- Project-local Codex MCP config exists at `.codex\config.toml`.
- Start Unreal MCP with `tools\start-unreal-mcp.ps1`; check endpoint reachability with `tools\check-unreal-mcp.ps1`.
- Verified 2026-06-25: `initialize` POST to `http://127.0.0.1:8000/mcp` returned protocol `2025-11-25`; `tools/list` returned `list_toolsets`, `describe_toolset`, and `call_tool`.

## Blocking Full Asset Pipeline

- video-engine health reports PrusaSlicer and Whisper not found.
- audio-engine health reports generation providers not configured.
- `audio-engine` provider-backed production SFX packages have not yet been imported into UE5.
- UE5 asset import evidence for generated FBX files has not been captured yet.
- ProductOS `gen_asset.py` remains blocked by missing `boto3` and AWS CLI/Secrets Manager access; use `tools/generate-openai-image-assets.ps1` for repeatable local image generation.

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

Internal image asset pipeline:

```powershell
npm run assets:dry-run
npm run assets:generate
npm run assets:validate-routing
```

Unreal MCP:

```powershell
powershell -ExecutionPolicy Bypass -File D:\Phoenix\nix-code\games\light-warrior\tools\start-unreal-mcp.ps1
powershell -ExecutionPolicy Bypass -File D:\Phoenix\nix-code\games\light-warrior\tools\check-unreal-mcp.ps1
```
