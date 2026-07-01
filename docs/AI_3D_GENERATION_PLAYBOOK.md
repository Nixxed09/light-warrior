# AI 3D Generation Playbook

How we generate production-quality 3D game assets from concept art using AI, and the lessons learned doing it.

## Working Pipeline (Proven 2026-06-23)

```
Concept Art (PNG) ──▶ fal.ai upload ──▶ Hyper3D Rodin ──▶ GLB download
     │                                                         │
     └─ OpenAI image gen                                       ▼
        or hand-drawn                                   Blender import
                                                              │
                                                     Scale + rename + smooth
                                                              │
                                                        FBX export ──▶ UE5
```

### What Works

| Step | Tool | Notes |
|------|------|-------|
| Concept art | OpenAI gpt-image-1 | `tools/generate-openai-image-assets.ps1`, key in user env `OPENAI_IMAGE_KEY` |
| Upload to CDN | `fal_client.upload_file()` | Returns hosted URL for Hyper3D input |
| 3D generation | Hyper3D Rodin via fal.ai | `fal-ai/hyper3d/rodin`, ~60-90s per model |
| Result fetch | Raw HTTP GET | Endpoint: `queue.fal.run/fal-ai/hyper3d-rodin/requests/{id}` |
| GLB download | `urllib.request.urlretrieve()` | From `result["model_mesh"]["url"]` |
| Blender import | `bpy.ops.import_scene.gltf()` | Via Blender MCP `execute_blender_code` |
| FBX export | `bpy.ops.export_scene.fbx()` | `axis_forward='-Y', axis_up='Z'` |
| UE5 import | `tools/ue-import-assets.py` | Headless via `UnrealEditor-Cmd.exe` |

### What Doesn't Work (Avoid These)

| Approach | Problem |
|----------|---------|
| `import_generated_asset` MCP tool | `'model_mesh'` key error — tool expects different response format for FAL_AI mode |
| `fal_client.result()` inside Blender | Returns full 3D model data inline (3MB+), overflows MCP output buffer |
| `input_image_paths` with FAL_AI mode | Converts to base64 data URLs which fal.ai rejects on result retrieval (422) |
| PowerShell here-strings with Python | Single quotes in Python code conflict with `@'...'@` syntax |
| `fal_client.result()` on old jobs | GET endpoint re-validates original input params — fails if input format was wrong |

## Secrets & Keys

| Key | Location | Purpose |
|-----|----------|---------|
| `FAL_KEY` | `D:\TE-Code\.claude\vault\MASTER_VAULT.env` | fal.ai API (Hyper3D Rodin) |
| `FAL_KEY` | Windows user environment variable | Same key, set for CLI tools |
| `FAL_KEY` | Blender addon preferences (`hyper3d_api_key`) | Same key, set in Blender UI |
| `OPENAI_IMAGE_KEY` | Windows user environment variable | OpenAI image generation |

To set user env vars:
```powershell
[System.Environment]::SetEnvironmentVariable("FAL_KEY", "<key>", "User")
```

## Prompts That Produced Good Results

### Concept Art Prompts (OpenAI gpt-image-1)

These prompts are stored in `assets/generated/internal-image-gen-batch-20260623.json`. Key patterns:

**Hero Character:**
> "Stylized game character concept art, front view T-pose, medieval knight with glowing golden armor, blue cape, warm white-gold palette (#FFF5D4, #4A90D9), clean silhouette suitable for 3D modeling, solid background, game-ready proportions"

**Shadow Enemy:**
> "Dark fantasy creature concept art, front view, hunched shadow imp with violet-black body (#1A0A2E), glowing red-violet attack indicators (#FF2060), menacing silhouette, oily dark surface, solid background, game character sheet style"

**Weapon:**
> "Fantasy weapon concept art, ornate war hammer with golden sacred runes, stone head with lightning energy core (#4A80FF), long wooden handle, front and side view, clean background, game asset style"

### Hyper3D Rodin Parameters

```python
{
    "input_image_urls": ["<fal-hosted-url>"],
    "bbox_condition": [w, d, h],  # proportional ratios
}
```

Bbox guidance:
- Human character: `[1, 1, 2]` (taller than wide)
- Small creature: `[1, 1, 1]` (roughly cubic)
- Weapon/long object: `[1, 1, 3]` (tall/long)
- Flat/wide object: `[2, 2, 1]`

### Results Quality

| Asset | Verts | Faces | Inference Time | Quality Notes |
|-------|-------|-------|----------------|---------------|
| Light Warrior | 21,280 | 33,652 | 72s | Good proportions, armor detail, cape, base plate |
| Shadow Imp | 20,380 | 31,998 | 92s | Menacing silhouette, dark tendrils, hunched posture |
| Thunder Hammer | 22,721 | 36,303 | 68s | Ornate head, rune engravings, proper weapon shape |

