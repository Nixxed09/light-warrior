# Light Warrior

Fresh GamesOS rebuild of the original `light-warrior` concept.

Old project archive:

```text
D:\Phoenix\nix-code\games\archive\pre-gamesos-redesign-20260616\light-warrior
```

Start with `GAME_SPEC.md`.

Theme source of truth:

- `WORLD_MODEL.md`
- `LILA_ALIGNMENT.md`
- `GAME_SYSTEMS_LILA_PROGRESSION.md`
- `THEME_ALIGNMENT.md`
- Primary pack: `D:\TE-Code\ProductOS\GamesOS\theme-packs\ai-schism-luminous-guardian`
- Support packs: `egyptian-resonance-temples` and `tartaria-unveiled`

## Current Prototype

The repo contains a Vite + Three.js mechanic sketch for proving the sacred-circle loop before a UE5 production build.

Under the current GamesOS standard, this browser build is not the production foundation and should not be treated as the first playable or vertical slice.

```powershell
npm install
npm run dev -- --port 5177
npm run build
```

Local dev URL:

```text
http://127.0.0.1:5177
```

Implemented in this pass:

- Third-person arena camera.
- Camera-relative WASD movement, Q/E or drag camera orbit, Shift dash, Space light strike.
- Sacred circle safety zone that expands on kills and shrinks under pressure.
- Health drain when the player leaves the light.
- Thunder Hammer temple at the north edge of the arena.
- Two enemy types: fast shadow imps and charging berserkers.
- 90-second victory state and failure state.

## Production Direction

Production target: Unreal Engine 5.

World target: one reclaimed combat node on the huge flat Old Earth plane. The first playable should feel like a small local fight inside a much larger Tartarian-resonance grid, with no globe, space framing, or visible world edge.

Next real milestone: UE5 first playable with movement, dash, light strike, sacred circle, one enemy, Thunder Hammer temple, 90-second run state, and Play-In-Editor evidence.

Read these before expanding scope:

- `LILA_ALIGNMENT.md`
- `docs/AI_LUMINOUS_WITNESS_CONTRACT.md`
- `PROVEN_BETTER_NEW.md`
- `FIRST_FIVE_MINUTES.md`
- `SIGNAL_TEST.md`
- `MINIMUM_VIABLE_IDEA.md`
- `PLAYER_THANK_YOU.md`
- `DISTRIBUTION_STRATEGY.md`
- `REMIX_PRIMITIVES.md`
