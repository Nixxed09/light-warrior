"""
Build: weapon.thunder_hammer v2 (Light Warrior)

Procedural Thunder Hammer for UE5. Style anchor: STYLE_GUIDE.md / ASSET_MANIFEST.json
- "luminous guardian power made physical: sacred gold, pale stone or electrum
  details, readable shockwave silhouette."
- Readability requirement: hammer head, grip, lightning core read during motion.
- Palette: sacred gold (head), electrum / pale stone (trim, grip), dawn-blue
  emissive lightning core. Red is reserved for enemy damage only - none here.

Scale: real-world, 1 Blender unit = 1 m. Z-up, grip pommel at origin.
Idempotent: clears prior SM_LW_ThunderHammer objects and the default Cube.
Export target: assets/generated/models/thunder_hammer/SM_LW_ThunderHammer.fbx
"""

import bpy
import os

PREFIX = "SM_LW_ThunderHammer"
EXPORT_DIR = r"D:\Phoenix\nix-code\games\light-warrior\assets\generated\models\thunder_hammer"

C_GOLD     = (0.83, 0.60, 0.16, 1.0)
C_ELECTRUM = (0.74, 0.72, 0.55, 1.0)
C_STONE    = (0.78, 0.76, 0.70, 1.0)
C_CORE     = (0.20, 0.55, 1.00, 1.0)


def purge():
    for obj in list(bpy.data.objects):
        if obj.name.startswith(PREFIX) or obj.name == "Cube":
            bpy.data.objects.remove(obj, do_unlink=True)


def make_material(name, color, metallic=1.0, roughness=0.3,
                  emission=None, emission_strength=0.0):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Metallic"].default_value = metallic
    bsdf.inputs["Roughness"].default_value = roughness
    if emission is not None:
        for key in ("Emission Color", "Emission"):
            if key in bsdf.inputs:
                bsdf.inputs[key].default_value = emission
                break
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = emission_strength
    return mat


def add_box(name, size, location, material):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (size[0] / 2.0, size[1] / 2.0, size[2] / 2.0)
    bpy.ops.object.transform_apply(scale=True)
    obj.data.materials.append(material)
    return obj


def add_cyl(name, radius, depth, location, material, verts=24):
    bpy.ops.mesh.primitive_cylinder_add(radius=radius, depth=depth,
                                         location=location, vertices=verts)
    obj = bpy.context.active_object
    obj.name = name
    obj.data.materials.append(material)
    return obj


def bevel(obj, width=0.015, segments=2):
    bpy.context.view_layer.objects.active = obj
    m = obj.modifiers.new("Bevel", "BEVEL")
    m.width = width
    m.segments = segments
    m.limit_method = "ANGLE"
    m.angle_limit = 0.785
    bpy.ops.object.modifier_apply(modifier=m.name)


def export_fbx(root_name):
    os.makedirs(EXPORT_DIR, exist_ok=True)
    fbx_path = os.path.join(EXPORT_DIR, f"{root_name}.fbx")
    bpy.ops.object.select_all(action='DESELECT')
    for obj in bpy.data.objects:
        if obj.name.startswith(root_name):
            obj.select_set(True)
    bpy.ops.export_scene.fbx(
        filepath=fbx_path, use_selection=True,
        apply_scale_options='FBX_SCALE_ALL', axis_forward='-Y', axis_up='Z',
        mesh_smooth_type='FACE', use_mesh_modifiers=True, add_leaf_bones=False
    )
    print(f"[ThunderHammer] Exported: {fbx_path}")