## Style Guidance for Light Warrior

### Color Language (from STYLE_GUIDE.md)

| Role | Color | Hex | Material Properties |
|------|-------|-----|-------------------|
| Hero Armor | White-gold | `#FFF5D4` | metallic=0.7, roughness=0.35 |
| Hero Fabric | Dawn-blue | `#4A90D9` | metallic=0.0, roughness=0.80 |
| Sacred Gold | Pure gold | `#FFD700` | metallic=0.9, roughness=0.25, emission strength=2.5 |
| Shadow Body | Violet-black | `#1A0A2E` | metallic=0.1, roughness=0.40, oily noise variation |
| Attack Tell | Red-violet | `#FF2060` | metallic=0.0, roughness=0.1, emission strength=6.0 |
| Lightning | Blue-white | `#4A80FF` | metallic=0.0, roughness=0.0, emission strength=8.0 |
| Pale Stone | Warm stone | `#E8DCC8` | metallic=0.0, roughness=0.70 |

### Shape Language

- **Hero**: upright, broad shoulders, strong silhouette, geometric armor plates
- **Enemies**: hunched, asymmetric, jagged edges, liquid/oily surfaces
- **Sacred objects**: circles, radial symmetry, clean geometric forms
- **Weapons**: heavy heads, long handles, visible energy cores

### Post-Generation Checklist

After importing an AI-generated mesh:

1. **Scale** to game units (hero=1.8m, imp=0.8m, hammer=1.2m)
2. **Rename** to `SM_LW_{Name}_Hyper3D`
3. **Smooth shade** all faces
4. **Subdivision** level 1 viewport / level 2 render on organic parts
5. **Remove base plate** if present (generated models often include one)
6. **Reassign materials** to match style guide palette (Hyper3D materials won't match)
7. **Check normals** — flip any inverted faces
8. **Export FBX** with `axis_forward='-Y', axis_up='Z'`
9. **Save .blend** as `hyper3d_source.blend` alongside the GLB

## Reproducible Script (Copy-Paste)

Run this inside Blender MCP `execute_blender_code`:

```python
import sys, os, json, urllib.request
sys.path.insert(0, "C:/Users/pberg/AppData/Roaming/Python/Python311/site-packages")
os.environ["FAL_KEY"] = "<your-fal-key>"
import fal_client

# 1. Upload concept art
url = fal_client.upload_file("path/to/concept.png")

# 2. Submit generation
handle = fal_client.submit("fal-ai/hyper3d/rodin", arguments={
    "input_image_urls": [url],
    "bbox_condition": [1, 1, 2],
})
print(f"Submitted: {handle.request_id}")

# 3. Poll until complete
import time
while True:
    status = fal_client.status("fal-ai/hyper3d/rodin", handle.request_id, with_logs=False)
    if "Completed" in str(status):
        break
    time.sleep(10)

# 4. Download GLB via raw HTTP (avoids MCP buffer overflow)
key = os.environ["FAL_KEY"]
result_url = f"https://queue.fal.run/fal-ai/hyper3d-rodin/requests/{handle.request_id}"
req = urllib.request.Request(result_url, headers={"Authorization": f"Key {key}"})
data = json.loads(urllib.request.urlopen(req).read())
glb_url = data["model_mesh"]["url"]

save_path = "path/to/output/model.glb"
os.makedirs(os.path.dirname(save_path), exist_ok=True)
urllib.request.urlretrieve(glb_url, save_path)

# 5. Import into Blender
import bpy
bpy.ops.import_scene.gltf(filepath=save_path)
```

## Quality Control Checklist

Before marking an AI-generated asset as production-ready:

- [ ] Silhouette reads correctly at game camera distance
- [ ] Poly count reasonable for real-time (target: 5K-30K per character)
- [ ] No floating geometry or mesh artifacts
- [ ] Materials reassigned to style guide palette
- [ ] Scale matches game units
- [ ] FBX imports clean into UE5 (no warnings)
- [ ] Looks correct in UE5 viewport with production lighting
- [ ] Screenshot evidence captured to `captures/`

## Iteration Log

| Date | Asset | Method | Result | Key Learning |
|------|-------|--------|--------|-------------|
| 2026-06-23 | All 6 | Blender MCP procedural | Geometric blockouts | Good for layout, not for production |
| 2026-06-23 | All 6 | Blender MCP + PBR materials | Better colors, still boxy | Material pass is high-impact but mesh quality is the bottleneck |
| 2026-06-23 | Hero, Imp, Hammer | Hyper3D Rodin (concept→3D) | Detailed meshes, 20-22K verts | Best quality so far — concept art quality drives mesh quality |
