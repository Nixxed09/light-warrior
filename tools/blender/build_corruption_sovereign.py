"""
Build: boss.corruption_sovereign (Light Warrior)

Final boss. Towering 3m dark-influence sovereign with corrupted crown,
tri-eyes, phase orbs, shadow wings, and rune channels.
- The anti-thesis of the Light Warrior: oppressive, enormous, angular, void-purple.
- Three floating phase orbs the player destroys across the fight.
- Red-violet clawed attack tells, violet rune glow power channels.

Scale: ~3.0m tall. 1 Blender unit = 1 m.
Idempotent: clears prior SM_LW_CorruptionSovereign objects and the default Cube.
Export target: assets/generated/models/corruption_sovereign/SM_LW_CorruptionSovereign.fbx
"""

import bpy
import math
import os

PREFIX = "SM_LW_CorruptionSovereign"
EXPORT_DIR = r"D:\Phoenix\nix-code\games\light-warrior\assets\generated\models\corruption_sovereign"

C_VOID     = (0.04, 0.01, 0.08, 1.0)
C_CORRUPT  = (0.12, 0.04, 0.18, 1.0)
C_CROWN    = (0.25, 0.08, 0.35, 1.0)
C_TELL     = (0.85, 0.10, 0.35, 1.0)
C_EYES     = (1.0, 0.0, 0.30, 1.0)
C_RUNES    = (0.60, 0.0, 0.80, 1.0)


def purge():
    for obj in list(bpy.data.objects):
        if obj.name.startswith(PREFIX) or obj.name == "Cube":
            bpy.data.objects.remove(obj, do_unlink=True)


def make_mat(name, color, metallic=0.0, roughness=0.5, emission=None, em_str=0.0):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Metallic"].default_value = metallic
    bsdf.inputs["Roughness"].default_value = roughness
    if emission:
        for key in ("Emission Color", "Emission"):
            if key in bsdf.inputs:
                bsdf.inputs[key].default_value = emission
                break
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = em_str
    return mat


def add_box(name, size, loc, mat):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    o = bpy.context.active_object
    o.name = name
    o.scale = (size[0]/2, size[1]/2, size[2]/2)
    bpy.ops.object.transform_apply(scale=True)
    o.data.materials.append(mat)
    return o


def add_cone(name, r1, r2, d, loc, mat, v=6):
    bpy.ops.mesh.primitive_cone_add(radius1=r1, radius2=r2, depth=d, location=loc, vertices=v)
    o = bpy.context.active_object
    o.name = name
    o.data.materials.append(mat)
    return o


def add_cyl(name, r, d, loc, mat, v=12):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=d, location=loc, vertices=v)
    o = bpy.context.active_object
    o.name = name
    o.data.materials.append(mat)
    return o


def add_sphere(name, r, loc, mat, seg=12, ring=8):
    bpy.ops.mesh.primitive_uv_sphere_add(radius=r, location=loc, segments=seg, ring_count=ring)
    o = bpy.context.active_object
    o.name = name
    o.data.materials.append(mat)
    return o


def bevel(obj, w=0.02, s=2):
    bpy.context.view_layer.objects.active = obj
    m = obj.modifiers.new("Bevel", "BEVEL")
    m.width = w
    m.segments = s
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
    print(f"[CorruptionSovereign] Exported: {fbx_path}")


