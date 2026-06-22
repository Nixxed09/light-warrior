# AI Feature Contract - Luminous Witness

Runtime AI is disabled by default. Light Warrior must remain complete with authored fallback lines before model calls are enabled.

## Feature Summary

- AI feature id: `luminous_witness`
- Pattern: `state_aware_guide`, later `bounded_companion`
- Enabled now: `false`
- Runtime mode: `none` for first playable, `hybrid` only after evidence
- Player value: helps the player recognize that brave action expands the light without removing agency
- Where the player sees it: short temple-field barks, post-run recap, optional map/route nudges
- What it improves: first-session comprehension, emotional payoff, replay recognition

## State Inputs

Structured facts only:

- `current_objective`: survive, reach temple, restore circle, finish wave
- `player_status`: health band, inside/outside circle, thunder hammer active
- `circle_status`: current radius band, shrinking, expanding, under pressure
- `nearby_entities`: temple visible, enemy count band, elite present
- `recent_successes`: enemy defeated, temple activated, circle expanded
- `recent_failures`: stayed in circle too long, took darkness damage, missed temple window
- `progression_state`: first run, first temple activation, first victory, repeated failure

## Allowed Actions

The AI may only propose these verbs:

- `comment`: one short in-world observation
- `hint`: one hint from the authored hint ladder
- `warn`: a short danger cue when the player is missing a readable state
- `celebrate`: short line after brave action restores the field
- `mark_optional_location`: request a non-authoritative marker for a visible temple or route

## Forbidden Actions

- Move, attack, dodge, aim, or activate a temple for the player.
- Reveal hidden routes before the designed true-map gate.
- Invent canon, rewards, enemies, upgrades, or quest truth.
- Override combat tuning or enemy behavior.
- Use personal player data in prompts.
- Speak outside Light Warrior's Lila and Old Earth reality.

## Content Outputs

- One-sentence hint, 18 words max.
- Short in-character comment, 14 words max.
- Post-run recap line, 26 words max.
- Ambient bark, 10 words max.
- Structured intent:

```json
{
  "verb": "hint",
  "line": "The hammer node answers courage.",
  "target": "thunder_hammer_temple",
  "priority": "low"
}
```

## Model And Routing

- Default tier: `local_slm`
- Escalate to: `fast_cloud` only for post-run recap quality
- Frontier only for: none in first playable or vertical slice
- Voice: `audio_engine` later, `none` now
- ASR: `none`
- Max latency before fallback fires: 900 ms
- On timeout: `authored_fallback`

## Memory

- Scope: `per_session`
- Stores: `world_knowledge`, `run_summary`
- Backend: `summary`
- Retention: `session`
- Player can delete: true when persistent memory exists
- Summarize after turns: 8

## Determinism And Replay

- Effect logged for replay: `{ effect, cache_key, seed }`
- Multiplayer: host resolves AI, clients replay logged effect
- Cache by state signature: objective + circle band + player status + recent beat
- Fallback tables are deterministic and used for first playable

## Lila Alignment

- Hand of Logos laws: Resonance, Poles, Consent, Mirrors, Arising, Remembering
- Plane(s): Aetheric with Mental and Source glimpses
- Teaching dial: Named, but combat stays mostly wordless
- Behavior: names the law only after the player feels it fire
- Canon voice: in-world, direct, with no machine-self references

## Fallback

First playable authored lines:

- Hint: `The temple waits beyond safety.`
- Warning: `The dark field is taking ground.`
- Celebration: `Bravery widened the circle.`
- Temple activation: `The hammer node remembers you.`
- Victory: `This node burns again.`
- Failure: `The field is not lost. Rise and reclaim it.`

Offline behavior: authored fallback only.

Error behavior: no line if fallback key is missing; never block play.

## Cost Budget

- Max generations per session: 12 after enabled
- Cooldown seconds: 20 for hints, 6 for barks
- Cache strategy: state signature and line type
- Max prompt size: 2 KB
- Max response size: 80 tokens
- Local/cloud provider: gateway routed, no provider calls from game
- Scale estimate at 1,000 players: local or cached by default
- Scale estimate at 10,000 players: local/cached, recap only cloud
- Scale estimate at 100,000 players: keep disabled unless telemetry proves cost and value

## Safety And Disclosure

- Disclosure required: true when runtime AI is enabled
- Store disclosure notes: verify current platform policy before enabling
- Kid/family safety notes: no open-ended chat, no personal data, no player profiling beyond game state
- Moderation/filtering: gateway drops out-of-canon or out-of-schema output to fallback
- Logging policy: log validated effects, cache keys, and fallback rate; no raw personal data
- Delete/correct memory policy: required before persistent memory
- Adult/studio approval required before enabling runtime AI: true

## QA And Evidence

- The game works with runtime AI disabled.
- Authored fallback covers the first playable.
- Playtest with AI off: player understands safety, temple risk, circle expansion.
- Playtest with AI on: player understands the same faster or feels stronger recognition.
- Pass only if players retain agency and do not wait for the guide.
- Fail if players describe the feature as a hint bot or if it distracts from combat.

## Ship Decision

- Keep disabled for first playable.
- Enable for a test branch after UE5 first playable evidence exists.
- Remove if authored feedback communicates the law better than runtime AI.
