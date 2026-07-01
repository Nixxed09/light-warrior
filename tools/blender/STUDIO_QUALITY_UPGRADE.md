# Studio Quality Upgrade Guide

How to take Light Warrior blockout meshes from "programmer art" to "looks like it came
from a game studio" — using only Blender Python + free resources.

## The Gap (what blockouts are missing)

| What studios do | What our blockouts have | What to add |
|---|---|---|
| Smooth organic forms | Sharp box/cylinder seams | Subdivision Surface modifier |
| PBR textured materials | Flat solid colors | Procedural node textures or PolyHaven PBR maps |
| Surface detail | None | Bump/normal maps from procedural noise |
| Studio presentation | Default grey viewport | Three-point lighting + dark backdrop |
| Edge definition | Hard faceted edges | Smooth shading + weighted normals |
| Energy/magic effects | Flat emissive color | Noise-driven varied emission + slight transparency |
| Scale reference | Floating in void | Ground plane or environment context |

## Upgrade Recipe (per asset)

### 1. Subdivision + Smooth Shading

```python
for obj in bpy.data.objects:
    if obj.name.startswith(PREFIX) and obj.type == 'MESH':
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.shade_smooth()
        sub = obj.modifiers.new("Subdivision", "SUBSURF")
        sub.levels = 2
        sub.render_levels = 3
```

This alone transforms the look. Bevels from the blockout become smooth curves.

### 2. Material Upgrade Patterns

**Metallic surfaces (gold, electrum, steel):**
- Noise Texture (scale 20-30) → Color Ramp (two tones of the metal) → Base Color
- Same Noise → rougher Color Ramp (0.1-0.4 range) → Roughness
- This creates natural variation: polished highlights vs worn areas

**Stone/organic surfaces (grip, architecture):**
- Voronoi Texture (scale 30-50) → Color variation
- Noise Texture → Roughness variation
- Voronoi Distance → Bump node (strength 0.1-0.2) → Normal input
- Bump sells the surface without adding geometry

**Emissive/energy (lightning core, eyes, tells):**
- Noise Texture (scale 5-10, high detail) → Color Ramp (dark base → bright tip)
- Same noise → Emission Color (varied intensity across surface)
- Emission Strength: 15-25 for cores, 8-12 for tells
- Alpha: 0.8-0.9 for energy volume transparency

**Enemy surfaces (shadow body):**
- Very dark base with subtle noise variation (keep it reading as void)
- Roughness: 0.7-0.9 (matte, absorbs light — opposite of hero's metallic shine)
- NO bump on shadow body (smooth = formless = unnatural)

### 3. Three-Point Studio Lighting

```python
# Key: warm gold, upper-right, area light, energy 150-250
# Fill: cool dawn-blue, left side, area light, energy 60-100
# Rim: white, behind/above, area light, energy 100-180
# Background: near-black violet (0.02, 0.01, 0.04)
```

Key light color should match the asset's palette:
- Hero assets: warm gold key `(1.0, 0.92, 0.75)`
- Enemy assets: cool violet key `(0.7, 0.6, 0.9)` — enemies should look cold
- Boss: desaturated cold key — the sovereign absorbs warmth

### 4. PolyHaven PBR Textures (for next-level surfaces)

Instead of procedural noise, download real PBR texture sets:

```python
# In a Blender MCP session:
search_polyhaven_assets(asset_type="textures", categories="metal")
download_polyhaven_asset(asset_id="hammered_gold", asset_type="textures", resolution="2k")
```

Good candidates for Light Warrior:
- `metal_plate` or `hammered_metal` — for gold/electrum surfaces
- `rock_face` or `sandstone` — for grip/architecture stone
- `marble` — for pale sacred architecture

Requires UV unwrapping first (procedural textures skip this step).

### 5. UV Unwrapping (when using image textures)

```python
bpy.context.view_layer.objects.active = obj
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.uv.smart_project(angle_limit=1.15, island_margin=0.02)
bpy.ops.object.mode_set(mode='OBJECT')
```

Smart UV Project works well for geometric/stylized assets. Only needed when using image-
based textures, not for procedural noise-based materials.

## Quality Tiers

| Tier | Time per asset | What you get | When to use |
|---|---|---|---|
| **Blockout** | 5 min | Silhouette + flat color | Proving gameplay, layout, scale |
| **Polished** | 15 min | Subdivision + procedural PBR + lighting | Presentations, vertical slice, concept validation |
| **Production** | 1-2 hr | PolyHaven textures + hand-tuned UVs + LODs + collision | Ship-ready engine assets |
| **Hero** | 2-4 hr | Sculpted detail + custom textures + animation-ready topology | Key marketing/trailer shots |

For Light Warrior's current stage (vertical slice), **Polished** tier is the target. Move
to Production when preparing UE5 import for the playable build.

## Automation

The upgrade process can be scripted just like the builds. A future
`upgrade_to_polished.py` would:
1. Load a blockout `.blend`
2. Apply subdivision + smooth
3. Swap materials with PBR versions
4. Set up studio lighting
5. Render a preview
6. Export upgraded FBX

This keeps the blockout scripts clean (geometry only) and separates the polish pass.
