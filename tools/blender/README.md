# Light Warrior Blender Build Scripts

Procedural Python scripts that build game assets in Blender via MCP. Each script is
idempotent (clears its own prior output, not the whole scene) and exports UE5-ready FBX.

## How To Run

### Via Blender MCP (live, driven by Claude)

```python
# In a Claude session with Blender MCP connected:
exec(open(r"D:\Phoenix\nix-code\games\light-warrior\tools\blender\build_thunder_hammer.py").read())
```

### Via Blender UI (manual)

1. Open Blender.
2. Switch to the **Scripting** tab (top bar).
3. Click **Open**, navigate to the script.
4. Click **Run Script** (play button).

### Via command line (headless, no GUI)

```powershell
& "C:\Program Files\Blender Foundation\Blender 4.5\blender.exe" --background --python .\tools\blender\build_thunder_hammer.py
```

## Scripts

| Script | Asset | Parts | Output |
|--------|-------|-------|--------|
| `build_thunder_hammer.py` | weapon.thunder_hammer | 16 | `assets/generated/models/thunder_hammer/` |
| `build_shadow_imp.py` | enemy.shadow_imp | 17 | `assets/generated/models/shadow_imp/` |
| `build_berserker.py` | enemy.berserker | 30 | `assets/generated/models/berserker/` |
| `build_corruption_sovereign.py` | boss.corruption_sovereign | 51 | `assets/generated/models/corruption_sovereign/` |

## Conventions

### Naming

- Object prefix: `SM_LW_<AssetName>_<Part>` (e.g. `SM_LW_ThunderHammer_Head`)
- Material prefix: `M_LW_<Name>` (e.g. `M_LW_Gold`, `M_LW_ShadowBody`)
- Root empty: `SM_LW_<AssetName>` — all parts parented under this for clean export.

### Scale

1 Blender unit = 1 meter. All assets built at real-world scale so they compose correctly
in UE5 without import rescaling.

### FBX Export Settings (UE5)

```python
bpy.ops.export_scene.fbx(
    filepath=fbx_path,
    use_selection=True,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Y',
    axis_up='Z',
    mesh_smooth_type='FACE',
    use_mesh_modifiers=True,
    add_leaf_bones=False
)
```

These settings match UE5's default FBX import. Do not change `axis_forward` or `axis_up`
unless the UE5 import transform is also adjusted.

### Color Palette (from STYLE_GUIDE.md)

**Hero palette:**
- Sacred gold: `(0.83, 0.60, 0.16)` — hero surfaces, rewards
- Electrum: `(0.74, 0.72, 0.55)` — trim, detail, striking faces
- Pale stone: `(0.78, 0.76, 0.70)` — grips, architecture
- Dawn-blue core: `(0.20, 0.55, 1.00)` — emissive lightning, power channels

**Enemy palette:**
- Shadow body: `(0.08, 0.03, 0.12)` — dark violet-black
- Shadow detail: `(0.15, 0.06, 0.22)` — slightly lighter violet
- Attack tell: `(0.85, 0.10, 0.35)` — red-violet, emissive 8.0+
- Enemy eyes: `(1.0, 0.15, 0.40)` — hot pink-red, emissive 12.0+

**Boss palette:**
- Void body: `(0.04, 0.01, 0.08)` — near-black
- Corrupt armor: `(0.12, 0.04, 0.18)` — deep violet, metallic 0.6
- Corrupt runes: `(0.60, 0.0, 0.80)` — violet emissive 10.0

Red is reserved for enemy damage feedback only — never on hero assets.

### Asset Evidence Chain

Every generated model must have:
1. Source `.blend` file
2. Exported `.fbx` for UE5
3. `asset_metadata.json` (see existing examples)
4. Preview screenshot (saved to the model dir or `captures/`)
5. Entry in `ASSET_MANIFEST.json`

Asset is not "done" until it's imported into UE5 and has Play-In-Editor evidence.

## Build Method Decision Guide

| Asset type | Recommended method | Why |
|---|---|---|
| Geometric props (weapons, pickups, architecture) | Procedural Python | Full control, version-controllable, matches readable style |
| Characters (player, enemies) | Procedural blockout, then iterate | Readable silhouettes from primitives, detail can be added |
| Organic/complex characters | AI gen (Hyper3D/Hunyuan) if enabled | Faster for organic detail, needs cleanup after |
| Environment lighting | PolyHaven HDRI download | Free, high-quality, instant |
| Reference/kitbash parts | Sketchfab search + download | Good for inspiration meshes, needs license check |
| Materials/textures | PolyHaven texture download | Free PBR textures, import directly |
| VFX (circles, particles) | Procedural Python | Clean geometric shapes, emissive materials |

## Studio Quality Upgrade

See `STUDIO_QUALITY_UPGRADE.md` for the full guide on taking blockouts to presentation
quality: subdivision smoothing, procedural PBR materials, studio lighting, PolyHaven
textures, and the four quality tiers (Blockout → Polished → Production → Hero).

## Parallel Blender Sessions

These scripts work in parallel Blender sessions (separate ports). See
`GamesOS/docs/MULTI_BLENDER_PARALLEL_SETUP.md` for setup. Each builder uses their own
port and saves to their own `.blend` — scripts export to the shared `assets/generated/`
tree.
