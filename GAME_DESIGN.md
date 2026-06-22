# Light Warrior Game Design

## Player Fantasy

I am a warrior of light who reclaims sacred ground by stepping into danger and turning darkness into power.

## World Fantasy

I am fighting on Old Earth, a huge flat world-plane whose true scale has been hidden by false maps and broken routes. Each arena is a damaged Tartarian-resonance node. When I act bravely, I do not only win a fight; I restore working infrastructure in the world.

## Theme Alignment

Primary theme pack: `D:\TE-Code\ProductOS\GamesOS\theme-packs\ai-schism-luminous-guardian`.

Light Warrior uses that pack's luminous guardian versus dark influence pattern, translated into heroic arena action. The player is the human agency side of the theme: they must choose risk, reach sacred technology, and visibly push darkness back.

Secondary supports:

- `egyptian-resonance-temples` for sacred temple landmarks and resonance audio/architecture.
- `tartaria-unveiled` for restored-node language and geometry-as-circuitry environment states.

Expansion source of truth: `THEME_ALIGNMENT.md`.
World source of truth: `WORLD_MODEL.md`.
Lila source of truth: `LILA_ALIGNMENT.md`.
Systems source of truth: `GAME_SYSTEMS_LILA_PROGRESSION.md`.

## Explicit Hook

```text
Bravery expands the light.
```

The player is safest inside the sacred circle, but the power needed to win is outside it. The game should repeatedly ask the player to make a brave choice: stay safe and slowly lose ground, or step into darkness to claim power and expand the light.

In Lila terms, this is the Law of Mirrors and the Law of Resonance made physical: the field reflects the player's courage, and the node answers the frequency they bring into it.

## Core Loop

Fight from the light -> risk a run to a weapon temple -> unleash a power spike -> expand the sacred circle -> survive a stronger wave.

## Systems Loop

Risk creates Courage -> Courage activates temples -> temples convert darkness into Light -> Light expands the circle -> expanded circle reveals node geometry -> restored geometry yields true-map progress.

Shadow Debt grows when the player waits passively or abandons breaches. It returns later as stronger pressure, making cause and effect visible without turning the game punitive.

## Hook Loop

```text
See safety -> see tempting power outside safety -> choose risk -> survive darkness -> claim power -> expand light -> face a larger threat.
```

## First 10 Seconds

The player stands in a glowing circle, dashes into darkness, strikes a shadow demon, sees it burst into light, and watches the circle expand.

## First 30 Seconds

The player notices the Thunder Hammer temple outside the circle, leaves safety, takes pressure from darkness/enemies, activates the hammer, slams the first cluster, and sees the sacred circle expand dramatically.

## Full Game Vision

An epic UE5 third-person arena action game set across reclaimed nodes on the vast flat Old Earth plane, with readable heroic combat, weapon temples, living light/dark territory, true-map route discovery, escalating arenas, and replayable mastery.

## First Playable

One arena, movement, dash, light strike, Thunder Hammer temple, two enemies, sacred circle expansion/shrink, one 90-second wave.

First playable resources:

- Light from enemy bursts and temple activation.
- Courage from leaving safety and reaching Thunder Hammer.
- Shadow Debt as a simple pressure concept, only if readable.

First playable enemies:

- Shadow Imp teaches Resonance.
- Berserker Demon teaches clarity under pressure.

First playable weapon:

- Thunder Hammer teaches that brave contact with old technology expands the field.

## Non-Negotiable Design Tests

- If the player never leaves the circle, the wave should become difficult or unwinnable.
- If the player reaches the temple, the power spike should be unmistakable.
- If the player fights bravely, the circle should visibly reward them.
- If the player makes a mistake in darkness, recovery should be possible but tense.
- If guidance appears, it must help the player recognize the law without solving the fight.

## Vertical Slice

One polished arena with final-direction character movement, camera, lighting, VFX, audio, enemy tells, result flow, and 30-second gameplay proof.

## World Expansion Path

The first arena is the Thunder Hammer node. Future arenas are neighboring nodes revealed by true maps, temple keys, sky timing, and restored route lines. The player never walks to a visible world edge; the world stays larger than sight through haze, glare, storm curtains, mountain rings, firmament distortion, and locked routes.

## Lila And AI Expansion Path

Light Warrior starts with authored feedback. Runtime AI is optional and disabled until the UE5 first playable proves the loop.

The first AI enhancement is the Luminous Witness: the temple field speaking in short bounded lines. It reads only structured game state, proposes only approved verbs, and names the law after the player feels it fire.
