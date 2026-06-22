# Light Warrior Game Spec

## GamesOS Position

Light Warrior is a bright arena action game where every hit pushes back darkness and every weapon temple changes how the player moves through the fight.

## Explicit Hook

```text
Bravery expands the light.
```

The player starts inside a sacred circle that is safe but limited. The strongest power sits outside that safety. To win, the player must leave the light, survive darkness long enough to claim a temple weapon, then return stronger and push the circle outward through combat.

This is the game. If a feature does not strengthen this risk/reward loop, it waits.

## Hook Rules

1. The sacred circle is sanctuary, progress meter, and territory.
2. Darkness creates pressure: health drain, enemy advantage, reduced recovery, or escalating threat.
3. Weapon temples are placed outside the current circle so power requires risk.
4. Successful attacks expand or stabilize the circle.
5. Mistakes shrink, corrupt, or threaten the circle.
6. The player must always understand why the circle changed.
7. Every arena should ask: "How brave are you willing to be for more power?"

## Full Game Vision

Light Warrior should become an epic third-person arena action game about reclaiming sacred ground from living darkness. The finished game should feel like a compact, highly readable action roguelite: strong movement, satisfying hits, dramatic light/dark contrast, weapon temples, escalating arenas, heroic music, and replayable mastery.

The ambition is not a flat canvas wave shooter. It is a modern arena combat game where the battlefield itself responds to player courage.

## Target Player And Platform

- Target player: action roguelite players, younger heroic fantasy fans, and players who like readable skill-based combat.
- Primary platform: PC/Steam.
- Secondary platform: itch.io and controller-first builds.

## Player Instinct

The player wants to feel brave: stepping out of safety, striking darkness, and seeing the world visibly brighten because they acted.

## Product Promise

Give the player a heroic arena where courage is mechanical. The light circle is both sanctuary and scoreboard. The player wins by leaving safety, mastering a weapon temple, and returning with enough power to reclaim the arena.

## Core Loop

```text
Fight from the light -> risk a run to a weapon temple -> unleash a power spike -> expand the sacred circle -> survive a stronger wave.
```

## Design Pillars

- Light is safety, power, progress, and readability.
- Darkness is pressure, risk, corruption, and enemy control.
- Bravery is the core verb: leaving safety should be scary, useful, and rewarding.
- The sacred circle is not a health zone; it is the visible state of the player's courage.
- Every weapon changes movement choices, not just damage numbers.
- Combat must be readable enough for younger players and deep enough for mastery.

## Preserve

- Warrior of light fantasy.
- Sacred ground defense.
- Shadow demons.
- Weapon temples.
- Heroic, readable combat.

## Replace

- Single-file HTML canvas architecture.
- Click-to-shoot flat combat.
- Too many weapons before one weapon feels great.
- Leaderboards and mobile support before core game feel.

## Pincus Pattern

### Proven Pattern

Use the arena readability of Hades, the pressure curve of Vampire Survivors, and the instant feedback of arcade brawlers.

### Quality References

- `Hades`: readable arenas, satisfying dash/attack rhythm, clear enemy tells.
- `Kena: Bridge of Spirits`: bright heroic fantasy tone and readable magical combat.
- `Risk of Rain 2`: escalating survival pressure and replayable combat mastery.

### Obvious Improvement

The sacred ground is a living combat system, not a static base. It expands when the player fights well and shrinks when darkness wins.

### New Hook

Bravery expands the light: weapon temples sit outside the safe circle, and the player must risk darkness to earn the power that reclaims the arena.

### Fast Signal

A 30-second clip where the player sees safety, chooses to leave it, reaches Thunder Hammer, slams demons, and expands the light circle enough to feel the arena being reclaimed.

## First 10 Seconds

```yaml
player_orientation: "The player stands inside a glowing sacred circle surrounded by shadow."
first_action: "Dash and strike the first shadow demon."
immediate_feedback: "The demon bursts into light and the circle expands."
first_success_30s: "The player activates one weapon temple before the second wave."
first_mistake: "Standing outside the light drains health."
recovery: "Light trails guide the player back to safety."
bold_beat: "A temple weapon transforms the next attack into a huge light shockwave."
true_signal: "The player chooses to leave safety for a stronger weapon and understands that bravery expanded the light."
```

## Hook Implementation Requirements

- The temple must begin outside the sacred circle.
- The UI must show that the temple is valuable before the player reaches it.
- Darkness must create an immediate but fair cost.
- Thunder Hammer must make the risk feel obviously worth it.
- Circle expansion must be visible within one second of a successful brave action.
- The first playable must support at least one bad outcome: hiding in safety should not be enough to win.

## Engine

Unreal Engine 5 preferred.

Use UE5 if the target is the epic third-person arena fantasy: lighting, character animation, camera feel, VFX, Niagara, controller input, marketplace-compatible assets, and cinematic presentation matter here.

Godot 4 is acceptable only if we deliberately choose a smaller stylized 2.5D version. The spec target is UE5.

