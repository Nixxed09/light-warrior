# First Five Minutes

## Hook Contract

```text
Bravery expands the light.
```

Every beat in the first five minutes must reinforce this contract:

- Safety is visible inside the sacred circle.
- Tempting power is visible outside the circle.
- Darkness creates fair pressure when the player leaves safety.
- Thunder Hammer makes the brave choice feel worth it.
- The circle expands because the player acted bravely, not because time passed.

## First 10 Seconds

Goal:

```text
Where am I, what do I do first, and did the game respond?
```

Sequence:

1. Player appears inside a glowing sacred circle.
2. Camera frames the circle boundary, one shadow enemy, and the northern temple silhouette.
3. Player moves and dashes.
4. First enemy approaches with a readable tell.
5. Player uses light strike.
6. Enemy bursts into light.
7. Sacred circle expands visibly.

Success criteria:

- Player understands the circle is safe within 5 seconds.
- Player understands shadow outside the circle is dangerous without reading more than one short prompt.
- Player sees that action changes the arena.
- Player can infer that fighting bravely makes the light grow.

## First 30 Seconds

Sequence:

1. More enemies approach from corruption.
2. The circle slowly loses ground if the player stalls.
3. The temple glows outside the safe area.
4. A light trail or camera framing points toward the temple.
5. Player reaches Thunder Hammer.
6. Temple activation creates a power spike.
7. Hammer slam clears enemies and expands the circle.

Success criteria:

- Player leaves the circle by choice, not only by accident.
- Thunder Hammer feels worth the risk.
- The bold beat happens before the player is bored or confused.
- Player connects the temple risk to a larger, safer light circle.

## First Five Minutes

Intended session:

1. Learn safety: stand in light, see darkness pressure.
2. Learn action: dash and strike shadow imps.
3. Learn consequence: kills expand light, passivity shrinks it.
4. Learn risk: outside light drains health.
5. Learn reward: Thunder Hammer changes movement and impact.
6. Learn enemy contrast: imps swarm, berserkers charge with a tell.
7. Complete a 90-second survival wave.
8. See result: sanctuary reclaimed or darkness wins.
9. Restart quickly.

The intended emotional read is:

```text
I was safe, I got brave, I became stronger, and the arena changed because of me.
```

## First Mistakes

Expected mistakes:

- Player camps inside the circle.
- Player misses that the temple is interactable.
- Player loses the hero in VFX.
- Player does not understand why health is draining.

Recovery rules:

- The circle edge pulses when the player is outside safety.
- Damage feedback points back toward the circle.
- Temple glow must be visible from spawn.
- The first enemy should not kill the player before they test controls.

## UX Rules

- No lore screens before first input.
- No upgrade screen before the first successful temple beat.
- Keep prompts short and spatial.
- Every prompt must disappear once the player proves the behavior.

## UE5 First Playable Acceptance

The UE5 build does not pass first playable until:

- Play-In-Editor opens directly into the arena.
- Controller and keyboard movement both work.
- Dash, strike, circle expansion, health drain, temple activation, and one enemy kill are visible.
- A player who camps inside the circle is pressured enough to understand that safety alone is losing.
- A screenshot and 10-second capture exist in `captures/`.
