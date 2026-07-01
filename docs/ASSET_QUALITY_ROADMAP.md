# Asset Quality Roadmap

## Current State

All 6 core assets were built via Blender MCP (`execute_blender_code`) as geometric blockouts — cylinders, cubes, spheres, UV spheres arranged into recognizable shapes. Materials are flat Principled BSDF with `base_color` only. No roughness variation, no metallic maps, no emission, no procedural detail. The result reads as colored primitives, not game-ready art.

The concept art (in `assets/generated/concepts/`) shows the target: rich PBR surfaces, volumetric glow, subsurface lighting, intricate silhouettes, and atmospheric depth. The gap is large but closeable in tiers.

**Visual target:** Stylized action game (Hades, Dead Cells, Slay the Spire tier) — NOT photorealistic. Clean readable silhouettes with strong material identity and lighting-driven mood.

---

## Color Palette Reference

These are the canonical colors from `STYLE_GUIDE.md`, translated to exact values for both Blender (linear) and UE5 (sRGB hex for Material Instance parameters).

| Name | Role | sRGB Hex | Blender Linear RGB | UE5 Usage |
|------|------|----------|---------------------|-----------|
| Sacred Gold | Power, safety, reward | `#FFD700` | `(1.0, 0.68, 0.0)` | Emissive + base on circle, trims, weapon core |
| Warm Gold (light) | Gentle glow, restored stone | `#F5DEB3` | `(0.91, 0.76, 0.49)` | Base on temple stone, inlays |
| Dawn Blue | Hero fabric, sacred tech | `#4A90D9` | `(0.13, 0.34, 0.70)` | Hero cape, inner circle, energy lines |
| White Armor | Hero plate, clean surfaces | `#E8E0D0` | `(0.79, 0.73, 0.63)` | Hero chest, pauldrons, helmet |
| Pale Stone | Arena floor, temple walls | `#C8B89A` | `(0.57, 0.48, 0.33)` | Arena base, pedestals |
| Violet-Black | Corruption, enemy bodies | `#1A0A2E` | `(0.008, 0.003, 0.02)` | Enemy base, corruption floor |
| Deep Violet | Shadow energy, pressure | `#4B0082` | `(0.05, 0.0, 0.11)` | Enemy glow, corruption pulse |
| Red-Violet Tell | Attack warning, damage | `#C41E3A` | `(0.55, 0.01, 0.04)` | Attack tell orbs, damage cracks |
| Hero Skin | Exposed skin tone | `#D4A574` | `(0.64, 0.39, 0.18)` | Head, hands, neck |

**Known issue:** The current blockouts used approximate colors picked during construction. A material-correction pass using these exact values is the single highest-impact quality improvement.

---

## Tier 1: Material & Color Correction (Current Tools, No New Deps)

**What it gets us:** Assets that look like they belong in the same game. Proper PBR reads under UE5 lighting. Glowing elements actually glow. Metals look metallic. The style guide palette is enforced.

**Effort:** 1-2 sessions per asset. Can be done entirely via `mcp__blender-mcp__execute_blender_code`.

### 1a. Fix Base Colors

Re-assign every material's `Base Color` to the exact values in the palette table above. Current colors were eyeballed and are inconsistent.

```python
# Example: fix M_HeroArmor to correct White Armor
mat = bpy.data.materials["M_HeroArmor"]
bsdf = mat.node_tree.nodes["Principled BSDF"]
bsdf.inputs["Base Color"].default_value = (0.79, 0.73, 0.63, 1.0)
```

### 1b. Add PBR Properties

Every material currently has only `Base Color` set. Add:

| Material Type | Metallic | Roughness | Notes |
|---------------|----------|-----------|-------|
| Gold/Sacred | 0.9 | 0.25 | Polished sacred gold |
| Armor plate | 0.7 | 0.35 | Worn metal, not mirror |
| Fabric/Cape | 0.0 | 0.8 | Matte cloth |
| Skin | 0.0 | 0.5 | Plus Subsurface 0.3 |
| Stone | 0.0 | 0.7 | Rough, matte |
| Shadow body | 0.1 | 0.4 | Semi-glossy, oily |
| Corruption floor | 0.0 | 0.6 | Matte with wet spots |
| Energy/glow | 0.0 | 0.0 | Emission only |

```python
# Example: make gold trims actually metallic and shiny
mat = bpy.data.materials["M_HeroGoldGlow"]
bsdf = mat.node_tree.nodes["Principled BSDF"]
bsdf.inputs["Base Color"].default_value = (1.0, 0.68, 0.0, 1.0)
bsdf.inputs["Metallic"].default_value = 0.9
bsdf.inputs["Roughness"].default_value = 0.25
```

### 1c. Add Emission for Glow Elements

