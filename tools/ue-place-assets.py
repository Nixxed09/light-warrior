"""
UE5 Python script to create an asset showcase level with all imported meshes,
production lighting, fog, and post-processing.
Run via: UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this_file> -unattended -nopause
"""

import unreal

LEVEL_NAME = "/Game/LightWarrior/Maps/AssetShowcase"


def load_and_spawn(asset_path, location, rotation=(0, 0, 0), scale=(1, 1, 1)):
    mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
    if mesh is None:
        unreal.log_warning(f"Could not load: {asset_path}")
        return None

    loc = unreal.Vector(location[0] * 100, location[1] * 100, location[2] * 100)
    rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, loc, rot
    )
    if actor:
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.set_actor_scale3d(unreal.Vector(scale[0], scale[1], scale[2]))
        actor.set_actor_label(asset_path.split("/")[-1])
    return actor


def spawn_all_in_folder(folder_path, center, spread_radius=5.0):
    assets = unreal.EditorAssetLibrary.list_assets(folder_path, recursive=True)
    meshes = [a for a in assets if "SM_" in a]

    count = len(meshes)
    if count == 0:
        return 0

    for i, asset_path in enumerate(meshes):
        clean_path = asset_path.split(".")[0]
        load_and_spawn(clean_path, center)

    unreal.log(f"Spawned {count} meshes from {folder_path}")
    return count


def setup_lighting():
    """Production-quality lighting: sun, sky, fog, point lights, post-process."""

    # Warm directional sun — slightly angled for dramatic shadows
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(-40, 25, 0)
    )
    if sun:
        sun.set_actor_label("SacredSunLight")
        light = sun.light_component
        light.set_editor_property("intensity", 8.0)
        light.set_editor_property("light_color", unreal.Color(255, 240, 210, 255))

    # Sky light for ambient fill
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 500),
        unreal.Rotator(0, 0, 0)
    )
    if sky:
        sky.set_actor_label("AmbientSkyLight")

    # Sacred gold point light at arena center
    center_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(0, 0, 300),
        unreal.Rotator(0, 0, 0)
    )
    if center_light:
        center_light.set_actor_label("CenterSacredGlow")
        pl = center_light.point_light_component
        pl.set_editor_property("intensity", 5000.0)
        pl.set_editor_property("light_color", unreal.Color(255, 210, 50, 255))
        pl.set_editor_property("attenuation_radius", 1500.0)

    # Violet uplight under corruption floor
    corruption_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(0, 0, -100),
        unreal.Rotator(0, 0, 0)
    )
    if corruption_light:
        corruption_light.set_actor_label("CorruptionUplight")
        cl = corruption_light.point_light_component
        cl.set_editor_property("intensity", 3000.0)
        cl.set_editor_property("light_color", unreal.Color(75, 0, 130, 255))
        cl.set_editor_property("attenuation_radius", 2000.0)

    # Exponential height fog for atmospheric depth
    try:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.ExponentialHeightFog,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if fog:
            fog.set_actor_label("VioletAtmosphere")
            fog_comp = fog.component
            fog_comp.set_editor_property("fog_density", 0.02)
            fog_comp.set_editor_property("fog_max_opacity", 0.8)
            fog_comp.set_editor_property("fog_height_falloff", 0.2)
            unreal.log("Added exponential height fog")
    except Exception as e:
        unreal.log_warning(f"Could not add fog: {e}")

    # Post-process volume for bloom and color grading
    try:
        pp = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PostProcessVolume,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if pp:
            pp.set_actor_label("ShowcasePostProcess")
            pp.set_editor_property("unbound", True)
            settings = pp.settings
            settings.set_editor_property("override_bloom_intensity", True)
            settings.set_editor_property("bloom_intensity", 1.5)
            settings.set_editor_property("override_bloom_threshold", True)
            settings.set_editor_property("bloom_threshold", 0.8)
            settings.set_editor_property("override_auto_exposure_bias", True)
            settings.set_editor_property("auto_exposure_bias", 0.5)
            unreal.log("Added post-process volume with bloom")
    except Exception as e:
        unreal.log_warning(f"Could not add post-process: {e}")

    unreal.log("Lighting setup complete")


def main():
    unreal.log("=== Asset Showcase Level Builder (Quality Pass) ===")

    unreal.EditorLevelLibrary.new_level(LEVEL_NAME)
    unreal.log(f"Created level: {LEVEL_NAME}")

    arena_count = spawn_all_in_folder("/Game/LightWarrior/Meshes/Arena", center=(0, 0, 0))
    vfx_count = spawn_all_in_folder("/Game/LightWarrior/Meshes/VFX", center=(0, 0, 0.1))
    char_count = spawn_all_in_folder("/Game/LightWarrior/Meshes/Characters", center=(0, 0, 0))
    weapon_count = spawn_all_in_folder("/Game/LightWarrior/Meshes/Weapons", center=(0.5, 0, 0))
    enemy_count = spawn_all_in_folder("/Game/LightWarrior/Meshes/Enemies", center=(8, 0, 0))

    setup_lighting()

    unreal.EditorLevelLibrary.save_current_level()

    total = arena_count + vfx_count + char_count + weapon_count + enemy_count
    unreal.log(f"=== Showcase level complete: {total} meshes + production lighting ===")


main()
