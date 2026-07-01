# Blender-to-UE5 Asset Pipeline

Repeatable pipeline for creating game assets in Blender via MCP, exporting FBX, importing into UE5 headlessly, and placing them in levels.

## Prerequisites

| Tool | Version | Location |
|------|---------|----------|
| Blender LTS | 4.5.5+ | System install (must be on PATH or launched manually) |
| UE5 | 5.8 | `D:\Games\UE_5.8` |
| Python | 3.10+ | System install (for pip) |
| blender-mcp | latest | `pip install blender-mcp` (Python package + Blender addon) |
| Claude Code | latest | CLI with MCP support |

### One-Time Setup

**1. Install blender-mcp Python package:**

```powershell
python -m pip install blender-mcp
```

**2. Enable the Blender addon:**

- Open Blender > Edit > Preferences > Add-ons
- Search for "Blender MCP"
- Enable it
- In the addon preferences, click "Start MCP Server" (runs on port 9876)

**3. Register MCP server with Claude Code:**

```powershell
claude mcp add blender-mcp -- python -m blender_mcp.server
```

**4. Verify connection:**

Run `/mcp` in Claude Code. You should see `blender-mcp` listed with tools like `execute_blender_code`, `get_scene_info`, `get_viewport_screenshot`.

## Pipeline Stages

```
planned ──▶ generated_candidate ──▶ imported ──▶ approved
  │              │                      │            │
  │              │                      │            └─ Visual review in editor passed
  │              │                      └─ .uasset files exist in Content/
  │              └─ .blend + .fbx + preview.png + metadata exist
  └─ Entry in ASSET_MANIFEST.json only
```

## Stage 1: Create Asset in Blender

Use the `mcp__blender-mcp__execute_blender_code` tool. Each asset follows this pattern:

### 1a. Clear scene and set up

```python
import bpy
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.context.scene.unit_settings.system = 'METRIC'
bpy.context.scene.unit_settings.scale_length = 1.0
```

### 1b. Build meshes

Create meshes using Blender Python (`bpy.ops.mesh.primitive_*`, bmesh, etc.). Follow these naming conventions:

- Static meshes: `SM_{AssetName}_{Part}` (e.g., `SM_LW_ArenaStone_Base`)
- Materials: `M_{MaterialName}` (e.g., `M_SacredGold`, `M_VioletCorruption`)

Adhere to `STYLE_GUIDE.md` color language:
- Hero: white-gold (`#FFF5D4`), dawn-blue (`#4A90D9`)
- Enemies: violet-black (`#1A0A2E`), red-violet attack tell (`#FF2060`)
- Sacred/power: sacred gold (`#FFD700`), pale stone (`#E8DCC8`)

### 1c. Assign materials

```python
mat = bpy.data.materials.new(name="M_SacredGold")
mat.use_nodes = True
bsdf = mat.node_tree.nodes["Principled BSDF"]
bsdf.inputs["Base Color"].default_value = (1.0, 0.843, 0.0, 1.0)
bsdf.inputs["Metallic"].default_value = 0.9
bsdf.inputs["Roughness"].default_value = 0.3
obj.data.materials.append(mat)
```

### 1d. Render preview

```python
bpy.context.scene.render.engine = 'BLENDER_EEVEE_NEXT'
bpy.context.scene.render.resolution_x = 1280
bpy.context.scene.render.resolution_y = 720
# Set up camera aimed at asset center
# ...
bpy.ops.render.render(write_still=True)
```

### 1e. Save .blend

```python
bpy.ops.wm.save_as_mainfile(filepath="D:/Phoenix/nix-code/games/light-warrior/assets/generated/models/{name}/source.blend")
```

### 1f. Export FBX

```python
bpy.ops.export_scene.fbx(
    filepath="D:/Phoenix/nix-code/games/light-warrior/assets/generated/models/{name}/SM_{Name}_LOD0.fbx",
    use_selection=False,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Y',
    axis_up='Z',
    mesh_smooth_type='FACE',
    use_mesh_modifiers=True,
    add_leaf_bones=False,
)
```

### 1g. Write metadata

Create `asset_metadata.json` with:

