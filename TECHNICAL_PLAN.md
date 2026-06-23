# Technical Plan

## Engine

Unreal Engine 5.

Reason:

- Light Warrior targets third-person 3D action, controller feel, cinematic lighting, strong VFX, and marketplace-compatible asset production.
- The earlier Vite/Three.js prototype remains as a legacy reference only.
- Godot 4 is a fallback only if the project is deliberately rescoped to a smaller stylized 2.5D version.

Transition authority: `docs/ENGINE_TRANSITION_THREEJS_TO_UE5.md`.

## Architecture

- Third-person player controller.
- Arena manager.
- Sacred circle territory system.
- Enemy spawn/wave controller.
- Weapon temple interaction.
- Run-state timer and result flow.
- Old Earth world-plane and true-map data hooks.
- Lila alignment hooks from `LILA_ALIGNMENT.md`.
- Optional AI-infusion hooks for the disabled-by-default Luminous Witness.
- Combat feedback and VFX layer.
- Theme alignment data hooks from `THEME_ALIGNMENT.md` and `GAME_MANIFEST.json`.

## Theme Implementation Hooks

Use `THEME_ALIGNMENT.md` when naming or tuning Blueprint classes, Data Assets, enemies, weapons, and level states.

- `BP_LightWarriorCharacter` represents human agency channeling luminous guardian power.
- `BP_SacredCircle` represents the covenant field, sanctuary, and reclaimed territory.
- `BP_ThunderHammerTemple` represents sacred technology that requires brave exposure.
- Shadow enemies represent dark influence trying to contain, distort, or retake territory.
- Reclaimed level states should move from corrupted dark surfaces to restored light, visible seams, and activated temple geometry.
- The first arena should be authored as one reclaimed node on a huge flat Old Earth plane. Use horizon haze, glare, storm walls, mountain silhouettes, and firmament distortion to imply unreachable scale without showing a world edge.
- Maps, route gates, and node-line unlocks should be Data Asset friendly so future levels can expand through true-map discoveries.
- Lila laws should be Data Asset friendly: active laws, plane, teaching dial, and octave gates should be readable by UI, VFX, and future AI systems without hardcoding lore in prompts.
- Runtime AI must use structured state and bounded verbs only. Authored fallback ships first.
- Progression systems should read from `GAME_SYSTEMS_LILA_PROGRESSION.md`: resources, weapons, enemy families, laws, and node progression must keep cause and effect visible.

## UE5 First Playable Modules

### `BP_LightWarriorCharacter`

Responsibilities:

- Camera-relative movement.
- Dash with cooldown and tuning hooks.
- Basic light strike with generous melee arc.
- Health and outside-light damage handling.
- Fall death when the player drops below the arena kill plane.
- Controller-first input mapping.

Acceptance:

- Movement and dash feel responsive in Play-In-Editor.
- Strike hits a nearby shadow enemy with visible feedback.
- Falling off the arena defeats the player and triggers a run restart.

### `BP_SacredCircle`

Responsibilities:

- Own current radius, min radius, max radius, shrink rate, expansion events.
- Render boundary as readable light/dark VFX.
- Expose `ExpandFromCombat(amount)` and `ApplyDarknessPressure(amount)`.
- Signal when the player is outside safety.

Acceptance:

- Circle expands on enemy defeat.
- Circle shrinks under pressure.
- Player damage outside the circle is readable.

### `BP_ThunderHammerTemple`

Responsibilities:

- Landmark visible from spawn.
- Activation when player enters temple zone.
- Temporary hammer state/timer.
- Shockwave attack event.

Acceptance:

- Player chooses to leave safety to reach it.
- Activation produces an obvious power spike.

### `BP_ShadowImp`

Responsibilities:

- Simple approach behavior.
- Low health.
- Clear dissolve/burst on defeat.

Acceptance:

- Readable weak enemy used in first 10 seconds.

### `BP_BerserkerDemon`

Responsibilities:

- Slower approach.
- Charge windup tell.
- Higher health and damage.

Acceptance:

- Player can see and dodge charge intent.

### `BP_ArenaGameMode`

Responsibilities:

- Start run.
- Spawn first wave.
- Track 90-second survival timer.
- Trigger victory/failure result.
- Broadcast run state and remaining time to UI.
- Restart the level after failure when `bRestartAfterFailure` is enabled.

Acceptance:

- Menu-to-arena-to-result loop exists before vertical-slice polish.
- Victory triggers when the timer ends and failure triggers when the hero reaches zero health.
- Failure reloads the current level after a short delay so the player starts over cleanly.

### `BP_OldEarthNodeData`

Responsibilities:

- Identify the arena as a named Old Earth node.
- Store node role, linked routes, temple key requirements, and visible horizon masks.
- Provide designer-facing fields for false-map limits and true-map unlocks.

Acceptance:

- First arena has a node identity before vertical-slice art.
- Future arenas can be connected through restored routes without changing combat code.

### `BP_LilaAlignmentData`

Responsibilities:

- Store active laws, plane, teaching dial, and octave bands for the arena.
- Expose law names to UI/reward systems when the game earns named teaching.
- Give future AI prompts a validated alignment source instead of ad hoc text.

Acceptance:

- First arena declares Resonance, Poles, Consent, Mirrors, Arising, and Remembering.
- Combat can remain wordless while progression can name discovered laws.

Native scaffold:

- `ULilaAlignmentData`
- `ELightWarriorLilaLaw`
- `ELightWarriorOctaveBand`

