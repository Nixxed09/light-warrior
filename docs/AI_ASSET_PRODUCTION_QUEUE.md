# Light Warrior AI Asset Production Queue

## Purpose

This file applies the GamesOS AI-assisted production standard to Light Warrior. The goal is to replace the current playable UE5 blockout with assets that make the theme unmistakable and improve moment-to-moment readability.

Light Warrior production rule:

```text
Generated assets are candidates until they make "Bravery expands the light" more visible in UE5 gameplay.
```

Use:

- `D:\TE-Code\ProductOS\GamesOS\docs\GAMESOS_AI_ASSISTED_PRODUCTION_STANDARD.md`
- `D:\TE-Code\ProductOS\GamesOS\docs\GAMESOS_ASSET_CREATION_FLOW.md`
- `STYLE_GUIDE.md`
- `THEME_ALIGNMENT.md`
- `ASSET_MANIFEST.json`

Codex image generation may be used for fast in-session concept/reference images before `image-engine` packaging. Any useful Codex-generated image must be saved into the project package, recorded in metadata, reviewed against this file, and proven in UE5 before approval.

## Internal Image Gen 2 Batch

The repo now has a complete OpenAI image-generation batch for the first production pass:

```text
assets/generated/internal-image-gen-batch-20260623.json
tools/generate-openai-image-assets.ps1
```

Dry run:

```powershell
npm run assets:dry-run
```

Generate all PNG candidates:

```powershell
$env:OPENAI_API_KEY="<key>"
npm run assets:generate
```

Generate one asset:

```powershell
powershell -ExecutionPolicy Bypass -File tools\generate-openai-image-assets.ps1 -AssetId enemy.shadow_imp
```

The batch targets OpenAI's image generation endpoint with `gpt-image-2` by default and writes base64 responses into the asset package paths listed in the batch. If a workspace does not have that model enabled, pass `-Model <enabled-image-model>`.

The first full batch covers:

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

## Asset Routing

The system-level routing map is:

```text
assets/generated/asset-routing.json
```

Validate it with:

```powershell
npm run assets:validate-routing
```

Use this file to decide which generator owns each asset:

- Character, enemy, weapon, pickup, arena, and temple visuals use concept art as direction for Blender/video-engine packages, then UE5 import.
- Sacred circle, corruption, and restored stone use concept art as material/VFX reference and do not need Blender unless a mesh support piece is later added.
- HUD icons and upgrade cards use concept art as UI reference and go to UE UI/Canvas/UMG, not Blender.
- Music and SFX use audio-engine and do not use image generation or Blender.

Current generated Blender/model candidates:

- `environment.arena` -> `assets/generated/models/arena`
- `environment.thunder_hammer_temple` -> `assets/generated/models/thunder_hammer_temple`

Current concept packages are references only until their matching Blender/UE package exists and passes gameplay capture.

Current local blocker:

- The in-chat image tool can create references for review, but it did not expose saved file paths in this shell.
- This shell does not have `OPENAI_API_KEY` set, so the repo-local generator can prepare requests but cannot fetch PNGs yet.
- ProductOS `gen_asset.py` is still blocked by missing `boto3` and AWS CLI/Secrets Manager access.

## Current Priority

The next visual leap should happen in this order:

1. Hero body and face.
2. Shadow enemy silhouettes.
3. Sacred circle and light-strike VFX.
4. Arena/temple set dressing.
5. First SFX pack and short music loop.

Reason: the player currently has playable mechanics, but the avatar, enemies, and feedback are still too close to blockout primitives. Improving those first gives the biggest visible quality gain without hiding gameplay problems.

## Shared Package Contract

Every asset package should live under:

```text
assets/generated/
  concepts/<asset_id>/
  models/<asset_id>/
  audio/<asset_id>/
  metadata/<asset_id>/
```

Every package should include:

```text
request.json
prompt-package.json
asset_metadata.json
review-notes.md
```

UE5-imported packages should add:

```text
ue-import-result.json
playable-evidence.json
```

## Asset 1: `character.light_warrior`

### Gameplay Job

Third-person player avatar with visible body, face, glowing hammer, and white-gold/dawn-blue heroic identity during movement and combat.

### Concept Prompt Package

```json
{
  "asset_id": "character.light_warrior",
  "provider_system": "image-engine",
  "asset_type": "character",
  "style": "stylized heroic fantasy, UE5 third-person arena readability",
  "positive_prompt": "heroic light warrior character sheet, readable human face, strong athletic body, white-gold armor, dawn-blue luminous accents, glowing thunder hammer, brave warm expression, clean silhouette, front side back views, sacred gold light versus violet shadow theme",
  "negative_prompt": "gore, grimdark mud, horror, over-detailed armor noise, tiny face, hidden body, unreadable weapon, photorealistic clutter",
  "output_requirements": ["front view", "side view", "back view", "gameplay camera thumbnail", "hammer silhouette"]
}
```