```json
{
  "asset_id": "category.name",
  "name": "Display Name",
  "status": "generated_candidate",
  "created": "2026-06-22",
  "tool": "blender-mcp via Claude Code",
  "blender_version": "4.5.5",
  "source_file": "source.blend",
  "exports": [
    {
      "file": "SM_{Name}_LOD0.fbx",
      "format": "FBX",
      "target_engine": "ue5",
      "file_size_bytes": 0
    }
  ],
  "preview": "preview.png",
  "meshes": ["SM_{Name}_{Part}", "..."],
  "materials": ["M_{MaterialName}", "..."],
  "dimensions": { "height_m": 1.8 },
  "style_guide_alignment": { "silhouette": "...", "colors": "..." },
  "next_gate": "UE5 import, scale review, Play-In-Editor evidence"
}
```

### Output file structure

```
assets/generated/models/{name}/
  source.blend           # Blender project (editable source of truth)
  SM_{Name}_LOD0.fbx     # FBX export for UE5
  preview.png            # EEVEE render preview
  asset_metadata.json    # Structured metadata
```

### Update ASSET_MANIFEST.json

Set the asset's `"status"` to `"generated_candidate"` and add `"source": "assets/generated/models/{name}"`.

## Stage 2: Import into UE5

### Import script: `tools/ue-import-assets.py`

Add a new entry to the `IMPORTS` list:

```python
{
    "fbx": os.path.join(MODELS_DIR, "{name}", "SM_{Name}_LOD0.fbx"),
    "dest": "/Game/LightWarrior/Meshes/{Category}",
    "name": "SM_{Name}",
},
```

### Run the import headlessly

```powershell
$ue = "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$proj = "D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject"
$script = "D:\Phoenix\nix-code\games\light-warrior\tools\ue-import-assets.py"
& $ue $proj "-ExecutePythonScript=$script" -unattended -nopause -nosplash
```

### UE5 content paths

| Category | UE5 Path |
|----------|----------|
| Arena | `/Game/LightWarrior/Meshes/Arena` |
| Characters | `/Game/LightWarrior/Meshes/Characters` |
| Enemies | `/Game/LightWarrior/Meshes/Enemies` |
| Weapons | `/Game/LightWarrior/Meshes/Weapons` |
| VFX | `/Game/LightWarrior/Meshes/VFX` |

### Verify the import

Check for `.uasset` files:

```powershell
Get-ChildItem -Recurse "engine\Content\LightWarrior\Meshes" -Filter "*.uasset" | Measure-Object
```

Check the UE5 log for Python output (stdout only shows platform validation):

```powershell
Select-String -Path "engine\Saved\Logs\LightWarrior.log" -Pattern "Import|Python|Error" | Select-Object -Last 20
```

### Update ASSET_MANIFEST.json

Set the asset's `"status"` to `"imported"` and add `"ue5_path"`.

### Re-importing

The import script uses `replace_existing=True`. Just re-run the same command after updating the FBX. Existing `.uasset` files will be overwritten.

## Stage 3: Place in Level

### Placement script: `tools/ue-place-assets.py`

Uses `EditorLevelLibrary` to create a map and spawn all meshes.

```powershell
$ue = "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$proj = "D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject"
$script = "D:\Phoenix\nix-code\games\light-warrior\tools\ue-place-assets.py"
& $ue $proj "-ExecutePythonScript=$script" -unattended -nopause -nosplash
```

This creates `AssetShowcase.umap` at `/Game/LightWarrior/Maps/AssetShowcase`.

## Stage 4: Visual Review in Editor

```powershell
$ue = "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$proj = "D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject"
& $ue $proj "/Game/LightWarrior/Maps/AssetShowcase"
```

Or use the game loop for automated evidence capture:

```powershell
.\tools\ue-loop.ps1 -Scenario asset-showcase
```

### Update ASSET_MANIFEST.json

After visual review passes, set `"status"` to `"approved"`.

## Quality Gates

### At generated_candidate

- [ ] Mesh names follow `SM_{Prefix}_{Part}` convention
- [ ] Materials follow `M_{Name}` convention
- [ ] Colors match `STYLE_GUIDE.md` palette
- [ ] Preview render shows readable silhouette
- [ ] FBX exports without errors
- [ ] `asset_metadata.json` is complete
- [ ] File size is reasonable for the asset type

### At imported

- [ ] `.uasset` files exist in the correct Content folder
- [ ] UE5 log shows no import errors
- [ ] Materials imported (even if placeholder — UE5 materials need separate authoring)
- [ ] Collision generated

