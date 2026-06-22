# Lila Gameplay Contract - Light Warrior

This contract deepens Lila inside Light Warrior while preserving the main idea:

```text
Bravery expands the light.
```

The player remains a warrior in a readable UE5 arena. Lila supplies the laws the arena obeys.

## Design Spine

Light Warrior is an action game first. Every Lila layer must improve one of these:

- clearer combat reads
- stronger risk/reward
- more meaningful circle expansion
- better temple temptation
- more memorable post-run progression

If a Lila feature slows combat, replaces player action, or turns the game into explanation, it does not belong in the first playable.

## Law To Mechanic Mapping

| Lila law | Combat expression | Progression expression | AI/Witness expression |
|---|---|---|---|
| Resonance | Brave actions raise the field: circle expands, VFX warms, tells sharpen. | Restored nodes unlock stronger field tuning and temple routes. | Names field changes after the player causes them. |
| Poles | Shadow enemies convert into light bursts instead of only dying. | Later upgrades shift corruption into usable charge. | Celebrates conversion, never vengeance. |
| Consent | Darkness takes ground where the player abandons the field; action withdraws that grip. | Route locks open when the player restores node authority. | Warns when pressure is taking ground. |
| Mirrors | The circle is a mirror of the player's courage and clarity. | Recaps show how player behavior changed the node. | Reflects player behavior in short, direct lines. |
| Arising | Enemy variants have visible sustaining conditions: spawner, tether, suppression field. | Advanced play removes causes instead of only clearing bodies. | Points to visible conditions, not hidden solutions. |
| Remembering | Temple activation and victory reveal that the node was always alive. | True-map layers and law names unlock after felt mastery. | Names the law only after the player has felt it fire. |

## First Playable Contract

The first playable may use only these Lila mechanics:

- Circle expansion from brave combat success.
- Circle shrink from dark pressure and passive waiting.
- Thunder Hammer activation outside safety.
- Enemy burst as corruption dissolving into light.
- Authored Luminous Witness fallback lines.
- Optional named-law UI after a run, not during the first combat seconds.

The first playable must not require runtime AI, dialogue trees, lore menus, moral-choice screens, or abstract meters.

## Octave Contract

Octave is a perception layer, not XP.

| Octave band | What changes in play | How it is earned |
|---|---|---|
| Asleep | Low route visibility, harsh darkness, minimal node geometry. | Starting state. |
| Brave | Temple line glows, enemy tells sharpen, circle pulse becomes clearer. | Leaving safety and surviving. |
| Clear | Resonance seams and route hints become readable. | Restoring the circle through combat and temple use. |
| Radiant | True-map overlays, hidden route gates, Source flashes. | Victory, high-risk restoration, later campaign mastery. |

For the vertical slice, Octave can be implemented as authored VFX/material states driven by run events.

## Luminous Witness Contract

The Luminous Witness is the temple field, not a separate personality. It exists to make recognition land at the right moment.

It speaks only when one of these beats has occurred:

- player stayed too long in safety and pressure is rising
- player first leaves the circle
- player first takes darkness damage
- player activates Thunder Hammer
- player expands the circle through combat
- player wins the node
- player fails and restarts

It must remain sparse. If players wait for it, it is too loud.

## Future Runtime AI Contract

Runtime AI can be enabled only after authored fallback proves useful. The model reads structured state and proposes one of the approved verbs. The game validates the verb and applies only a logged effect.

The first runtime test should be A-B:

- A: authored fallback only
- B: same fallback plus generated wording from the same allowed beat

Pass condition: players understand bravery, temple risk, and circle expansion faster without feeling guided through combat.

## What Not To Build Yet

- Open-ended chat.
- A companion that follows the player.
- AI combat tactics.
- Law lectures during combat.
- Procedural quests.
- Source-plane endgame content.

Those can come after the UE5 arena proves the core action loop.