These should glow in both Blender and UE5:
- `M_HeroGoldGlow` — sacred gold emission (warm)
- `M_LightningCore` — blue-white lightning emission
- `M_AttackTell` — red-violet pulsing tell
- `M_BerserkerChargeTell` — red-violet charge glow
- `M_CircleGoldRing` — sacred circle gold emission
- `M_CircleExpansionPulse` — bright gold burst
- `M_CircleSafetyFill` — soft dawn-blue fill glow

```python
# Example: make lightning core glow
mat = bpy.data.materials["M_LightningCore"]
bsdf = mat.node_tree.nodes["Principled BSDF"]
bsdf.inputs["Emission Color"].default_value = (0.5, 0.7, 1.0, 1.0)
bsdf.inputs["Emission Strength"].default_value = 5.0
```

### 1d. Add Procedural Roughness Variation

Flat roughness makes everything look plastic. Add Noise Texture → ColorRamp → Roughness for organic variation:

```python
# Add roughness variation to stone
mat = bpy.data.materials["M_PaleStone"]
tree = mat.node_tree
bsdf = tree.nodes["Principled BSDF"]
tex_coord = tree.nodes.new("ShaderNodeTexCoord")
noise = tree.nodes.new("ShaderNodeTexNoise")
noise.inputs["Scale"].default_value = 8.0
noise.inputs["Detail"].default_value = 6.0
ramp = tree.nodes.new("ShaderNodeValToRGB")
ramp.color_ramp.elements[0].position = 0.4
ramp.color_ramp.elements[0].color = (0.5, 0.5, 0.5, 1)
ramp.color_ramp.elements[1].position = 0.6
ramp.color_ramp.elements[1].color = (0.8, 0.8, 0.8, 1)
tree.links.new(tex_coord.outputs["Object"], noise.inputs["Vector"])
tree.links.new(noise.outputs["Fac"], ramp.inputs["Fac"])
tree.links.new(ramp.outputs["Color"], bsdf.inputs["Roughness"])
```

### 1e. Geometry Quick Wins

- **Bevel modifier** on all hard-edge meshes (armor, weapons, stone). Even 1-2 segments at 0.01m catches light on edges and kills the "untextured cube" look.
- **Subdivision Surface** (level 1) on organic meshes (character torso, enemy bodies). Smooths cylinder-into-body shapes.
- **Smooth shading** — many meshes may still be flat-shaded. `bpy.ops.object.shade_smooth()` on everything, then use Auto Smooth for hard-edge control.

```python
# Add bevel + smooth to all armor pieces
for obj in bpy.data.objects:
    if obj.type == "MESH" and "SM_LW_" in obj.name:
        bpy.context.view_layer.objects.active = obj
        bevel = obj.modifiers.new("Bevel", "BEVEL")
        bevel.width = 0.005
        bevel.segments = 2
        bpy.ops.object.shade_smooth()
```

### 1f. UE5 Material Instances

After re-exporting corrected FBX, create UE5 Material Instances with proper parameters via the Python import script. The imported Blender materials become basic UE materials; layering Material Instances on top gives:
- Emissive channels for glow (multiply by a parameter to animate intensity)
- Fresnel rim light (cheap "edge glow" for heroes)
- Texture tiling for stone detail

This can be done via `tools/ue-place-assets.py` extended with `unreal.MaterialInstanceConstant` creation.

### 1g. Showcase Level Lighting

The current level has one directional + one sky light. Add:
- **Exponential Height Fog** for atmospheric depth (violet-tinted)
- **Post Process Volume** with bloom (makes emission glow visible) and slight color grading (warm midtones, cool shadows)
- **Point lights** at sacred circle center (warm gold) and at each temple pedestal
- **Rect lights** under corruption floor (violet uplight)

---

## Tier 2: Sculpting & Geometric Detail (Blender MCP)

**What it gets us:** Characters that look like characters, not mannequins. Organic muscle definition, facial features, armor surface detail, weapon engravings.

**Effort:** 2-4 sessions per character. Requires iterative sculpting via `execute_blender_code`.

### 2a. Sculpt Mode for Organic Detail

Blender's sculpt mode can be driven via MCP. The workflow:
1. Start with current blockout mesh
2. Apply Subdivision Surface modifier (level 2-3) to get enough geometry
3. Use sculpt brushes via Python: `bpy.ops.sculpt.brush_stroke()`
4. Key brushes: `SculptDraw` (add volume), `Smooth` (blend), `Crease` (define edges), `Clay Strips` (build up form)

```python
# Enter sculpt mode on character torso
obj = bpy.data.objects["SM_LW_Torso"]
bpy.context.view_layer.objects.active = obj
bpy.ops.object.mode_set(mode='SCULPT')
# Sculpt operations via stroke data...
```

