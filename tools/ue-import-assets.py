"""
UE5 Python script to import Light Warrior FBX assets.
Run via: UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this_file> -unattended -nopause
"""

import unreal
import os

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MODELS_DIR = os.path.join(PROJECT_ROOT, "assets", "generated", "models")

IMPORTS = [
    # --- Original blockouts ---
    {
        "fbx": os.path.join(MODELS_DIR, "arena", "SM_LW_Arena_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Arena",
        "name": "SM_LW_Arena",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "sacred_circle", "SM_SacredCircle_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/VFX",
        "name": "SM_SacredCircle",
    },
    # --- Polished assets (subdivision + PBR materials) ---
    {
        "fbx": os.path.join(MODELS_DIR, "light_warrior_character", "SM_LightWarrior_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Characters",
        "name": "SM_LightWarrior",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "thunder_hammer", "SM_LW_ThunderHammer.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Weapons",
        "name": "SM_LW_ThunderHammer",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "shadow_imp", "SM_LW_ShadowImp.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Enemies",
        "name": "SM_LW_ShadowImp",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "berserker", "SM_LW_Berserker.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Enemies",
        "name": "SM_LW_Berserker",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "corruption_sovereign", "SM_LW_CorruptionSovereign.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Enemies",
        "name": "SM_LW_CorruptionSovereign",
    },
    # --- AI-generated assets (older) ---
    {
        "fbx": os.path.join(MODELS_DIR, "light_shard", "SM_LW_LightShard_AI.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Pickups",
        "name": "SM_LW_LightShard",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "corruption_crystals", "SM_LW_CorruptionCrystals_AI.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Arena",
        "name": "SM_LW_CorruptionCrystals",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "temple_pedestal", "SM_LW_TemplePedestal_AI.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Arena",
        "name": "SM_LW_TemplePedestal",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "temple_pillar", "SM_LW_TemplePillar_AI.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Arena",
        "name": "SM_LW_TemplePillar",
    },
    # --- Hyper3D Rodin AI-generated (concept art to 3D) ---
    {
        "fbx": os.path.join(MODELS_DIR, "light_warrior_character", "SM_LW_Hero_Hyper3D_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Characters",
        "name": "SM_LW_Hero_Hyper3D",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "shadow_imp", "SM_LW_ShadowImp_Hyper3D_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Enemies",
        "name": "SM_LW_ShadowImp_Hyper3D",
    },
    {
        "fbx": os.path.join(MODELS_DIR, "thunder_hammer", "SM_LW_ThunderHammer_Hyper3D_LOD0.fbx"),
        "dest": "/Game/LightWarrior/Meshes/Weapons",
        "name": "SM_LW_ThunderHammer_Hyper3D",
    },
]


def import_fbx(fbx_path, destination_path, asset_name):
    if not os.path.exists(fbx_path):
        unreal.log_error(f"FBX not found: {fbx_path}")
        return False

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx_path)
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_as_skeletal", False)
    options.static_mesh_import_data.set_editor_property("combine_meshes", False)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", True)
    options.static_mesh_import_data.set_editor_property("generate_lightmap_u_vs", True)

    task.set_editor_property("options", options)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    if task.get_editor_property("imported_object_paths"):
        unreal.log(f"Imported: {asset_name} -> {destination_path}")
        return True
    else:
        unreal.log_warning(f"Import may have issues: {asset_name}")
        return True


def main():
    unreal.log("=== Light Warrior Asset Import ===")
    success = 0
    failed = 0

    for item in IMPORTS:
        unreal.log(f"Importing {item['name']} from {item['fbx']}...")
        if import_fbx(item["fbx"], item["dest"], item["name"]):
            success += 1
        else:
            failed += 1

    unreal.log(f"=== Import complete: {success} succeeded, {failed} failed ===")


main()
