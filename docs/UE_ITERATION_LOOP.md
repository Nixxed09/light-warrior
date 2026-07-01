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
- whether attack tells, hit feedback, and the temple route are visible
- `route_clear_seconds` for First Temple Run route proofs
- `route_pacing_window_met` for the 3-minute paced proof
- `combat_gates_started`, `combat_gates_completed`, and `combat_gate_requirement_met` for paced proof combat gates
- `shard_pickups`, `temple_offering_spent`, `resource_requirement_met`, and final resource totals for paced proof resource flow
- `survival_pressure_applied`, `damage_events`, `final_health`, `total_damage_taken`, and `survival_requirement_met` for paced proof survival pressure
- `input_move_samples`, `input_hold_seconds`, `input_combat_actions`, `input_offering_actions`, `input_survival_actions`, and `input_requirement_met` for paced proof input intent
- `input_transcript_actions`, `input_transcript_written`, and `input_transcript_requirement_met` for the generated input transcript artifact
- whether the HUD blocks action
- whether the huge flat world edge is visible

These measurements are intentionally small and practical. Add deeper telemetry only when it directly improves playtest decisions.

## Automation Flags

Common scenarios:

- `first-playable`: default opening state.
- `combat-readability`: places the player near enemy pressure and checks attack/hit readability.
- `first-light-well-loop`: proves a Light Well restoration payoff.
- `thunder-hammer`: triggers the Thunder Hammer activation path for bold-beat proof.
- `first-temple-run`: stages restored wells, temple unlock, Thunder Hammer activation, light shards, and the final pressure wave.
- `first-temple-run-proof`: proves the full objective sequence in one PIE session.
- `first-temple-run-route-proof`: drives the pawn through the continuous objective route and requires `LW_ROUTE_PROOF_COMPLETE`.
- `first-temple-run-paced-proof`: uses authored input intent over the same route, requires the three well combat gates to start and complete, requires shard pickups plus a temple offering spend before activation, requires final-wave survival pressure and a healthy finish, requires movement/hold/combat/offering/survival input markers, and only passes when `route_clear_seconds` is between 150 and 190 seconds.

Run the current vertical-slice pacing gate with:

```powershell
.\tools\ue-loop.ps1 -Name first-temple-run-paced-proof -Scenario first-temple-run-paced-proof -ShotDelaySeconds 178 -QuitDelaySeconds 3 -SkipBuild
```

The fast route proof is still useful for reliability checks. The paced proof is the better production gate for the 3-minute vertical slice because it proves route completion, combat gate completion, resource gathering/spending, pressure survival, input intent, final-wave survival, and pacing in one run.

Each paced proof also writes an input transcript next to the evidence file:

```text
captures\ue-loop\<name>-<timestamp>-input-transcript.json
```

That transcript is generated from `LW_INPUT_ACTION` markers and records movement targets, objective holds, Light Strike actions, temple offering, and survival pressure. Manual runs use the same shape through `LW_PLAYER_INPUT_ACTION` markers.

To record a visible manual playtest with real player input markers:

```powershell
.\tools\ue-capture.ps1 -Name manual-first-temple-run -Scenario first-playable -ShotDelaySeconds 240 -QuitDelaySeconds 5 -RecordInput -Visible -SkipBuild
```

`-RecordInput` passes `-LWRecordInput` into the game. The character logs movement, look samples, dash, Light Strike, and Thunder Strike with timestamp and world location. `tools\ue-capture.ps1` then writes:

```text
captures\ue-loop\<name>-<timestamp>-input-transcript.json
```

Existing logs can be converted without rerunning Unreal:

```powershell
.\tools\extract-input-transcript.ps1 -LogPath captures\ue-loop\<run>.log
```

Use the manual transcript to compare where a human hesitates, gets lost, spams actions, misses pickups, avoids pressure, or fails to notice the Thunder Hammer route. Those are production signals for the next improvement loop.

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