**Limitation:** Sculpting via code is possible but awkward — you're defining brush strokes as coordinate arrays rather than painting interactively. Best used for systematic detail (muscle lines, panel grooves) rather than freeform artistry.

### 2b. UV Unwrapping

Current meshes have no UV maps (or auto-generated ones). Proper UVs enable:
- Painted textures instead of flat colors
- Normal maps for surface detail without geometry cost
- Proper texture tiling on large surfaces (arena floor)

```python
# Smart UV project all meshes
for obj in bpy.data.objects:
    if obj.type == "MESH":
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.uv.smart_project(angle_limit=66, island_margin=0.02)
        bpy.ops.object.mode_set(mode='OBJECT')
```

### 2c. Normal Map Baking (High-Poly to Low-Poly)

1. Duplicate mesh, apply high subdivision (level 3-4)
2. Sculpt detail on high-poly
3. Bake normals from high to low via `bpy.ops.object.bake(type='NORMAL')`
4. Apply baked normal map to low-poly material
5. Export low-poly with normal map — detail at zero geometry cost

### 2d. Retopology

If sculpted meshes get too dense, retopologize:
- Blender's Remesh modifier (voxel or quad) for quick cleanup
- `bpy.ops.object.voxel_remesh()` for automatic retopo
- Target: 2K-8K tris per character, 500-2K per weapon/prop

---

## Tier 3: AI-Assisted 3D Generation (Blender MCP Built-in)

**What it gets us:** Production-quality meshes with built-in materials generated from text prompts or concept art images. Dramatically faster than manual sculpting for complex organic shapes.

**Effort:** Minutes per asset (generation time), plus cleanup/scale/material adjustment session.

### 3a. Hyper3D Rodin — Text to 3D

The `mcp__blender-mcp__generate_hyper3d_model_via_text` tool generates textured 3D models from descriptions. Returns a model with built-in materials directly into the Blender scene.

```
Tool: mcp__blender-mcp__generate_hyper3d_model_via_text
text_prompt: "Heroic fantasy warrior in white-gold plate armor with blue cape,
              stylized game character, clean topology, sacred gold trim details"
bbox_condition: [1.0, 0.6, 1.8]  # width, depth, height ratio
```

**Best for:** Characters, complex props, anything organic. The generated mesh will likely need:
- Rescaling to match game units
- Material reassignment to match our style guide palette
- Possible mesh cleanup (remove internal faces, fix normals)

### 3b. Hyper3D Rodin — Image to 3D

The `mcp__blender-mcp__generate_hyper3d_model_via_images` tool takes concept art images and generates a 3D mesh. We already have concept art for every asset.

```
Tool: mcp__blender-mcp__generate_hyper3d_model_via_images
input_image_paths: ["D:/Phoenix/nix-code/games/light-warrior/assets/generated/concepts/character_light_warrior/concept_internal_image_gen_01.png"]
bbox_condition: [1.0, 0.6, 1.8]
```

**This is the highest-impact quality tool we have.** The concept art already captures the visual target. Feeding it directly into image-to-3D generation bridges the gap between "colored cylinders" and "actual character model" in one step.

