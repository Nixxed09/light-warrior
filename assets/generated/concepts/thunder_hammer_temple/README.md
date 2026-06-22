# Thunder Hammer Temple Concept Package

## Purpose

This package defines the first visual target for the Thunder Hammer temple: a landmark reward outside the safe sacred circle. It should prove the core hook visually before gameplay text explains it.

```text
Bravery expands the light.
```

## Current State

- Request JSON is ready for image-engine at `image-engine-request.json`.
- image-engine is running locally on `http://127.0.0.1:3100`.
- Generation is blocked until `TE_SERVICE_KEY` or an `x-service-key` is configured.
- The built-in image generation fallback did not expose a new saved output file in `.codex/generated_images`, so no concept bitmap has been accepted into this package yet.

## Production Route

Run through the TE image-engine MCP tool:

```text
generate_game_asset
```

Or call the local API:

```powershell
$key = $env:TE_SERVICE_KEY
$body = Get-Content -Raw .\assets\generated\concepts\thunder_hammer_temple\image-engine-request.json |
  ConvertFrom-Json |
  Select-Object -ExpandProperty request |
  ConvertTo-Json -Depth 8

Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:3100/api/ai-invoke/generate-game-asset `
  -Headers @{ "x-service-key" = $key } `
  -ContentType "application/json" `
  -Body $body
```

Save accepted outputs in this folder and record the source URL, provider, prompt, and review decision in `review-notes.md`.

## Acceptance

- The safe area reads as a warm gold circle.
- The outside area reads as violet danger, not just dark scenery.
- The temple is clearly outside the safe circle.
- The hammer reads as the reward target from a gameplay camera.
- The concept suggests a future UE5 blockout direction.
- No text/UI/watermark is present.
