# Engine Transition: Three.js Sketch To UE5 Production

## Decision

Light Warrior has moved from a Vite + Three.js browser mechanic sketch to Unreal Engine 5 production.

The browser build is still useful as fast evidence for mechanic readability, but it is not the production architecture, vertical slice, art target, or first playable under GamesOS standards.

## Current Authority

- Production engine: Unreal Engine 5.
- UE project: `engine/LightWarrior.uproject`.
- Manifest authority: `GAME_MANIFEST.json` uses `target_engine: "unreal"` and `engine_automation.primary_adapter: "unreal_mcp"`.
- Current production lane: `direct_unreal`.
- Setup authority: `D:\TE-Code\ProductOS\GamesOS\docs\MACHINE_SETUP_STANDARD.md`.
- Engine standard: `D:\TE-Code\ProductOS\GamesOS\docs\ENGINE_MCP_PRODUCTION_STANDARD.md`.

## What Three.js Can Still Do

- Prove a mechanic in minutes before committing UE5 work.
- Test camera framing, first-10-seconds readability, and control assumptions.
- Produce reference captures that directly inform UE5 implementation.
- Help compare risk/reward loop clarity before assets or Blueprints are final.

## What Three.js Must Not Do

- Become the long-term gameplay architecture.
- Define final art, animation, VFX, audio, progression, save/settings, or content pipelines.
- Count as the GamesOS first playable or vertical slice.
- Delay the UE5 build loop once the engine is available.

## UE5 Proof Gates

A Light Warrior gameplay improvement is production-recognized when it has:

- A compiling UE5 module via `tools\ue-build.ps1`.
- Play-In-Editor or standalone evidence from `tools\ue-loop.ps1`.
- Screenshot, JSON, and review notes in `captures\ue-loop\`.
- Updated task state in `TASK_GRAPH.json`.
- A short production note in `PRODUCTION_LOG.md`.
- Any generated assets tracked through `ASSET_MANIFEST.json` and imported or staged for UE5.

## Current Command Path

```powershell
.\tools\status.ps1
.\tools\verify.ps1
.\tools\ue-build.ps1
.\tools\ue-loop.ps1 -Name playable-pass
```

Open manually when needed:

```powershell
& "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject"
```

## Canonical References

- `README.md` - project direction and current prototype status.
- `MINIMUM_VIABLE_IDEA.md` - why the browser sketch is disposable.
- `TECHNICAL_PLAN.md` - UE5 architecture and module targets.
- `docs\BUILD_RUNBOOK.md` - build, launch, capture, and evidence loop.
- `GAME_MANIFEST.json` - machine-readable engine authority.
- `D:\TE-Code\ProductOS\GamesOS\docs\GAMESOS_STUDIO_OPERATING_SYSTEM.md` - GamesOS studio workflow.
- `D:\TE-Code\ProductOS\GamesOS\docs\GAMESOS_AI_ASSISTED_PRODUCTION_STANDARD.md` - asset and evidence standard.
- `D:\TE-Code\ProductOS\GamesOS\docs\GAMESOS_TOOL_INTEGRATION_SPEC.md` - tool lanes and adapters.
