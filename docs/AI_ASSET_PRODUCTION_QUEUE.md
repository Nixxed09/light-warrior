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

1. Generate `character.light_warrior` concept sheet.
2. Generate `enemy.shadow_imp` and `enemy.berserker` silhouette sheets.
3. Generate `vfx.sacred_circle` reference frames.
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

