# Media Engine Integration

## Purpose

Light Warrior should use the TE media engines as production support systems around the UE5 build, not as replacements for UE5 gameplay implementation.

The link is now explicit:

| Engine | Local Path | GitHub | Role For Light Warrior |
|---|---|---|---|
| image-engine | `D:\TE-Code\image-engine` | `https://github.com/Traviseric/image-engine` | Concept images, style sheets, character/temple/UI visual exploration, 2D reference frames. |
| video-engine | `D:\TE-Code\video-engine` | `https://github.com/Traviseric/video-engine` | Blender MCP, 3D props/characters/environments, FBX export for UE5, screenshot/video capture, trailer/clip assembly. |
| audio-engine | `D:\TE-Code\audio-engine` | `https://github.com/Traviseric/audio-engine` | SFX, music loop, mastering, voice/foley generation, audio manifest for video-engine/UE5. |

## Current Clone Status

- `video-engine` was cloned into `D:\TE-Code\video-engine`.
- `audio-engine` was cloned into `D:\TE-Code\audio-engine`.
- `image-engine` already existed and was pulled fast-forward from origin.
- Base dependencies are installed for all three engines.
- `image-engine` responds on `http://127.0.0.1:3100`.
- `video-engine` responds on `http://127.0.0.1:8200/api/health`.
- `audio-engine` responds on `http://127.0.0.1:8300/api/health`.
- Blender MCP responds on `127.0.0.1:9876`.
- video-engine health reports Blender `connected` and FFmpeg `available`.
- Current health evidence is saved at `captures/media-service-health.json`.

## Service Ports

| Service | Expected Port | Current Use |
|---|---:|---|
| image-engine | `3100` | `npm run dev -- --port 3100` or repo `npm run dev`; concept and game asset images. |
| video-engine | `8200` | FastAPI orchestration, game asset pipeline, Blender/FFmpeg/video jobs. |
| audio-engine | `8300` | FastAPI music/SFX/voice/mastering jobs. |
| Blender MCP | `9876` | TCP bridge used by video-engine for 3D creation/export. |

Startup helpers:

```powershell
.\tools\start-media-services.ps1
.\tools\start-blender-mcp.ps1
```

Local compatibility note: the installed `ahujasid/blender-mcp` add-on expects raw TCP commands shaped as `{"type":"get_scene_info","params":{}}`, not JSON-RPC `method` calls. The local clone at `D:\TE-Code\video-engine` has been patched so `src\engines\blender\client.py` uses the add-on protocol for direct TCP.

## Light Warrior Production Flow

See `docs/GAMESOS_ASSET_CREATION_FLOW.md` for the full studio-style asset workflow. The short version: use `image-engine` for concept direction, `video-engine` plus Blender for 3D candidates and UE5 FBX packages, `audio-engine` for feedback audio, and UE5 as the final acceptance environment.

The project-specific queue is now in `docs/AI_ASSET_PRODUCTION_QUEUE.md`. Use that file for the next hero, enemy, VFX, environment, and audio requests so every generated item has a gameplay job, prompt package, review gate, and UE5 evidence requirement.

## Media Engine First Principles

- `image-engine` makes images: concepts, character sheets, UI/key art, sprites, icons, backgrounds.
- `video-engine` cooks Blender-backed 3D assets: props, characters, enemies, environments, landmarks, GLB/FBX/USD candidates, previews, and video/trailer artifacts.
- `audio-engine` makes sound: music loops, SFX, voice, mastering, and audio manifests.
- UE5 still owns gameplay: movement, combat, physics, Blueprint logic, input, camera, save flow, and final in-engine proof.

For characters specifically, `video-engine` can create a 3D character candidate when `asset_type` is `character`, but that is not enough for a production character. Light Warrior must still track rigging, animation, capsule/collision fit, material setup, UE5 import, controller/AI hookup, and Play-In-Editor evidence.

### 1. Visual Concept And Style

Use `image-engine` first when the asset is still a 2D design question:

- Light warrior concept sheet.
- Shadow imp silhouette sheet.
- Berserker demon silhouette sheet.
- Thunder Hammer temple visual target.
- Sacred circle VFX reference frames.
- Steam capsule/key art explorations later.

Relevant interface:

- `D:\TE-Code\image-engine\mcp-server\README.md`
- MCP tool: `generate_game_asset`
- Useful styles: `stylized`, `low-poly`, `realistic`
- Useful moods: `heroic`, `magical`, `dark`, `vibrant`

### 2. 3D Asset Blockout And Export

Use `video-engine` when an asset needs to become a UE5 import candidate.

Relevant docs:

