# GamesOS Asset Creation Flow

## Purpose

GamesOS assets should move through a repeatable studio-style pipeline, not one-off generation. The goal is to turn a gameplay need into an approved UE5-ready asset with provenance, visual review, import settings, and Play-In-Editor proof.

For Light Warrior, the theme rule is:

```text
Bravery expands the light.
```

Every character, enemy, prop, VFX, level piece, and audio cue should make that rule easier to read in normal play.

## Tool Roles

| Tool | Role | Not Responsible For |
|---|---|---|
| `image-engine` | Concept sheets, style explorations, reference frames, UI/key-art directions | Final UE5 meshes, collision, animation, gameplay integration |
| `video-engine` | Blender-backed 3D asset jobs, FBX/GLB export, metadata, mesh validation, preview/capture/trailer assembly | Final art direction, skeletal rig quality, UE import tuning, final in-engine acceptance |
| Blender + Blender MCP | Modeling, procedural blockouts, UV/material setup, LOD generation, preview renders, `.blend` source files | Gameplay logic or final UE actor setup |
| UE5 | Final import, materials, collision, sockets, animation blueprints, Niagara/VFX, gameplay fit, Play-In-Editor proof | Raw asset generation |
| `audio-engine` | SFX, music loops, mastering, audio manifests | Gameplay event wiring inside UE5 |

## Correct Pipeline

### 1. Asset Brief

Create or update an entry in `ASSET_MANIFEST.json`.

Required fields:

- asset id, for example `character.light_warrior`
- kind: `character`, `prop`, `environment`, `effect`, `music_loop`, or `sfx_pack`
- theme/style anchors
- gameplay job
- readability requirement
- prompt brief
- target folder under `assets/generated/`

No generation starts until the gameplay job is clear.

### 2. Concept Sheet

Use `image-engine` first when the design is still visual.

Output package:

```text
assets/generated/concepts/<asset_id>/
  request.json
  concept_01.png
  concept_02.png
  concept_03.png
  review-notes.md
```

Review gate:

- silhouette reads at gameplay distance
- palette matches `STYLE_GUIDE.md`
- theme role is obvious
- no visual noise that hurts combat readability
- one direction is approved before 3D work starts

### 3. Blender Source Asset

Use Blender directly or through `video-engine`/Blender MCP.

For generated 3D candidates, video-engine already supports:

- `POST /api/pipeline/game_asset`
- `target_engine: "ue5"`
- `asset_type: "prop" | "character" | "environment"`
- FBX export for UE5
- `SM_<name>_LOD#` naming
- `asset_metadata.json`
- validation fields for poly count, vertices, manifold state, UV warnings/errors

Recommended request shape:

```json
{
  "description": "stylized heroic Light Warrior character, white-gold and dawn-blue, readable face, glowing hammer, simple strong silhouette, UE5 third-person arena",
  "target_engine": "ue5",
  "asset_type": "character",
  "style": "stylized heroic fantasy, readable gameplay silhouette, sacred gold light, violet darkness contrast",
  "max_poly_count": 50000,
  "output_dir": "D:/Phoenix/nix-code/games/light-warrior/assets/generated/models/character_light_warrior",
  "ue5_lod_levels": 2
}
```

Blender package:

```text
assets/generated/models/<asset_id>/
  source.blend
  SM_<name>_LOD0.fbx
  SM_<name>_LOD1.fbx
  preview.png
  asset_metadata.json
  ue5-import-notes.md
  review-notes.md
```

Review gate:

- mesh scale matches UE mannequin-ish gameplay scale
- pivot/origin is correct
- LOD0 and LOD1 exist for UE5 static assets when needed
- UV status is known
- collision need is documented
- material slots are named
- source `.blend` is preserved

### 4. UE5 Import And Setup

Do not call an asset production-ready just because an FBX exists.

UE5 setup must include:

- imported mesh under `engine/Content/`
- material instances
- collision or capsule fit
- sockets if used by gameplay
- Blueprint child or native actor reference
- scale verified next to the player
- readable in the actual camera
- Play-In-Editor capture

Character-specific extras:

- rig/skeleton decision
- animation set or placeholder animation plan
- movement capsule fit
- face/body readability in the real camera
- attack/hit/death poses or animation stubs

### 5. Audio Package

Use `audio-engine` for interaction feedback once gameplay events exist.

First Light Warrior audio package:

```text
assets/generated/audio/core_sfx/
  dash_shimmer.wav
  light_strike.wav
  shadow_break.wav
  circle_expand.wav
  well_restore.wav
  temple_awaken.wav
  thunder_hammer_slam.wav
  audio_metadata.json
  ue5-import-notes.md
```

Review gate:

- sound has clear gameplay meaning
- no masking of important combat cues
- loudness normalized
- UE5 cue/import path documented

### 6. In-Engine Proof

Every asset must end with evidence.

Required proof:

```text
captures/<asset_id>-ue5.png
captures/<asset_id>-gameplay.mp4
```

Acceptance:

- visible in standalone or Play-In-Editor
- readable at gameplay distance
- supports the hook
- no severe scale/material/collision issues
- task graph and production log updated

## Asset Status Ladder

Use these statuses in `ASSET_MANIFEST.json`:

| Status | Meaning |
|---|---|
| `planned` | Need identified, no approved concept |
| `concept_candidate` | Concept generated, not approved |
| `concept_approved` | Visual direction selected |
| `model_candidate` | Blender/video-engine model generated |
| `ue5_imported` | Asset imports into UE5 |
| `playable_candidate` | Asset is visible and usable in a playable build |
| `approved_first_playable` | Good enough for current vertical slice |
| `needs_rework` | Failed review gate |

## What To Automate Next

The current video-engine `game_asset` pipeline is a good foundation, but GamesOS needs a higher-level wrapper that enforces this studio flow.

Needed wrapper:

```text
gamesos_asset_job
  input: asset_id + ASSET_MANIFEST entry
  stage 1: generate/reference concepts
  stage 2: create Blender model candidate
  stage 3: export UE5 FBX/metadata
  stage 4: validate mesh/package
  stage 5: produce preview sheet
  stage 6: create UE5 import checklist
  stage 7: require human/art-director approval
```

Do not let this wrapper auto-approve production assets. It should prepare candidates and evidence. Final acceptance happens in UE5.

## Light Warrior Priority Order

1. `character.light_warrior`: readable hero body/face/hammer.
2. `enemy.shadow_imp`: small fast enemy with clear violet silhouette and attack tell.
3. `vfx.sacred_circle`: radial expansion event and safe/danger boundary.
4. `environment.arena`: authored arena floor, corrupted outside, restored paths.
5. `environment.thunder_hammer_temple`: replace placeholder temple with approved FBX/imported mesh.
6. `audio.core_sfx`: dash, strike, restore, hammer, enemy burst.

The next best asset job is `character.light_warrior`, because the current UE5 character is still made of primitive placeholder meshes and the player has already flagged readability issues.
