# Light Warrior Agent Guide

## Mission

Build Light Warrior as a real GamesOS game, not a disposable browser toy.

The production engine is Unreal Engine 5 (5.8 at `D:\Games\UE_5.8`). The earlier Vite/Three.js implementation remains in the repo as a legacy reference artifact.

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

- UE5: Unreal Editor 5.8 at `D:\Games\UE_5.8`. C++ module compiles. Procedural arena is playable.
- UE iteration loop: `tools/ue-loop.ps1` builds, launches, captures screenshots, and writes review metadata.
- Vite/Three.js prototype: legacy reference only (`npm run build` still passes).
- Blender LTS 4.5.5: installed.
- Unreal MCP: not configured locally.
- AI asset generation: blocked by missing AWS credentials / `TE_SERVICE_KEY`.

Do not mark UE5 production tasks complete until Play-In-Editor runs and screenshot/video evidence exists.

## Working Loop

Primary UE5 loop:

```powershell
.\tools\status.ps1
.\tools\verify.ps1 -RequireProductionEngine
.\tools\ue-loop.ps1 -Scenario <name>
```

Legacy prototype (reference only):

```powershell
npm run dev
```

## Evidence Rules

- Gameplay task: capture video or screenshot into `captures/`.
- Visual/style task: update `STYLE_GUIDE.md`, `ASSET_MANIFEST.json`, and capture evidence.
- Playtest task: update `PLAYTEST_NOTES.md`.
- State change: update `TASK_GRAPH.json`, `PIN.md`, and `PRODUCTION_LOG.md`.

## Do Not Do Yet

- Do not add more temples before the first 10 seconds are captured and reviewed.
- Do not add bosses before the vertical slice proves combat feel.
- Do not generate production assets without `STYLE_GUIDE.md` and `ASSET_MANIFEST.json`.

