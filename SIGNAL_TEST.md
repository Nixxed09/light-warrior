# Signal Test

## Core Question

Does a target player understand the hook, then voluntarily leave safety to reach the Thunder Hammer temple because the risk/reward feels exciting?

Hook being tested:

```text
Bravery expands the light.
```

## Strong Signal

Strong signal looks like:

- Player leaves the circle intentionally after understanding the risk.
- Player reacts to the hammer slam without explanation.
- Player asks to retry after failure.
- Player says what the next weapon temple could be.
- Player can explain that brave action made the light grow.

## Weak Signal

Weak signal looks like:

- Player stays inside the circle and waits.
- Player reaches the temple only by accident.
- Player does not notice circle expansion.
- Player describes the game as a generic arena fighter.
- Player thinks the circle expansion is automatic or decorative.
- Player quits after one failure without wanting another run.

## Minimum Playtest Group

Before expanding production beyond the UE5 first playable:

- 3 target action players.
- 2 younger heroic fantasy players or parent-observed younger players.
- 1 non-target observer for readability notes.

## Test Script

Do not explain the full game. Tell the player only:

```text
Survive and reclaim the arena.
```

Observe:

- First movement.
- First attack.
- Whether they notice health drain outside light.
- Whether they choose the temple.
- Whether they connect temple risk to light expansion.
- Whether they retry.

## Evidence Required

For every signal test:

- Build identifier.
- Player type.
- Video or timestamped notes.
- First confusion.
- First voluntary risk.
- Did the player say or imply that bravery expanded the light?
- Retry behavior.
- Signal rating: `strong`, `mixed`, or `weak`.

## Kill Or Pivot Criteria

Prototype window:

- The browser prototype may answer rough readability questions.
- UE5 first playable must answer game-feel questions.

Pivot if:

- 3 of 5 target players do not voluntarily risk the temple after two runs.
- Players understand combat but ignore the circle.
- Players understand the circle but do not care about the temple reward.
- Players notice the temple reward but do not connect brave risk to reclaimed territory.

Pivot order:

1. Improve camera/readability.
2. Improve temple reward.
3. Change temple placement/risk.
4. Test fallback hook A or B from `PROVEN_BETTER_NEW.md`.
5. Reconsider the concept only if the player instinct fails.
