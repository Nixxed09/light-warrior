"""
Build: enemy.shadow_imp (Light Warrior)

Small fast pressure enemy. Angular violet-black body, red-violet attack tells,
gold dissolve on defeat.
- "embodied dark influence: angular violet-black body, bright readable attack
  tell, dissolves into reclaimed light on defeat."
- Readability: violet-black silhouette, red-violet attack tell, gold dissolve
  must read from the default UE5 camera.

Scale: ~0.7m tall. 1 Blender unit = 1 m.
Idempotent: clears prior SM_LW_ShadowImp objects and the default Cube.
Export target: assets/generated/models/shadow_imp/SM_LW_ShadowImp.fbx
"""

import bpy
import math
import os

PREFIX = "SM_LW_ShadowImp"
EXPORT_DIR = r"D:\Phoenix\nix-code\games\light-warrior\assets\generated\models\shadow_imp"

C_SHADOW     = (0.08, 0.03, 0.12, 1.0)
C_SHADOW_MID = (0.15, 0.06, 0.22, 1.0)
C_TELL       = (0.85, 0.10, 0.35, 1.0)
C_EYES       = (1.0, 0.15, 0.40, 1.0)


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


def add_cone(name, r1, r2, depth, loc, mat, verts=6):
    bpy.ops.mesh.primitive_cone_add(radius1=r1, radius2=r2, depth=depth, location=loc, vertices=verts)
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
    print(f"[ShadowImp] Exported: {fbx_path}")


def build():
    purge()

    mat_body   = make_mat("M_LW_ShadowBody",   C_SHADOW,     roughness=0.7)
    mat_detail = make_mat("M_LW_ShadowDetail", C_SHADOW_MID, roughness=0.6)
    mat_tell   = make_mat("M_LW_AttackTell",   C_TELL,       emission=C_TELL, em_str=8.0)
    mat_eyes   = make_mat("M_LW_ShadowEyes",  C_EYES,       emission=C_EYES, em_str=12.0)

    parts = []

    torso = add_box(f"{PREFIX}_Torso", (0.28, 0.20, 0.30), (0, 0, 0.40), mat_body)
    torso.rotation_euler = (math.radians(10), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(torso)

    chest_spike = add_cone(f"{PREFIX}_ChestSpike", 0.06, 0.0, 0.12,
                            (0, -0.12, 0.42), mat_detail, 4)
    chest_spike.rotation_euler = (math.radians(-70), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(chest_spike)

    head = add_box(f"{PREFIX}_Head", (0.22, 0.18, 0.16), (0, -0.04, 0.62), mat_body)
    head.rotation_euler = (math.radians(15), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(head)

    for side, x in [("L", 0.10), ("R", -0.10)]:
        horn = add_cone(f"{PREFIX}_Horn_{side}", 0.03, 0.0, 0.18,
                         (x, 0.02, 0.72), mat_detail, 4)
        horn.rotation_euler = (math.radians(-30), math.radians(15 if side == "L" else -15), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(horn)

    for side, x in [("L", 0.06), ("R", -0.06)]:
        eye = add_sphere(f"{PREFIX}_Eye_{side}", 0.025, (x, -0.10, 0.64), mat_eyes, 8, 6)
        parts.append(eye)

    for side, x_sign in [("L", 1), ("R", -1)]:
        arm = add_box(f"{PREFIX}_Arm_{side}", (0.08, 0.08, 0.22),
                       (x_sign * 0.20, -0.02, 0.38), mat_body)
        arm.rotation_euler = (math.radians(20), 0, math.radians(-20 * x_sign))
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(arm)

        claw = add_cone(f"{PREFIX}_Claw_{side}", 0.05, 0.0, 0.16,
                         (x_sign * 0.28, -0.08, 0.24), mat_tell, 3)
        claw.rotation_euler = (math.radians(60), 0, math.radians(15 * x_sign))
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(claw)

    for side, x_sign in [("L", 1), ("R", -1)]:
        thigh = add_box(f"{PREFIX}_Leg_{side}", (0.10, 0.10, 0.18),
                         (x_sign * 0.08, 0, 0.18), mat_body)
        parts.append(thigh)
        foot = add_cone(f"{PREFIX}_Foot_{side}", 0.06, 0.02, 0.08,
                         (x_sign * 0.08, -0.04, 0.06), mat_detail, 4)
        parts.append(foot)

    tail = add_cone(f"{PREFIX}_Tail", 0.05, 0.0, 0.25,
                     (0, 0.18, 0.32), mat_detail, 4)
    tail.rotation_euler = (math.radians(-50), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(tail)

    tell_orb = add_sphere(f"{PREFIX}_TellOrb", 0.04, (0, -0.11, 0.42), mat_tell, 8, 6)
    parts.append(tell_orb)

    bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
    root = bpy.context.active_object
    root.name = PREFIX
    for p in parts:
        p.parent = root
        p.matrix_parent_inverse = root.matrix_world.inverted()

    print(f"[ShadowImp] Built {len(parts)} parts under '{root.name}'")
    return root


if __name__ == "__main__":
    build()
    export_fbx(PREFIX)