## Input, Camera, And Feel Targets

- Controller-first third-person movement.
- Dash must feel immediate, readable, and invulnerable only if explicitly designed that way.
- Camera should keep the player, nearest threat, and sacred circle boundary readable.
- Lock-on is optional; soft aim assist and generous melee arcs are preferred for accessibility.
- Enemy tells must be visible through VFX-heavy light/dark scenes.

## Vertical Slice Quality Target

One arena that could represent the finished game:

- third-person character movement with dash
- camera that stays readable under pressure
- sacred light circle expanding/shrinking
- one polished weapon temple
- two enemy types with readable tells
- final-direction lighting, VFX, impact audio, and music
- menu-to-arena-to-results flow
- screenshot and 30-second gameplay capture

## Staged Expansion Plan

1. First playable: movement, dash, light strike, sacred circle, two enemies.
2. Vertical slice: one polished arena with Thunder Hammer temple and 90-second wave.
3. Combat expansion: four weapon temples, enemy families, elite variants.
4. Progression: unlockable blessings, arena modifiers, mastery ranks.
5. Campaign/roguelite structure: multiple corrupted sanctuaries and boss encounters.
6. Launch candidate: controller polish, settings, save, performance, trailer, Steam page.

## First Playable

- One circular arena.
- Movement and dash.
- Basic light strike.
- One weapon temple: Thunder Hammer.
- Two enemy types.
- Sacred light circle expansion/shrink.
- One 90-second wave.
- Victory and failure states.

## First Playable Acceptance Criteria

- A fresh player understands the safe circle within 5 seconds.
- Dash, strike, and enemy burst feel responsive.
- The circle visibly expands from successful action.
- Leaving the circle creates understandable danger without feeling unfair.
- Thunder Hammer activation creates a clear power spike.
- The player can identify the temple as a tempting goal before leaving the circle.
- A player who never leaves safety should struggle or fail the wave.

## Out Of Scope Until First Playable Works

- Four temples.
- Bosses.
- High scores.
- Mobile controls.
- Analytics.
- Large weapon roster.

## Asset Quality Target

Bright heroic fantasy with strong silhouettes. Light must read as safety and power. Darkness must read as pressure. Weapon temples should be memorable landmarks, not generic pickups.

## Audio Target

Heroic percussion/choir/electronic hybrid. Every dash, hit, temple activation, circle expansion, and enemy burst needs strong feedback.

## Performance Target

60 FPS on midrange gaming PCs with 40 active enemies, VFX bursts, arena lighting, and controller input.

## UX, Save, And Settings Target

- Main menu, pause, restart run, return to hub/menu.
- Settings for controller remap, mouse sensitivity if supported, camera speed, VFX intensity, music, SFX, subtitles for barks/tutorial prompts.
- Save progression after arena completion.
- First playable can skip progression save, but not pause/restart.

## Accessibility Target

- Light/dark boundaries readable without relying only on brightness.
- Reduced screen shake and reduced flash options.
- UI scale and subtitle support.
- Difficulty tuning hooks for enemy speed, damage, and aim assist.

## True Signal

A target player voluntarily leaves the safe light circle to reach the temple weapon because the risk/reward feels exciting, then understands that brave action expanded the light.

## Playtest Questions

- Did the player understand that light was safety?
- Did they leave the circle by choice or only by accident?
- Did Thunder Hammer feel worth the risk?
- Did the player understand that bravery expanded the light?
- Did staying safe feel like a losing strategy?
- Were enemy attacks readable during VFX-heavy moments?
- Did the player ask what the next temple weapon does?

## Production Risks

- VFX can overwhelm readability.
- UE5 scope can balloon into cinematic polish before combat works.
- Sacred circle could become a passive health zone unless tied to aggressive play.
- Younger-friendly tone can conflict with satisfying combat if enemies lack impact.

## Quality Gates

### First Playable Gate

- Arena opens reliably.
- Movement, dash, strike, circle, and one weapon temple all work.
- First 10 seconds are captured.
- The captured clip proves the bravery loop: safety -> risk -> temple power -> light expansion.

### Vertical Slice Gate

- One arena has final-direction lighting, animation, VFX, audio, camera, and result flow.
- Fresh player learns the loop without developer explanation.
- Combat invites replay, not just completion.

### Launch Candidate Gate

- Multiple arenas and weapon temples.
- Save/progression, settings, controller polish, performance pass.
- Store trailer shows circle expansion and temple risk within first 10 seconds.

## First Asset Pack

- Light warrior.
- Shadow imp.
- Berserker demon.
- Sacred circle VFX.
- Thunder Hammer.
- Temple pedestal.
- Arena floor and corruption edge.
- Combat impact, dash, temple activation, and heroic loop.

## Build Tasks

1. Create UE5 project shell.
2. Build arena camera and movement.
3. Add dash and basic strike.
4. Add sacred circle radius logic.
5. Add one weapon temple.
6. Add two enemies.
7. Capture first 10 seconds.
8. Raise one arena to vertical-slice quality.