### 3D Prompt Package

```json
{
  "asset_id": "character.light_warrior",
  "provider_system": "video-engine",
  "target_engine": "ue5",
  "asset_type": "character",
  "style": "stylized heroic fantasy, clean UE5 material slots, readable body and face",
  "description": "stylized heroic Light Warrior player character with readable face, white-gold armor, dawn-blue glow, simple strong body silhouette, glowing thunder hammer accessory, third-person arena scale, game-ready material slots",
  "max_poly_count": 80000,
  "ue5_lod_levels": 2,
  "output_dir": "D:/Phoenix/nix-code/games/light-warrior/assets/generated/models/character_light_warrior"
}
```

### Review Gates

- Face is visible in the default UE5 camera.
- Body silhouette reads while moving forward, strafing, and dashing.
- Hammer reads as the primary weapon.
- White/gold/blue hero identity is distinct from violet enemies.
- Capsule/collision fit does not make the character float or clip.
- UE5 capture exists.

## Asset 2: `enemy.shadow_imp`

### Gameplay Job

Fast weak enemy that teaches the player to strike darkness and see the sacred circle expand.

### Concept Prompt Package

```json
{
  "asset_id": "enemy.shadow_imp",
  "provider_system": "image-engine",
  "asset_type": "character",
  "style": "stylized fantasy enemy silhouette sheet",
  "positive_prompt": "small angular shadow imp enemy, violet-black living darkness, bright readable attack tell, jagged body, glowing violet eyes, dissolves into reclaimed gold light on defeat, clean gameplay silhouette, front side back views",
  "negative_prompt": "gore, horror realism, tiny details, muddy black shape, friendly cute mascot, unreadable attack tell",
  "output_requirements": ["silhouette sheet", "attack tell frame", "defeat dissolve frame"]
}
```

### Review Gates

- Different silhouette from the player at arena camera distance.
- Attack tell is visible before damage.
- Defeat state clearly turns darkness into light.
- Does not obscure sacred circle boundary.

## Asset 3: `enemy.berserker`

### Gameplay Job

Slower pressure enemy with a clear charge tell that teaches the player to move bravely rather than turtle inside the circle.

### Concept Prompt Package

```json
{
  "asset_id": "enemy.berserker",
  "provider_system": "image-engine",
  "asset_type": "character",
  "style": "stylized fantasy heavy enemy silhouette sheet",
  "positive_prompt": "large angular berserker demon made of violet-black corruption, heavy silhouette, glowing red-violet charge tell, cracked shadow armor, slow powerful stance, readable arena enemy, front side back views",
  "negative_prompt": "gore, excessive spikes, muddy silhouette, tiny details, heroic armor, hidden charge tell",
  "output_requirements": ["heavy silhouette", "charge windup frame", "stunned or defeated frame"]
}
```

### Review Gates

- Clearly larger/heavier than the shadow imp.
- Charge tell is readable during movement.
- Damage color stays red edge only; primary danger remains violet-black.

## Asset 4: `vfx.sacred_circle`

### Gameplay Job

Make safety, danger, and territory gain readable without text.

### Reference Frame Prompt Package

```json
{
  "asset_id": "vfx.sacred_circle",
  "provider_system": "image-engine",
  "asset_type": "background",
  "style": "stylized UE5 VFX reference frame, clean combat readability",
  "positive_prompt": "top-down and third-person reference frames of a sacred golden circle expanding across dark violet corruption, warm round sanctuary, dawn-blue inner glow, gold radial pulse, clean boundary, readable arena floor",
  "negative_prompt": "opaque smoke, particle wall, messy bloom, unreadable floor, horror gore, excessive sparks covering enemies",
  "output_requirements": ["idle frame", "expansion peak frame", "darkness pressure frame"]
}
```

### UE5 Implementation Gate

- Niagara or material animation expands within one second of a brave success.
- Boundary is visible while fighting.
- VFX does not hide enemies, wells, temple, or player.

## Asset 5: `environment.arena`

### Gameplay Job

Sacred node under pressure: clear safe center, tempting temple beyond the boundary, visible restored seams after success.

### 3D Kit Prompt Package