**Recommended approach for each asset:**
1. Feed concept image into Hyper3D
2. Import generated mesh into a fresh Blender scene
3. Reassign materials to our palette (generated materials won't match exactly)
4. Adjust scale to match game units
5. Export FBX and re-import to UE5

### 3c. Hunyuan3D — Text/Image to 3D (Alternative)

`mcp__blender-mcp__generate_hunyuan3d_model` is an alternative generator. It accepts text, image, or both. Returns a job ID (async generation). Good for comparison — generate the same asset with both Hyper3D and Hunyuan3D, pick the better result.

```
Tool: mcp__blender-mcp__generate_hunyuan3d_model
text_prompt: "Dark violet shadow imp demon creature, angular spiky silhouette,
              glowing purple eyes, stylized game enemy"
input_image_url: "file:///D:/Phoenix/nix-code/games/light-warrior/assets/generated/concepts/enemy_shadow_imp/concept_internal_image_gen_01.png"
```

### 3d. PolyHaven — Environment Detail Assets

`mcp__blender-mcp__search_polyhaven_assets` + `download_polyhaven_asset` provides free CC0 assets. Useful for:
- **HDRIs** for lighting reference and Blender preview rendering
- **Textures** for stone, metal, fabric PBR textures (normal maps, roughness maps)
- **Models** for environmental props (rocks, debris, vegetation if any)

```
Tool: mcp__blender-mcp__search_polyhaven_assets
asset_type: "textures"
categories: "stone"
```

Then download specific textures:
```
Tool: mcp__blender-mcp__download_polyhaven_asset
asset_id: "castle_brick_02_red"
asset_type: "textures"
resolution: "2k"
```

**Best use:** Stone floor textures for the arena, metal textures for armor detail, HDRI for preview lighting.

### 3e. Sketchfab — Reference & Kitbash

`mcp__blender-mcp__search_sketchfab_models` finds downloadable 3D models for reference or kitbashing. Useful for:
- Studying how other artists solve the same design problems
- Borrowing topology approaches for characters
- Environment prop reference

**Note:** License terms vary. Use for reference and learning, not direct inclusion without checking.

---

## Tier 4: Full Production Pipeline

**What it gets us:** Animated characters, particle VFX, optimized performance, shippable assets.

**Effort:** Weeks of work. Most of this happens in UE5, not Blender.

### 4a. Skeletal Mesh & Rigging

Characters need skeletons for animation:
1. Create Armature in Blender (spine, arms, legs, head hierarchy)
2. Parent mesh to armature with automatic weights
3. Export as Skeletal Mesh FBX (not Static Mesh)
4. Import to UE5 as Skeletal Mesh
5. Create Animation Blueprints

```python
# Basic character armature
bpy.ops.object.armature_add()
armature = bpy.context.object
# Add bones: Hips → Spine → Chest → Head, L/R arms, L/R legs
# ... (substantial bone chain setup)
```

This is the biggest single task before characters can animate in-game.

### 4b. UE5 Niagara VFX

The Sacred Circle mesh is a base — the full VFX needs Niagara particle systems layered on top:
- Gold ring particle trail
- Expansion pulse wave
- Safety field shimmer
- Rune glow particles at nodes
- Corruption tendrils at boundary

This is UE5-side work, done via Blueprint or C++ Niagara system setup.

### 4c. LOD Generation

For performance, each mesh needs LOD (Level of Detail) chains:
- LOD0: full detail (current mesh)
- LOD1: 50% triangles (auto-decimated)
- LOD2: 25% triangles
- LOD3: billboard/impostor for far distance

Blender can auto-generate via Decimate modifier:
```python
obj = bpy.data.objects["SM_LW_Torso"]
decimate = obj.modifiers.new("LOD1", "DECIMATE")
decimate.ratio = 0.5
```

UE5 can also auto-generate LODs on import.

### 4d. Texture Atlasing

Combine multiple small textures into atlas sheets for draw call reduction:
- All hero materials → one atlas
- All enemy materials → one atlas
- All environment materials → one atlas

Becomes important when the scene has many unique actors.

---

## Recommended Execution Order

| Step | Tier | Action | Impact | Effort |
|------|------|--------|--------|--------|
| 1 | 1a | Fix all base colors to palette | HIGH | 30 min |
| 2 | 1b | Add metallic/roughness to all materials | HIGH | 30 min |
| 3 | 1c | Add emission to glow materials | HIGH | 20 min |
| 4 | 1e | Bevel + smooth shading pass | MEDIUM | 30 min |
| 5 | 1g | Fix showcase level lighting + fog + bloom | HIGH | 30 min |
| 6 | 3b | **Hyper3D image-to-3D on Light Warrior concept** | VERY HIGH | 15 min + cleanup |
| 7 | 3b | Hyper3D image-to-3D on Shadow Imp concept | VERY HIGH | 15 min + cleanup |
| 8 | 3b | Hyper3D image-to-3D on Thunder Hammer concept | HIGH | 15 min + cleanup |
| 9 | 1d | Procedural roughness on stone/corruption | MEDIUM | 30 min |
| 10 | 3d | PolyHaven stone textures for arena | MEDIUM | 15 min |
| 11 | 2b | UV unwrap all meshes | MEDIUM | 1 hr |
| 12 | 4a | Skeleton + rigging for Light Warrior | HIGH (for animation) | 2-4 hrs |

**Steps 1-5** can be done in a single session and will dramatically improve the look.
**Step 6-8** (AI generation from concept art) is the single biggest quality leap available — it turns concept paintings into actual 3D models with built-in materials.

---

## Quality Gates

Before an asset moves from `imported` to `approved` in `ASSET_MANIFEST.json`:

- [ ] Colors match the palette table above (sample in Blender, verify in UE5)
- [ ] Metallic/roughness values set per material type table
- [ ] Glow elements have emission (visible with bloom in UE5)
- [ ] Smooth shading applied, hard edges controlled via Auto Smooth
- [ ] Silhouette reads at gameplay camera distance (UE5 Play-In-Editor check)
- [ ] No Z-fighting or overlapping faces
- [ ] Scale correct in UE5 (character ~165cm, imp ~90cm, berserker ~175cm)
- [ ] Screenshot evidence captured in `captures/`