- `D:\TE-Code\video-engine\README.md`
- `D:\TE-Code\video-engine\specs\BLENDER_3D_TOOL.md`
- `D:\TE-Code\video-engine\docs\research\16-Game Asset Pipeline From Blender to Godot and Unreal Engine.md`
- `D:\TE-Code\video-engine\src\pipelines\game_asset\`

Game asset pipeline request shape:

```json
{
  "description": "stylized heroic Thunder Hammer temple pedestal with sacred gold rings and blue lightning core",
  "target_engine": "ue5",
  "asset_type": "environment",
  "style": "stylized heroic fantasy, readable silhouette, UE5 arena",
  "max_poly_count": 50000,
  "output_dir": "D:/Phoenix/nix-code/games/light-warrior/assets/generated/thunder_hammer_temple",
  "ue5_lod_levels": 2
}
```

Expected UE5 export behavior from `video-engine`:

- `target_engine: "ue5"` exports FBX.
- UE5 static mesh naming uses `SM_<name>_LOD#`.
- Packaging writes `asset_metadata.json`.
- Validation tracks poly count, vertex count, manifold state, UV presence, warnings, and errors.

Format decision for Light Warrior:

- Use FBX for UE5 static/skeletal asset import candidates.
- Use USD later for scene-level layout iteration if the UE5 project needs non-destructive arena/set dressing workflows.
- Keep `.blend` files and generated metadata as source/provenance.

### 3. Audio And Feedback

Use `audio-engine` for all game feedback audio and music planning.

Until production audio packages are generated, use `docs/UE5_PROCEDURAL_SFX_FLOW.md` for repeatable UE5 procedural SFX placeholders. Procedural SFX should use the same event names that final `audio-engine` assets will replace.

The game-side generator is now wired:

```powershell
npm run audio:dry-run
npm run audio:generate
npm run audio:generate:engine
```

See `docs/AUDIO_ENGINE_GAME_AUDIO_FLOW.md` for the command contract, fallback behavior, output package, and UE5 graduation checklist.

Relevant docs:

- `D:\TE-Code\audio-engine\README.md`
- `D:\TE-Code\audio-engine\specs\AUDIO_ENGINE.md`

Useful endpoints once the service is running:

```text
GET  http://localhost:8300/api/health
POST http://localhost:8300/api/generate/music
POST http://localhost:8300/api/generate/sfx
POST http://localhost:8300/api/process/master
POST http://localhost:8300/api/pipeline/ad_audio
```

First audio asset list:

- Dash shimmer.
- Light strike.
- Shadow imp burst.
- Berserker charge tell.
- Sacred circle expansion pulse.
- Outside-light damage warning.
- Temple activation chime.
- Thunder Hammer slam.
- 90-second heroic combat loop.

### 4. Capture And Trailer Proof

Use `video-engine` after UE5 first playable exists:

- Assemble 10-second first-action proof.
- Assemble 30-second Thunder Hammer signal clip.
- Export 16:9 gameplay proof.
- Later export Steam/social cuts.

Do not create store/trailer assets before the UE5 vertical slice exists.

## Asset Folder Contract

Generated artifacts for this project should go under:

```text
D:\Phoenix\nix-code\games\light-warrior\assets\generated\
```

Suggested subfolders:

```text
assets/generated/concepts/
assets/generated/models/
assets/generated/audio/
assets/generated/video/
assets/generated/metadata/
```

Each generated asset package should include:

- Source prompt/request JSON.
- Output file(s).
- Metadata/provenance JSON.
- Review notes.
- UE5 import result once imported.

## Readiness Gates

### Engine Availability

Before marking the media pipeline ready:

- `GET http://localhost:8200/api/health` succeeds for video-engine.
- `GET http://localhost:8300/api/health` succeeds for audio-engine.
- `GET http://localhost:3100` or configured image-engine health route succeeds.
- Blender MCP is listening on `127.0.0.1:9876`.

### Asset Production Gate

Before marking an asset production-ready:

- It exists under `assets/generated/`.
- It has metadata/provenance.
- It passes validation or has accepted warnings.
- It imports into UE5.
- It is visible in a Play-In-Editor capture.

## Current Blockers

- Provider credentials and service tokens still determine which generation calls can run unattended.
- PrusaSlicer and Whisper are not required for the first Light Warrior visual pass, but remain optional video-engine capabilities.
- image-engine `/api/health` requires auth; root route is the current unauthenticated liveness check.
- Every candidate still needs UE5 import/capture proof before approval.

## Immediate Next Steps

1. Configure generation provider credentials as needed.
2. Generate the hero, enemy, and sacred circle concept packages from `docs/AI_ASSET_PRODUCTION_QUEUE.md`.
3. Generate the core SFX pack with `npm run audio:generate`; use `npm run audio:generate:engine` once credentials are available.
4. Replace the procedural blockout with concept-driven geometry, materials, VFX, and SFX after each direction passes review.
5. Capture a 30-second UE5 proof clip before approving any candidate for the first playable.

## Current Generated Packages

- Concept request package: `assets/generated/concepts/thunder_hammer_temple`
- UE5 FBX candidate package: `assets/generated/models/thunder_hammer_temple`
