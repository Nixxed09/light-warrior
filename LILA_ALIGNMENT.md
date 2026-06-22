# LILA_ALIGNMENT - Light Warrior

Declares Light Warrior's cell in the Lila grid. This game keeps the laws physical and playable: the player learns by moving, risking, striking, restoring, and seeing the world answer.

## The Cell

| Axis | This game |
|---|---|
| Pillar | Sovereignty through brave action: the player reclaims ground from a control pressure that shrinks maps, suppresses routes, and trains fear. |
| Plane(s) | Aetheric first, with Mental and Source glimpses. The old world grid has working physics; the player's courage and focus decide how much of it can operate. |
| Octave | Asleep -> brave -> clear -> radiant. The player sees more of the node as they act with courage and integrity. |
| Hand of laws | Resonance, Poles, Consent, Mirrors, Arising, Remembering. |
| Teaching dial | Named for progression and upgrades; mostly wordless inside combat. The player should feel the law before seeing its name. |

## Why These Laws

**Resonance:** Light, temple geometry, hammer discharge, sacred circle growth, and enemy pressure are all field states. The player raises the field by acting bravely.

**Poles:** Darkness is raised, converted, and repaired where possible. Enemy defeat should read as corruption dissolving into light, not gore.

**Consent:** Shadow pressure takes ground only where the field has been abandoned. Player action withdraws that agreement and restores the node.

**Mirrors:** The circle shows the player's state. Brave play expands the field; fearful waiting lets the field contract.

**Arising:** Enemies are conditioned distortions held together by dark pressure. Later enemy families should expose the condition that created them.

**Remembering:** The full arc is the warrior remembering what the light is and why Old Earth nodes were built.

## The Tight Loop

The law must fire in seconds:

1. The player sees safety inside the sacred circle.
2. The player sees a temple or power outside the circle.
3. The player chooses danger instead of waiting.
4. The world answers: hammer power, enemy burst, circle expansion, restored geometry.
5. The arena becomes clearer and more alive because the player acted.

That is Lila inside Light Warrior: bravery is not a theme sentence. It is the lever that changes the world.

## Systems Contract

The full systems structure lives in `GAME_SYSTEMS_LILA_PROGRESSION.md`.

Short version:

- Resources are consequences: Light, Courage, Resonance Shards, True-Map Fragments, Shadow Debt, and Aether Charge.
- Weapons are temple functions: Thunder Hammer, Sun Spear, Bell Shield, Vortex Blade, Mercy Flame, and Star Bow.
- Enemies are control pressures: imps, berserkers, suppressors, tethers, mirror shades, null priests, debt collectors, and gate eaters.
- Progression moves node to node across Old Earth through true-map routes, not through a generic level list.
- Lessons are felt first and named later.

## Octave In Play

The Octave should begin as combat readability, not a menu stat.

- Low octave: darkness hides tells, map lines are faint, the temple feels distant, the player is reacting.
- Rising octave: enemy tells sharpen, route lines glow, resonance seams appear, temple geometry reads as machinery.
- High octave: true-map overlays, node links, hidden route gates, and Source-plane flashes become visible.

The player raises octave through brave action, restraint, truth, and restoration. The player lowers it through panic, passive waiting, exploitative play, and choosing power without restoration.

## AI Role

The first AI idea is the **Luminous Witness**: a state-aware guide and later bounded companion that notices the law firing and names it only when useful.

It is not a chatbot, quest solver, or combat autopilot. It is the temple field speaking from inside the world.

Allowed first verbs:

- `comment`
- `hint`
- `warn`
- `celebrate`
- `mark_optional_location`

The Luminous Witness must be complete as authored fallback lines before runtime AI is enabled.

## UE5 Translation

- `BP_SacredCircle` is the visible mirror of player bravery.
- `BP_ThunderHammerTemple` is an aetheric node and teacher.
- Enemy VFX should imply distortion collapsing into restored field light.
- UI should reveal laws through earned names, not front-loaded explanation.
- Future `BP_LuminousWitness` can read structured state and emit only approved verbs.
- `ULilaAlignmentData` stores active laws and octave bands for designers.
- `ULuminousWitnessFallbackData` stores authored fallback lines before runtime AI exists.

## Non-Negotiables

- The player remains the hero.
- AI never solves a fight.
- No runtime AI is required for first playable.
- Lila language stays in-world.
- The first playable still proves movement, dash, strike, circle, temple, enemy, timer, victory, and failure.