### `BP_LuminousWitness`

Responsibilities:

- Start as authored fallback only.
- Read structured state: objective, player status, circle status, temple visibility, recent success/failure.
- Emit only approved verbs: `comment`, `hint`, `warn`, `celebrate`, `mark_optional_location`.
- Never move, fight, activate, reward, or solve for the player.

Acceptance:

- First playable works with this system absent.
- If enabled later, A-B playtest proves faster comprehension or stronger emotional recognition.

Native scaffold:

- `ULuminousWitnessFallbackData`
- `ELuminousWitnessBeat`
- `ELuminousWitnessVerb`

### Progression Resources

First playable:

- `Light`: visible field gain from enemy bursts and temple activation.
- `Courage`: earned by leaving safety and reaching the temple.

Later:

- `ResonanceShards`: temple tuning.
- `TrueMapFragments`: route/node progression.
- `ShadowDebt`: delayed pressure from passive play or abandoned breaches.
- `AetherCharge`: advanced timing and field-energy spend.

Acceptance:

- Every resource has a visible cause and a visible effect.
- No resource exists only as a menu currency.

Native scaffold:

- `ULightWarriorProgressionComponent`
- `ELightWarriorResource`
- `FLightWarriorResourceChanged`

Current wiring:

- Leaving the sacred circle for an excursion records `Courage`.
- Successful light strikes record `Light`.
- Thunder Hammer activation records both `Courage` and `Light`.

### `ALightWarriorHUD`

Responsibilities:

- Show run state, objective, timer, purified wells, health, active well progress, Light, Courage, optional Shadow Debt, and active Thunder Hammer time.
- Keep the objective readable at a glance without covering combat.
- Keep first-playable controls visible until a proper menu/tutorial pass exists.
- Use Canvas HUD until UMG art direction is ready.

Acceptance:

- HUD compiles with the UE5 module.
- The HUD reinforces safety, risk, restoration, and resource cause/effect.
- The HUD remains sparse enough for combat readability.

## Data And Tuning

Create Data Assets or Blueprint variables for:

- Player move speed.
- Dash distance, cooldown, invulnerability flag.
- Strike range, arc, cooldown, damage.
- Circle start/min/max radius.
- Circle shrink and expansion values.
- Outside-light damage per second.
- Enemy speed, health, damage, tell duration.
- Temple activation duration and shockwave radius.

## Input

Required:

- Gamepad left stick movement.
- Gamepad face button attack.
- Gamepad shoulder/face button dash.
- Keyboard WASD movement.
- Keyboard Shift dash.
- Keyboard/Mouse or controller camera orbit.

Optional after first playable:

- Soft lock-on.
- Aim assist.
- Remappable controls screen.

## Art And VFX Blockout

First playable can use blockout assets, but they must establish:

- Hero silhouette.
- Shadow imp silhouette.
- Berserker silhouette.
- Sacred circle boundary.
- Temple landmark.
- Hammer activation VFX.
- Enemy burst VFX.

Blender is available for local blockouts at:

```text
C:\Program Files\Blender Foundation\Blender 4.5\blender.exe
```

Media engine integration is documented in `docs/MEDIA_ENGINE_INTEGRATION.md`.
The current Light Warrior AI asset queue is documented in `docs/AI_ASSET_PRODUCTION_QUEUE.md`.

Use:

- `image-engine` for concept images and visual reference frames.
- `video-engine` for Blender MCP, game asset generation, FBX export, metadata, validation, and later capture assembly.
- `audio-engine` for SFX, music loop, mastering, and audio manifests.

Use `docs/UE5_PROCEDURAL_SFX_FLOW.md` for the current first-pass runtime audio bridge. Procedural UE5 SFX prove event timing and feedback while `audio-engine` production packages are pending; final audio still comes from `audio-engine` and must be imported/proven in UE5.

For UE5 asset candidates, request `target_engine: "ue5"` from the video-engine game asset pipeline so exports use FBX and `SM_<name>_LOD#` naming.

## Audio Blockout

Required before vertical slice:

- Dash shimmer.
- Light strike.
- Enemy burst.
- Circle expansion pulse.
- Outside-light damage warning.
- Temple activation.
- Hammer slam.
- Simple heroic loop or ambient bed.

Audio can be placeholder for first playable, but silence is not acceptable for vertical slice.

## Evidence Gates

### UE5 Shell Gate

Required before `engine.ue5_shell` can move out of `blocked`:

- `engine/LightWarrior.uproject` exists.
- Unreal opens the project.
- Empty arena level runs in Play-In-Editor.
- Screenshot exists in `captures/`.
- Engine version is logged in `PRODUCTION_LOG.md`.

### First Playable Gate

Required:

- Play-In-Editor 10-second capture.
- Movement, dash, strike, enemy burst, circle expansion.
- Temple visible within first 10 seconds.
- 90-second run timer reaches victory, and zero health reaches failure.
- 30-second capture reaches or previews Thunder Hammer.
- Authored Luminous Witness fallback lines exist but runtime AI remains disabled.
- Task graph updates only after evidence exists.

### Vertical Slice Gate

Required:

- Final-direction lighting.
- VFX readability pass.
- Audio feedback.
- Menu-to-arena-to-result flow.
- Controller test.
- At least one observed playtest note.

## Verification

Use Unreal editor automation when available. Required evidence:

- Play-In-Editor run.
- Screenshot/video capture.
- Log review after every gameplay change.

Do not mark production gameplay tasks complete based only on the Three.js prototype.