def build():
    purge()

    mat_void  = make_mat("M_LW_VoidBody",      C_VOID,    roughness=0.8)
    mat_corr  = make_mat("M_LW_CorruptArmor",  C_CORRUPT, metallic=0.6, roughness=0.4)
    mat_crown = make_mat("M_LW_CorruptCrown",  C_CROWN,   metallic=0.8, roughness=0.3)
    mat_tell  = make_mat("M_LW_AttackTell",    C_TELL,    emission=C_TELL, em_str=8.0)
    mat_eyes  = make_mat("M_LW_SovereignEyes", C_EYES,    emission=C_EYES, em_str=18.0)
    mat_runes = make_mat("M_LW_CorruptRunes",  C_RUNES,   emission=C_RUNES, em_str=10.0)

    parts = []

    # Lower body: spectral robe
    robe_base = add_cone(f"{PREFIX}_RobeBase", 0.50, 0.30, 1.0, (0, 0, 0.55), mat_void, 8)
    parts.append(robe_base)
    robe_trim = add_cyl(f"{PREFIX}_RobeTrim", 0.52, 0.06, (0, 0, 0.06), mat_runes, 8)
    parts.append(robe_trim)
    belt = add_cyl(f"{PREFIX}_Belt", 0.32, 0.08, (0, 0, 1.05), mat_corr, 8)
    parts.append(belt)

    # Torso
    torso = add_box(f"{PREFIX}_Torso", (0.70, 0.45, 0.60), (0, 0, 1.40), mat_void)
    bevel(torso, 0.03, 2)
    parts.append(torso)
    chest_plate = add_box(f"{PREFIX}_ChestPlate", (0.60, 0.08, 0.50), (0, -0.24, 1.40), mat_corr)
    bevel(chest_plate, 0.025, 2)
    parts.append(chest_plate)
    chest_rune = add_box(f"{PREFIX}_ChestRune", (0.04, 0.01, 0.35), (0, -0.30, 1.40), mat_runes)
    parts.append(chest_rune)
    chest_rune_h = add_box(f"{PREFIX}_ChestRuneH", (0.30, 0.01, 0.04), (0, -0.30, 1.42), mat_runes)
    parts.append(chest_rune_h)

    # Shoulders
    for side, x in [("L", 0.50), ("R", -0.50)]:
        pauldron = add_box(f"{PREFIX}_Pauldron_{side}", (0.30, 0.28, 0.28), (x, 0, 1.65), mat_corr)
        bevel(pauldron, 0.03, 2)
        parts.append(pauldron)
        for i, offset in enumerate([-0.08, 0, 0.08]):
            spike = add_cone(f"{PREFIX}_PauldronSpike_{side}_{i}", 0.04, 0.0, 0.25 + i * 0.05,
                              (x + offset, 0, 1.82 + i * 0.02), mat_corr, 4)
            parts.append(spike)

    # Arms
    for side, x_sign in [("L", 1), ("R", -1)]:
        x = x_sign * 0.50
        upper = add_box(f"{PREFIX}_UpperArm_{side}", (0.12, 0.12, 0.40), (x, -0.04, 1.38), mat_void)
        parts.append(upper)
        forearm = add_box(f"{PREFIX}_Forearm_{side}", (0.10, 0.10, 0.45),
                           (x_sign * 0.55, -0.10, 0.95), mat_void)
        parts.append(forearm)
        gauntlet = add_cyl(f"{PREFIX}_Gauntlet_{side}", 0.08, 0.18,
                            (x_sign * 0.55, -0.10, 0.80), mat_corr, 8)
        parts.append(gauntlet)
        for fi, angle in enumerate([-25, 0, 25]):
            claw = add_cone(f"{PREFIX}_Claw_{side}_{fi}", 0.025, 0.0, 0.18,
                             (x_sign * 0.55 + math.sin(math.radians(angle)) * 0.04, -0.14, 0.65),
                             mat_tell, 3)
            claw.rotation_euler = (math.radians(15), math.radians(angle * 0.5), 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(claw)

    # Head
    head = add_box(f"{PREFIX}_Head", (0.26, 0.24, 0.28), (0, -0.04, 1.85), mat_void)
    bevel(head, 0.02, 2)
    parts.append(head)
    jaw = add_box(f"{PREFIX}_Jaw", (0.20, 0.16, 0.08), (0, -0.10, 1.72), mat_void)
    parts.append(jaw)

    # Tri-eyes
    eye_center = add_sphere(f"{PREFIX}_EyeCenter", 0.04, (0, -0.16, 1.90), mat_eyes, 10, 8)
    parts.append(eye_center)
    for side, x in [("L", 0.08), ("R", -0.08)]:
        eye = add_sphere(f"{PREFIX}_Eye_{side}", 0.03, (x, -0.15, 1.86), mat_eyes, 8, 6)
        parts.append(eye)

    # Crown
    crown_center = add_cone(f"{PREFIX}_CrownCenter", 0.06, 0.0, 0.50, (0, 0, 2.25), mat_crown, 4)
    parts.append(crown_center)
    crown_ring = add_cyl(f"{PREFIX}_CrownRing", 0.18, 0.06, (0, 0, 2.0), mat_crown, 6)
    parts.append(crown_ring)
    for i in range(6):
        angle = i * (2 * math.pi / 6)
        x = math.sin(angle) * 0.16
        y = math.cos(angle) * 0.16
        tine = add_cone(f"{PREFIX}_CrownTine_{i}", 0.03, 0.0, 0.30 + (i % 2) * 0.10,
                         (x, y, 2.12), mat_crown, 4)
        tine.rotation_euler = (math.radians(-15) * math.cos(angle),
                               math.radians(15) * math.sin(angle), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(tine)
    crown_rune = add_cyl(f"{PREFIX}_CrownRune", 0.19, 0.02, (0, 0, 2.0), mat_runes, 6)
    parts.append(crown_rune)

    # Phase orbs
    for i in range(3):
        angle = i * (2 * math.pi / 3) + math.radians(30)
        x = math.sin(angle) * 0.80
        y = math.cos(angle) * 0.80
        orb = add_sphere(f"{PREFIX}_PhaseOrb_{i}", 0.08, (x, y, 1.50), mat_runes, 10, 8)
        parts.append(orb)
        cage = add_box(f"{PREFIX}_OrbCage_{i}", (0.20, 0.20, 0.20), (x, y, 1.50), mat_corr)
        cage.rotation_euler = (math.radians(45), math.radians(45), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(cage)

    # Shadow wings
    for side, x_sign in [("L", 1), ("R", -1)]:
        wing = add_box(f"{PREFIX}_Wing_{side}", (0.08, 0.60, 0.80),
                        (x_sign * 0.30, 0.35, 1.50), mat_void)
        wing.rotation_euler = (math.radians(-10), math.radians(20 * x_sign), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(wing)
        wing_tip = add_cone(f"{PREFIX}_WingTip_{side}", 0.08, 0.0, 0.35,
                              (x_sign * 0.45, 0.65, 1.90), mat_corr, 4)
        wing_tip.rotation_euler = (math.radians(-30), math.radians(15 * x_sign), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(wing_tip)

    # Parent all
    bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
    root = bpy.context.active_object
    root.name = PREFIX
    for p in parts:
        p.parent = root
        p.matrix_parent_inverse = root.matrix_world.inverted()

    print(f"[CorruptionSovereign] Built {len(parts)} parts under '{root.name}'")
    return root


if __name__ == "__main__":
    build()
    export_fbx(PREFIX)