```json
{
  "asset_id": "environment.arena_kit",
  "provider_system": "video-engine",
  "target_engine": "ue5",
  "asset_type": "environment",
  "style": "stylized heroic fantasy arena kit, pale stone, gold inlays, violet corruption, readable navigation",
  "description": "modular UE5 arena set dressing kit with pale stone floor pieces, sacred gold seam strips, violet corruption chunks, light well pedestal, restored node markers, clean combat lanes",
  "max_poly_count": 60000,
  "ue5_lod_levels": 2,
  "output_dir": "D:/Phoenix/nix-code/games/light-warrior/assets/generated/models/environment_arena_kit"
}
```

### Review Gates

- Combat lanes remain readable.
- Corruption does not look like random clutter.
- Restored seams are visible after circle expansion.
- Temple remains the strongest landmark.

## Asset 6: `audio.core_sfx`

### Gameplay Job

Make bravery, danger, strike impact, enemy defeat, and restoration audible.

### Audio Pack Prompt Package

```json
{
  "asset_id": "audio.core_sfx",
  "provider_system": "audio-engine",
  "style": "heroic sacred fantasy, warm gold light, violet shadow pressure",
  "sfx": [
    {
      "id": "dash_shimmer",
      "prompt": "short bright dawn-blue dash shimmer, heroic but lightweight, no harsh transient"
    },
    {
      "id": "light_strike",
      "prompt": "crisp magical hammer-light strike, gold impact with small thunder crack"
    },
    {
      "id": "shadow_dissolve",
      "prompt": "violet shadow enemy dissolves into warm reclaimed light, satisfying but not horror"
    },
    {
      "id": "circle_expansion",
      "prompt": "sacred golden ring expands, choir shimmer and low pulse, one second"
    },
    {
      "id": "temple_activation",
      "prompt": "ancient temple chime activating sacred technology, gold and blue resonance"
    },
    {
      "id": "hammer_slam",
      "prompt": "thunder hammer slam, deep impact, clean lightning crack, not muddy"
    }
  ]
}
```

### Review Gates

- Strike and enemy defeat are distinct.
- Danger warning does not mask impact cues.
- Circle expansion is emotionally rewarding.
- UE5 playback is captured.

## Asset 7: `audio.first_loop`

### Gameplay Job

A short combat loop that supports heroic pressure without exhausting the player during repeated test runs.

### Music Prompt Package

```json
{
  "asset_id": "audio.first_loop",
  "provider_system": "audio-engine",
  "kind": "music_loop",
  "duration_seconds": 90,
  "loopable": true,
  "prompt": "heroic sacred arena combat loop, warm gold choir pads, restrained percussion, temple chimes, low violet shadow pressure, hopeful not grimdark, supports 90 second survival run"
}
```

### Review Gates

- Loops cleanly.
- Does not mask core SFX.
- Supports the 90-second run timer.
- Can be lowered under UI/result stingers later.

## First Implementation Sprint

### Sprint Goal

Make one UE5 capture where the player, enemies, circle, temple, and first feedback loop look and sound intentional.

### Tasks

1. Generate `character.light_warrior` concept sheet through Codex image generation or `image-engine`.
2. Generate `enemy.shadow_imp` and `enemy.berserker` silhouette sheets through Codex image generation or `image-engine`.
3. Generate `vfx.sacred_circle` reference frames through Codex image generation or `image-engine`.
4. Generate `audio.core_sfx` first pack.
5. Import or implement one improved asset from each category into UE5.
6. Capture a 30-second gameplay clip.
7. Update `ASSET_MANIFEST.json`, `TASK_GRAPH.json`, and `PRODUCTION_LOG.md`.

### Acceptance

- Player has a readable body, face direction, and weapon identity.
- Enemies are visually distinct from the player and each other.
- Circle expansion is obvious without reading UI.
- At least three SFX are wired: dash, strike, enemy dissolve or circle expansion.
- Capture proves the improved moment in actual UE5 gameplay.

## ProductOS Pull-In: 2026-06-23

Pulled the latest ProductOS GamesOS asset/style system and applied its stricter gates locally.

New local package shape:

```text
assets/generated/concepts/character_light_warrior/request.json
assets/generated/concepts/enemy_shadow_imp/request.json
assets/generated/concepts/vfx_sacred_circle/request.json
tools/gamesos-generate-concept.ps1
```

The wrapper calls:

```powershell
powershell -ExecutionPolicy Bypass -File tools\gamesos-generate-concept.ps1 -AssetId enemy.shadow_imp
```

Current generator blocker:

- `D:\TE-Code\ProductOS\GamesOS\tools\gen_asset.py` is present.
- `D:\TE-Code\teneo-production` is present.
- Python module `boto3` is missing.
- AWS CLI is not installed/on PATH, so Secrets Manager access is not yet configured.

Until that is fixed, UE5 style implementation continues through authored runtime materials, shape language, VFX rings, combat tells, and capture evidence.
