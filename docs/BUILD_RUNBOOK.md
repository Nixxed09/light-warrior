# Light Warrior Build Runbook

## Start Every Session

```powershell
.\tools\status.ps1
.\tools\verify.ps1
```

Then read the next pending task in `TASK_GRAPH.json`.

## Current Best Next Task

`engine.ue5_shell`

Reason: production has moved into `engine/LightWarrior.uproject`, but UE5 is not installed/discoverable. The next milestone is opening the project in Unreal, compiling the module, creating the first arena map, and capturing Play-In-Editor evidence.

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
- first C++ classes for character, sacred circle, Thunder Hammer temple, shadow enemy, and game mode

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
