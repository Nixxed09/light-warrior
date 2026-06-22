# Style Guide

## Identity Sentence

A heroic light-vs-dark arena where sacred gold light pushes back living violet shadow through brave movement and explosive weapon moments.

## Theme Pack Alignment

Primary GamesOS theme pack:

```text
D:\TE-Code\ProductOS\GamesOS\theme-packs\ai-schism-luminous-guardian
```

Light Warrior translates that pack's luminous guardian versus dark influence premise into physical arena combat. The hero is the human agency piece: bravery is not a line of lore, it is the player leaving safety, claiming sacred technology, and expanding reclaimed ground.

Secondary supports:

- `egyptian-resonance-temples` for sacred temple architecture, resonance, stone, gold, and ritual audio.
- `tartaria-unveiled` for restored nodes, functional geometry, and dormant structures lighting back up.

Use `THEME_ALIGNMENT.md` before adding characters, levels, weapons, enemies, or major mechanics.

## Hook Expression

```text
Bravery expands the light.
```

The style must make the hook readable before UI text explains it. The player should see a safe golden circle, a dangerous violet outside, a tempting temple beyond the boundary, and a visible burst of reclaimed ground after a brave success.

## Reference Mix

- Proven reference: Hades arena readability.
- Better direction: third-person heroic light territory with clearer safety/risk language.
- New signature: weapon temples outside the safe circle.

## Shape Language

- Primary silhouettes: strong warrior profile, circular sacred ground, angular shadow enemies, monumental temples.
- Repeated forms: rings, rays, fractured shadow spikes, temple arches.
- Forbidden forms: muddy realism, horror gore, noisy particle walls.
- Hook rule: safe space is round and warm; danger is jagged and violet; brave rewards radiate outward from the player.

## Color Language

- World palette: deep violets, midnight blue, warm gold, pale stone.
- Character palette: white/gold/blue hero accents.
- UI palette: clean dark panels with gold highlights.
- Reward/highlight color: radiant gold.
- Danger/error color: violet-black corruption with red edge only for damage.
- Hook contrast: temple rewards may share gold with the circle, but their glow must sit beyond the boundary so the risk reads instantly.

## Character Direction

- Player/avatar: readable heroic warrior with glowing weapon silhouette.
- Main creatures/NPCs: none in first playable.
- Enemy or obstacle language: shadow bodies with bright attack tells.
- Expression rules: enemies telegraph before attacks; hero glows stronger near power.
- Animation personality: brave, fast, snappy, weighty on impact.

## World Direction

- Biomes/locations: sacred arena, corrupted temple, light wells.
- Prop language: ancient stone, gold inlays, shadow cracks.
- Scale rules: temples are landmarks visible across arena.
- Materials/textures: luminous magic, worn stone, smoky shadow.

## UI And HUD Skin

- HUD tone: minimal heroic combat HUD.
- Icon style: gold glyphs on dark clean shapes.
- Button/panel treatment: readable, controller-friendly, low clutter.
- Feedback states: light charge, temple active, danger outside circle.
- Hook HUD rule: any prompt or meter must support the safety-risk-reward read; do not add UI that makes the circle feel like a generic health aura.
- Current UE HUD: dark translucent status panel, top-center node objective, Light/Courage resource pills, active well progress, hammer timer, sparse center reticle, and bottom control strip.
- HUD priority order: objective, health, well progress, run timer, active power, resources. Anything lower waits for UMG polish.

## Audio Identity

- Music mood: heroic percussion, choir, bright synth/magic layers.
- Primary interaction sounds: dash shimmer, light strike, shadow burst.
- Creature/character sounds: shadow snarls and dissolves.
- UI sounds: clean temple chimes.
- Ambient bed: sacred hum under shadow pressure.

## Motion And Feel

- Movement adjectives: brave, crisp, sweeping, powerful.
- Camera personality: cinematic but readable.
- Hit/collect/place feedback: strong hit stop, light burst, circle pulse.
- Particle/juice rules: readable arcs and bursts, no opaque clouds over enemies.
- Circle expansion rule: the expansion must be a visible radial event within one second of a brave combat or temple success.

## Asset Prompt Rules

- Prompt ingredients: heroic fantasy, sacred gold light, violet living darkness, readable silhouette, UE5 arena.
- Negative prompt rules: gore, grimdark mud, over-detailed armor, unreadable VFX.
- Resolution/import requirements: UE5-ready scale and materials.
- Naming convention: `category.object.variant_state`.

## Evidence

- Screenshot/video references: pending.
- Approved assets: none yet.
- Rejected directions: flat canvas shooter, generic medieval arena.
