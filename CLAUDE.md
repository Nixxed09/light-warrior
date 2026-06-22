# Light Warrior Agent Guide

## Mission

Build Light Warrior as a real GamesOS game, not a disposable browser toy.

The current Vite/Three.js implementation is a first-playable prototype and evidence artifact. The production target remains Unreal Engine 5 unless the game direction is explicitly changed.

## Required Reading Order

1. `PIN.md`
2. `TASK_GRAPH.json`
3. `GAME_SPEC.md`
4. `GAME_DESIGN.md`
5. `REFERENCE_GAMES.md`
6. `STYLE_GUIDE.md`
7. `TECHNICAL_PLAN.md`
8. `docs/ENVIRONMENT_SETUP.md`

## Current Environment Reality

- Vite/Three.js prototype: available.
- `npm run build`: passes.
- UE5: not installed or not discoverable locally.
- Unreal MCP: not configured locally.
- Blender/audio/video asset services: not currently available.

Do not mark UE5 production tasks complete until Unreal opens, Play-In-Editor runs, and screenshot/video evidence exists.

## Working Loop

For prototype work:

```powershell
.\tools\status.ps1
.\tools\verify.ps1
npm run dev
```

For production UE5 work after setup:

```powershell
.\tools\status.ps1
.\tools\verify.ps1 -RequireProductionEngine
```

## Evidence Rules

- Gameplay task: capture video or screenshot into `captures/`.
- Visual/style task: update `STYLE_GUIDE.md`, `ASSET_MANIFEST.json`, and capture evidence.
- Playtest task: update `PLAYTEST_NOTES.md`.
- State change: update `TASK_GRAPH.json`, `PIN.md`, and `PRODUCTION_LOG.md`.

## Do Not Do Yet

- Do not add more temples before the first 10 seconds are captured and reviewed.
- Do not add bosses before the vertical slice proves combat feel.
- Do not treat the Three.js prototype as final production architecture.
- Do not generate production assets without `STYLE_GUIDE.md` and `ASSET_MANIFEST.json`.

