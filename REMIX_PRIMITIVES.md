# Remix Primitives

## Purpose

Light Warrior should create reusable GamesOS primitives for future arena action games.

## Candidate Primitives

### Sacred Territory System

Reusable idea:

- A visible territory radius that expands from successful action and shrinks from enemy pressure or player mistakes.

Inputs:

- Player position.
- Enemy kills.
- Damage state.
- Time pressure.

Outputs:

- Safe radius.
- Boundary VFX.
- Health/damage modifier.
- Progress feedback.

### Temple Risk Reward Actor

Reusable idea:

- A landmark outside safety that grants a temporary power spike.

Inputs:

- Player proximity.
- Activation state.
- Cooldown or one-shot rule.

Outputs:

- Weapon mode.
- VFX/audio burst.
- UI timer.
- Enemy-clearing power moment.

### Readable Arena Camera

Reusable idea:

- A third-person arena camera that keeps player, nearest threat, objective landmark, and territory boundary visible.

Inputs:

- Player velocity.
- Threat positions.
- Objective position.
- Circle radius.

Outputs:

- Camera yaw, pitch, distance.
- Soft target.
- optional lock-on or assist.

### Shadow Enemy Tell Pattern

Reusable idea:

- Enemy family with simple silhouettes and clear pre-attack tells.

Families:

- Swarm imp.
- Charging berserker.
- Future ranged corruptor.

### Light Burst Feedback

Reusable idea:

- Unified kill/impact feedback that communicates reward and territory expansion without hiding combat.

Outputs:

- Hit stop.
- Radial light burst.
- Circle pulse.
- SFX event.

## Metadata To Preserve

For each primitive when implemented in UE5:

- Blueprint/class name.
- Required components.
- Tunable parameters.
- VFX/audio dependencies.
- Capture showing expected behavior.
- Notes on what failed in playtest.

## Current Status

The Three.js prototype contains rough versions of these primitives, but they are not production primitives.

The production primitive library starts only when the UE5 implementation exists and Play-In-Editor evidence is captured.