def build():
    purge()

    mat_gold  = make_material("M_LW_Gold",      C_GOLD,     metallic=1.0, roughness=0.30)
    mat_elec  = make_material("M_LW_Electrum",   C_ELECTRUM, metallic=1.0, roughness=0.22)
    mat_stone = make_material("M_LW_GripStone",  C_STONE,    metallic=0.0, roughness=0.65)
    mat_core  = make_material("M_LW_LightCore",  C_CORE,     metallic=0.0, roughness=0.10,
                              emission=C_CORE, emission_strength=14.0)

    parts = []

    grip = add_cyl(f"{PREFIX}_Grip", radius=0.04, depth=1.10,
                   location=(0, 0, 0.58), material=mat_stone)
    parts.append(grip)

    for i, z in enumerate([0.25, 0.50, 0.75]):
        band = add_cyl(f"{PREFIX}_GripBand_{i}", radius=0.052, depth=0.025,
                       location=(0, 0, z), material=mat_elec)
        parts.append(band)

    pommel = add_cyl(f"{PREFIX}_Pommel", radius=0.065, depth=0.12,
                     location=(0, 0, 0.04), material=mat_elec)
    bevel(pommel, width=0.015, segments=2)
    parts.append(pommel)

    pommel_gem = add_cyl(f"{PREFIX}_PommelGem", radius=0.025, depth=0.03,
                         location=(0, 0, -0.02), material=mat_core)
    parts.append(pommel_gem)

    collar = add_cyl(f"{PREFIX}_Collar", radius=0.065, depth=0.08,
                     location=(0, 0, 1.08), material=mat_elec)
    bevel(collar, width=0.012, segments=2)
    parts.append(collar)

    head = add_box(f"{PREFIX}_Head", size=(0.52, 0.30, 0.36),
                   location=(0, 0, 1.28), material=mat_gold)
    bevel(head, width=0.03, segments=3)
    parts.append(head)

    face_l = add_box(f"{PREFIX}_FaceL", size=(0.06, 0.32, 0.38),
                     location=(0.27, 0, 1.28), material=mat_elec)
    bevel(face_l, width=0.02, segments=2)
    parts.append(face_l)

    face_r = add_box(f"{PREFIX}_FaceR", size=(0.06, 0.32, 0.38),
                     location=(-0.27, 0, 1.28), material=mat_elec)
    bevel(face_r, width=0.02, segments=2)
    parts.append(face_r)

    core_main = add_box(f"{PREFIX}_CoreMain", size=(0.62, 0.08, 0.16),
                        location=(0, 0, 1.28), material=mat_core)
    parts.append(core_main)

    core_vert = add_box(f"{PREFIX}_CoreVert", size=(0.08, 0.08, 0.38),
                        location=(0, 0, 1.28), material=mat_core)
    parts.append(core_vert)

    bpy.ops.mesh.primitive_cone_add(radius1=0.09, radius2=0.0, depth=0.24,
                                    location=(0, 0, 1.58), vertices=4)
    crown = bpy.context.active_object
    crown.name = f"{PREFIX}_Crown"
    crown.rotation_euler = (0, 0, 0.785)
    bpy.ops.object.transform_apply(rotation=True)
    crown.data.materials.append(mat_gold)
    parts.append(crown)

    crown_base = add_cyl(f"{PREFIX}_CrownBase", radius=0.08, depth=0.04,
                         location=(0, 0, 1.46), material=mat_elec)
    parts.append(crown_base)

    rune_l = add_box(f"{PREFIX}_RuneL", size=(0.005, 0.20, 0.04),
                     location=(0.275, 0, 1.28), material=mat_core)
    parts.append(rune_l)
    rune_r = add_box(f"{PREFIX}_RuneR", size=(0.005, 0.20, 0.04),
                     location=(-0.275, 0, 1.28), material=mat_core)
    parts.append(rune_r)

    bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
    root = bpy.context.active_object
    root.name = PREFIX
    for p in parts:
        p.parent = root
        p.matrix_parent_inverse = root.matrix_world.inverted()

    print(f"[ThunderHammer v2] Built {len(parts)} parts under '{root.name}'")
    return root


if __name__ == "__main__":
    build()
    export_fbx(PREFIX)
