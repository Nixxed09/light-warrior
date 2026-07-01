import json
import os
from pathlib import Path

import unreal


PROJECT_ROOT = Path(__file__).resolve().parents[1]
AUDIO_PACKAGE = PROJECT_ROOT / "assets" / "generated" / "audio" / "core_sfx"
METADATA_PATH = AUDIO_PACKAGE / "audio_metadata.json"
UE_DESTINATION = "/Game/LightWarrior/Audio/CoreSfx"
IMPORT_RESULT_PATH = AUDIO_PACKAGE / "ue-import-result.json"


def load_metadata():
    if not METADATA_PATH.exists():
        raise RuntimeError(f"Missing audio metadata: {METADATA_PATH}")

    with METADATA_PATH.open("r", encoding="utf-8-sig") as handle:
        return json.load(handle)


def import_wav(source_path: Path):
    task = unreal.AssetImportTask()
    task.filename = str(source_path)
    task.destination_path = UE_DESTINATION
    task.automated = True
    task.save = True
    task.replace_existing = True

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:
        raise RuntimeError(f"UE did not import {source_path}")

    return list(task.imported_object_paths)


def main():
    metadata = load_metadata()
    imported = []
    errors = []

    unreal.EditorAssetLibrary.make_directory(UE_DESTINATION)

    for cue in metadata.get("files", []):
        rel_file = cue.get("file")
        if not rel_file:
            errors.append({"id": cue.get("id"), "error": "missing file path"})
            continue

        source_path = PROJECT_ROOT / rel_file
        if not source_path.exists():
            errors.append({"id": cue.get("id"), "file": rel_file, "error": "missing wav"})
            continue

        try:
            imported_paths = import_wav(source_path)
            imported.append(
                {
                    "id": cue.get("id"),
                    "event": cue.get("event"),
                    "source": rel_file.replace(os.sep, "/"),
                    "ue_assets": imported_paths,
                }
            )
        except Exception as exc:
            errors.append({"id": cue.get("id"), "file": rel_file, "error": str(exc)})

    result = {
        "asset_id": metadata.get("asset_id", "audio.core_sfx"),
        "status": "imported" if imported and not errors else "import_failed",
        "target": UE_DESTINATION,
        "imported": imported,
        "errors": errors,
        "note": "C++ runtime maps ELightWarriorSfx events to these imported assets and falls back to procedural tones if an asset is missing.",
    }

    with IMPORT_RESULT_PATH.open("w", encoding="utf-8") as handle:
        json.dump(result, handle, indent=2)

    unreal.log(f"Light Warrior audio import result: {result['status']} ({len(imported)} imported, {len(errors)} errors)")
    if errors:
        raise RuntimeError(f"Audio import completed with errors: {errors}")


if __name__ == "__main__":
    main()
