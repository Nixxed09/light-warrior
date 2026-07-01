"""
Build: enemy.berserker (Light Warrior)

Heavy pressure enemy. Massive top-heavy silhouette, bull-charge horns,
glowing red-violet fists and chest charge core.
- "dark influence as containment/retaliation: heavy angular silhouette,
  visible charge tell, violet-black corruption with limited red damage edge."
- Readability: large violet-black silhouette, red-violet charge tell, stun
  state, and gold defeat dissolve must read from the default UE5 camera.

Scale: ~1.6m tall. 1 Blender unit = 1 m.
Idempotent: clears prior SM_LW_Berserker objects and the default Cube.
Export target: assets/generated/models/berserker/SM_LW_Berserker.fbx
"""

import bpy
import math
import os

PREFIX = "SM_LW_Berserker"
EXPORT_DIR = r"D:\Phoenix\nix-code\games\light-warrior\assets\generated\models\berserker"

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


def add_cone(name, r1, r2, d, loc, mat, v=6):
    bpy.ops.mesh.primitive_cone_add(radius1=r1, radius2=r2, depth=d, location=loc, vertices=v)
    o = bpy.context.active_object
    o.name = name
    o.data.materials.append(mat)
    return o


def add_sphere(name, r, loc, mat, seg=10, ring=8):
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
    print(f"[Berserker] Exported: {fbx_path}")


def build():
    purge()

    mat_body   = make_mat("M_LW_ShadowBody",   C_SHADOW,     roughness=0.7)
    mat_detail = make_mat("M_LW_ShadowDetail", C_SHADOW_MID, roughness=0.6)
    mat_tell   = make_mat("M_LW_AttackTell",   C_TELL,       emission=C_TELL, em_str=8.0)
    mat_eyes   = make_mat("M_LW_ShadowEyes",  C_EYES,       emission=C_EYES, em_str=12.0)

    parts = []

    for side, x in [("L", 0.18), ("R", -0.18)]:
        thigh = add_box(f"{PREFIX}_Thigh_{side}", (0.18, 0.16, 0.40), (x, 0, 0.35), mat_body)
        parts.append(thigh)
        shin = add_box(f"{PREFIX}_Shin_{side}", (0.16, 0.14, 0.35), (x, 0, 0.10), mat_body)
        parts.append(shin)
        knee = add_cone(f"{PREFIX}_Knee_{side}", 0.06, 0.0, 0.14, (x, -0.10, 0.35), mat_detail, 4)
        knee.rotation_euler = (math.radians(-60), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(knee)
        foot = add_box(f"{PREFIX}_Foot_{side}", (0.18, 0.24, 0.08), (x, -0.04, -0.02), mat_detail)
        parts.append(foot)

    torso = add_box(f"{PREFIX}_Torso", (0.55, 0.40, 0.50), (0, 0, 0.75), mat_body)
    bevel(torso, 0.03, 2)
    parts.append(torso)

    belly = add_box(f"{PREFIX}_BellyPlate", (0.45, 0.05, 0.35), (0, -0.22, 0.72), mat_detail)
    bevel(belly, 0.02, 2)
    parts.append(belly)

    for side, x in [("L", 0.38), ("R", -0.38)]:
        pauldron = add_box(f"{PREFIX}_Pauldron_{side}", (0.22, 0.22, 0.20), (x, 0, 1.02), mat_body)
        bevel(pauldron, 0.025, 2)
        parts.append(pauldron)
        spike = add_cone(f"{PREFIX}_ShoulderSpike_{side}", 0.06, 0.0, 0.22, (x, 0, 1.18), mat_detail, 4)
        parts.append(spike)

    for side, x_sign in [("L", 1), ("R", -1)]:
        x = x_sign * 0.38
        upper = add_box(f"{PREFIX}_UpperArm_{side}", (0.14, 0.14, 0.30), (x, -0.02, 0.82), mat_body)
        upper.rotation_euler = (0, 0, math.radians(10 * x_sign))
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(upper)
        forearm = add_box(f"{PREFIX}_Forearm_{side}", (0.16, 0.16, 0.28),
                           (x_sign * 0.42, -0.06, 0.55), mat_body)
        parts.append(forearm)
        fist = add_box(f"{PREFIX}_Fist_{side}", (0.18, 0.18, 0.16),
                        (x_sign * 0.44, -0.08, 0.36), mat_tell)
        bevel(fist, 0.02, 2)
        parts.append(fist)

    head = add_box(f"{PREFIX}_Head", (0.24, 0.22, 0.22), (0, -0.06, 1.10), mat_body)
    bevel(head, 0.02, 2)
    parts.append(head)

    brow = add_box(f"{PREFIX}_Brow", (0.26, 0.10, 0.06), (0, -0.14, 1.18), mat_detail)
    parts.append(brow)

    for side, x in [("L", 0.06), ("R", -0.06)]:
        eye = add_sphere(f"{PREFIX}_Eye_{side}", 0.025, (x, -0.15, 1.12), mat_eyes, 8, 6)
        parts.append(eye)

    for side, x_sign in [("L", 1), ("R", -1)]:
        horn = add_cone(f"{PREFIX}_Horn_{side}", 0.05, 0.01, 0.28,
                         (x_sign * 0.14, -0.08, 1.22), mat_detail, 6)
        horn.rotation_euler = (math.radians(-40), math.radians(25 * x_sign), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(horn)

    charge_core = add_sphere(f"{PREFIX}_ChargeCore", 0.08, (0, -0.22, 0.80), mat_tell, 10, 8)
    parts.append(charge_core)

    for i, z in enumerate([0.65, 0.80, 0.95]):
        spike = add_cone(f"{PREFIX}_BackSpike_{i}", 0.05, 0.0, 0.18 + i * 0.04,
                          (0, 0.22, z), mat_detail, 4)
        spike.rotation_euler = (math.radians(30), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(spike)

    bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
    root = bpy.context.active_object
    root.name = PREFIX
    for p in parts:
        p.parent = root
        p.matrix_parent_inverse = root.matrix_world.inverted()

    print(f"[Berserker] Built {len(parts)} parts under '{root.name}'")
    return root


if __name__ == "__main__":
    build()
    export_fbx(PREFIX)
