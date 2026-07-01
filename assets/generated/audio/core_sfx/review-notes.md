# audio.core_sfx Review Notes

Generated: 2026-06-23T03:18:50.0394275Z

This package wires the Light Warrior gameplay event vocabulary to actual WAV files under `assets/generated/audio/core_sfx`.

Current source: audio-engine-with-local-wav-fallback

Events:

- Dash: `assets/generated/audio/core_sfx/dash_shimmer.wav`
- LightStrike: `assets/generated/audio/core_sfx/light_strike.wav`
- ShadowDissolve: `assets/generated/audio/core_sfx/shadow_dissolve.wav`
- CircleExpand: `assets/generated/audio/core_sfx/circle_expansion.wav`
- TempleActivate: `assets/generated/audio/core_sfx/temple_activation.wav`
- HammerSlam: `assets/generated/audio/core_sfx/hammer_slam.wav`

Production gate:

- Import the WAVs into `/Game/LightWarrior/Audio/CoreSfx`.
- Replace procedural tones only after UE5 capture evidence proves the generated cues work in context.
- Keep the event names stable: Dash, LightStrike, ShadowDissolve, CircleExpand, TempleActivate, HammerSlam.
