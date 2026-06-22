# Light Warrior UE5 Production Folder

This folder is the Unreal Engine production project root.

Project descriptor:

```text
engine/LightWarrior.uproject
```

Do not move the Vite/Three.js prototype into this folder. The browser prototype is a disposable mechanic sketch used to test the sacred-circle loop.

## Current State

The UE5 project shell and first C++ gameplay classes exist. Unreal Editor is available at:

```text
D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe
```

Compilation succeeds through UnrealBuildTool with UE 5.8. Map creation, Play-In-Editor, and screenshot verification remain to be completed in the editor.

Created production classes:

- `ALightWarriorCharacter`
- `ASacredCircle`
- `AThunderHammerTemple`
- `AShadowEnemy`
- `ALightWarriorGameMode`

Current code also includes a first-pass run state in `ALightWarriorGameMode`: auto-start, 90-second survival timer, victory on timer completion, and failure when the player reaches zero health. This compiles and still needs Play-In-Editor evidence.

Open `LightWarrior.uproject`, let Unreal generate project files if prompted, and create Blueprint children for first-playable tuning.

## First UE5 Shell Gate

Before `engine.ue5_shell` can be marked complete:

- `LightWarrior.uproject` exists here.
- The project opens in Unreal Editor.
- The `LightWarrior` module compiles.
- An empty arena level runs in Play-In-Editor.
- A screenshot or video is saved to `captures/`.
- `PRODUCTION_LOG.md` records the UE version and verification result.

## First Production Classes

Planned first playable classes:

- `BP_LightWarriorCharacter`
- `BP_SacredCircle`
- `BP_ThunderHammerTemple`
- `BP_ShadowImp`
- `BP_BerserkerDemon`
- `BP_ArenaGameMode`

See `../TECHNICAL_PLAN.md` for the implementation contract.

World model:

- `../WORLD_MODEL.md`
- The first arena is a reclaimed node on the huge flat Old Earth plane.
- Horizon treatment should imply unreachable scale through haze, glare, storm walls, mountain rings, and firmament distortion.
- Do not use globe, orbit, space, or visible edge framing.
