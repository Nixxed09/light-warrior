# Internal Image Asset Pipeline

## Purpose

This pipeline turns Light Warrior asset needs into repeatable, reviewable image candidates that feed Blender, UE5 materials/VFX, UI, and audio planning.

Generated images are not final game assets by default. They are source references until a routed package is imported or implemented in UE5 and proven in gameplay capture.

## Credential Setup

The OpenAI image API key is stored outside the repo as a Windows user environment variable:

```powershell
[Environment]::SetEnvironmentVariable("OPENAI_API_KEY", "<key>", "User")
```

Confirm it is available:

```powershell
[Environment]::GetEnvironmentVariable("OPENAI_API_KEY", "User") -ne $null
```

Do not commit keys into scripts, JSON, logs, docs, or generated metadata. The generator reads `OPENAI_API_KEY` from the current process first, then from the Windows user environment.

## Files

```text
assets/generated/internal-image-gen-batch-20260623.json
assets/generated/asset-routing.json
tools/generate-openai-image-assets.ps1
tools/validate-asset-routing.ps1
```

The batch defines prompts, sizes, output paths, and review gates. The routing file defines how each generated concept is used by the rest of production.

## Commands

Dry run the full batch:

```powershell
npm run assets:dry-run
```

Generate missing PNGs:

```powershell
npm run assets:generate
```

Generate one asset:

```powershell
powershell -ExecutionPolicy Bypass -File tools\generate-openai-image-assets.ps1 -AssetId enemy.shadow_imp
```

Regenerate an existing asset:

```powershell
powershell -ExecutionPolicy Bypass -File tools\generate-openai-image-assets.ps1 -AssetId enemy.shadow_imp -Force
```

Validate routing:

```powershell
npm run assets:validate-routing
```

Validate the project:

```powershell
npm run verify
```

## Resume Behavior

`npm run assets:generate` is resumable. If a PNG already exists, the generator skips it unless `-Force` is used.

Each generated package receives:

```text
concept_internal_image_gen_01.png
internal-image-gen-request.json
internal-image-gen-result.json
```

`internal-image-gen-request.json` records the exact prompt and review gate. `internal-image-gen-result.json` records the provider, model, output path, and candidate status.

## Use Cases

### Concept To Blender To UE

Use for:

- `character.light_warrior`
- `enemy.shadow_imp`
- `enemy.berserker`
- `weapon.thunder_hammer`
- `pickup.light_shard`

Flow:

1. Generate image concept.
2. Review silhouette, palette, and gameplay readability.
3. Create Blender/video-engine package under `assets/generated/models/<asset_id>/`.
4. Export FBX/LOD files and preserve `source.blend`.
5. Import into UE5 and wire the actor/component.
6. Capture gameplay evidence.

### Existing Blender To UE

Use for:

- `environment.arena`
- `environment.thunder_hammer_temple`

Flow:

1. Use existing Blender/FBX package as the model candidate.
2. Compare it against the generated concept reference.
3. Improve scale, materials, collision, landmark strength, and readable approach lanes.
4. Import into UE5 and capture gameplay evidence.

### Concept To UE Material Or VFX

Use for:

- `vfx.sacred_circle`
- `material.corruption`
- `material.restored_stone`

Flow:

1. Generate reference sheet.
2. Build/tune UE material, mesh pulse, or Niagara-style effect.
3. Validate in combat camera, not just in an isolated preview.
4. Confirm the effect does not hide player/enemy silhouettes.

### Concept To UE UI

Use for:

- `ui.hud_icons`
- `ui.upgrade_cards`

Flow:

1. Generate icon/card visual reference.
2. Crop, redraw, or rebuild as UE UI textures/Slate/UMG widgets.
3. Verify at actual HUD/card size.
4. Do not rely on text to make icons understandable.

### Audio Engine To UE

Use for:

- `audio.core_sfx`
- `audio.first_loop`

Flow:

1. Generate audio through audio-engine.
2. Import WAV/SoundCue/MetaSound assets into UE5.
3. Attach cues to gameplay events.
4. Validate timing and mix in a playable run.

## Improvement Loop

Every asset should move through this loop:

1. Define or update the `ASSET_MANIFEST.json` entry.
2. Confirm route in `assets/generated/asset-routing.json`.
3. Generate or regenerate candidate output.
4. Review against `STYLE_GUIDE.md`, `THEME_ALIGNMENT.md`, and the asset-specific acceptance gate.
5. Route the candidate to Blender, UE material/VFX, UE UI, or audio.
6. Capture UE5 evidence through the playable loop.
7. Update `PRODUCTION_LOG.md`, `TASK_GRAPH.json`, and the package review notes.
8. Keep the asset at candidate status until it improves normal gameplay readability.

The standard for approval is not whether the asset looks good in isolation. It must make the game world's reality clearer in play: the huge flat world, sacred technology, living darkness, luminous restoration, and the core rule that bravery expands the light.

## Current Generated Visual Set

The first internal image batch has generated PNG candidates for:

- `character.light_warrior`
- `enemy.shadow_imp`
- `enemy.berserker`
- `vfx.sacred_circle`
- `weapon.thunder_hammer`
- `environment.arena`
- `environment.thunder_hammer_temple`
- `pickup.light_shard`
- `material.corruption`
- `material.restored_stone`
- `ui.hud_icons`
- `ui.upgrade_cards`

Next production use is to promote the best candidates into Blender packages or UE runtime implementations according to `asset-routing.json`.