### At approved

- [ ] Asset is visible in the UE5 editor viewport
- [ ] Scale is correct relative to other game assets
- [ ] No z-fighting or mesh overlap issues
- [ ] Screenshot or video evidence captured to `captures/`

## Adding a New Asset

1. Add an entry to `ASSET_MANIFEST.json` with `"status": "planned"`
2. Build the asset in Blender via MCP (follow Stage 1)
3. Add an entry to the `IMPORTS` list in `tools/ue-import-assets.py`
4. Run the import (Stage 2)
5. Re-run placement if you want it in the showcase level (Stage 3)
6. Review in editor (Stage 4)

## Known Issues

| Issue | Workaround |
|-------|------------|
| UE5 headless stdout only shows platform SDK validation | Check the log file at `engine/Saved/Logs/*.log` for actual Python output |
| EEVEE shader compilation delays viewport screenshots | Wait a few seconds after switching to Rendered mode before capturing |
| `-nullrhi` may prevent some editor subsystems from loading | Use headless without `-nullrhi` for placement scripts that need `EditorLevelLibrary` |
| Blender materials don't transfer perfectly to UE5 | Materials import as basic placeholders; author production materials in UE5 Material Editor |
| FBX axis conversion can flip models | Use `axis_forward='-Y', axis_up='Z'` in the FBX export call (matches UE5 default import) |
| `save_current_level` deprecation warning | Harmless in UE 5.8; future versions should use `LevelEditorSubsystem` |

## Hyper3D Rodin (AI 3D Generation)

Blender MCP includes Hyper3D Rodin integration for generating 3D meshes from text prompts or concept art images. Uses fal.ai as the inference backend.

### Setup

1. **fal.ai key**: Add your key in the Blender MCP panel (3D Viewport sidebar N → BlenderMCP → check "Use Hyper3D Rodin" → select FAL_AI mode → paste key)
2. The key persists in addon preferences, but the checkbox resets on Blender restart — re-check it each session

### Workflow (via MCP tools — preferred)

Claude agents use the MCP tools directly. No manual Python needed:

```
1. generate_hyper3d_model_via_text(prompt, bbox_condition)  → returns request_id
2. poll_rodin_job_status(request_id)                        → poll until COMPLETED
3. import_generated_asset(name, request_id)                 → imports into Blender
4. Scale, orient, reassign materials, export FBX
```

For concept art input: `generate_hyper3d_model_via_images(image_urls, bbox_condition)`

### What works well vs. what doesn't

| Asset type | Quality | Notes |
|------------|---------|-------|
| Props (weapons, pickups, crystals) | Great | Clean shapes, good materials |
| Environment pieces (pillars, altars) | Great | Detailed surface geometry |
| Organic formations (crystal clusters) | Excellent | Best results — organic detail AI excels at |
| Full characters | Poor | Generates weapons/objects instead. Use procedural Python or image-to-3D with concept art |

### Post-import checklist

- **Orientation**: Models may arrive sideways (tallest axis on Y). Rotate 90° on X to stand upright
- **Scale**: Models are normalized (~1m). Scale to game units (1 BU = 1m)
- **Materials**: Single "model" material with baked texture. For style guide compliance, reassign with procedural PBR materials matching `STYLE_GUIDE.md`
- **Vertex count**: ~15k-20k per model. May need decimation for LODs
- **Grounding**: After scaling, offset Z so `min_z = 0` (stands on ground plane)

### Batch generation

Multiple generations can run in parallel on fal.ai — fire off 4+ prompts simultaneously, poll all request_ids, import as they complete. ~25-55 seconds per model.

## Parallel Blender Instances

To run multiple Blender MCPs simultaneously (e.g., building two assets at once):

```powershell
$env:BLENDER_PORT = "9877"
blender --python-expr "import blender_mcp; blender_mcp.start_server(port=9877)"
```

Then register a second MCP server with Claude Code on that port.

## Reference Files

- `STYLE_GUIDE.md` — color language, shape language, material naming
- `ASSET_MANIFEST.json` — master asset registry with status tracking
- `tools/ue-import-assets.py` — headless FBX import script
- `tools/ue-place-assets.py` — headless level placement script
- `tools/ue-loop.ps1` — build + launch + capture + review loop
