# UE Iteration Loop

Goal: make Light Warrior improve through engine evidence, not guesswork. This is the project implementation of the GamesOS Playable Improvement Loop.

## Loop

```powershell
.\tools\ue-loop.ps1 -Name first-pass
```

This does:

1. Compile the UE5 module.
2. Launch `LightWarrior.uproject` in game mode.
3. Let the C++ game mode bootstrap the playable arena.
4. Request an in-game screenshot through `FScreenshotRequest`.
5. Quit Unreal.
6. Count UE log warnings/errors.
7. Write playable evidence JSON.
8. Write a review checklist to `captures\ue-loop\latest-review.md`.

Screenshots are written under:

```text
captures\ue-loop
```

The latest playable evidence is written to:

```text
captures\ue-loop\latest-evidence.json
```

## Faster Loop

After a successful compile:

```powershell
.\tools\ue-loop.ps1 -Name fast-pass -SkipBuild
```

## Playable Improvement Contract

Every loop should follow:

```text
run -> capture -> measure friction -> choose one fix -> implement -> rebuild -> replay
```

The Light Warrior loop currently measures:

- screenshot creation
- Unreal process exit code
- UE log warning count
- UE log error count
- objective completion in the capture
- whether the first loop is visible
- whether the HUD blocks action
- whether the huge flat world edge is visible

These measurements are intentionally small and practical. Add deeper telemetry only when it directly improves playtest decisions.

## Automation Flags

Common scenarios:

- `first-playable`: default opening state.
- `combat-readability`: places the player near enemy pressure and checks attack/hit readability.
- `first-light-well-loop`: proves a Light Well restoration payoff.
- `thunder-hammer`: triggers the Thunder Hammer activation path for bold-beat proof.

Use `docs\UE5_PROCEDURAL_SFX_FLOW.md` when a scenario is being used to prove audio event timing.

The game mode listens for:

- `-LWAutoCapture`
- `-LWShot=<absolute png path>`
- `-LWShotDelay=<seconds>`
- `-LWQuitDelay=<seconds>`

These are used by `tools\ue-capture.ps1`.

## Review Rule

Each pass should fix one highest-impact problem:

- spawn/readability
- HUD clarity
- objective clarity
- arena scale
- enemy readability
- temple temptation
- light/dark contrast
- Old Earth/Lila signal

Then run the loop again.
