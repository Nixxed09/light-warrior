# Light Warrior Environment Setup

## Purpose

This project has two tracks:

1. Prototype track: Vite + TypeScript + Three.js. This exists now and is used to test the first playable quickly.
2. Production track: Unreal Engine 5. This is the intended engine for the commercial-quality vertical slice.

The prototype can continue only as a learning artifact. The production vertical slice must move into UE5 once the machine is ready.

## Current Verified State

As of 2026-06-16:

- `npm run build` passes.
- `node_modules` exists.
- Existing screenshots exist in `captures/`.
- ProductOS and image-engine repos exist under `D:\TE-Code`.
- video-engine and audio-engine repos exist under `D:\TE-Code`.
- Blender LTS 4.5.5 is installed.
- Epic Games Launcher is installed.
- image-engine, video-engine, and audio-engine base dependencies have been installed.
- image-engine, video-engine, and audio-engine have been started successfully in this session.
- UE5 is installed at `D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe`.
- Visual Studio Build Tools 2022 C++ workload is installed.
- Windows SDK 10.0.18362 is installed and provides `C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8`.
- `engine/LightWarrior.uproject` and first C++ production classes now exist. External command-line build is currently blocked while Unreal Editor/LiveCodingConsole are open with Live Coding active.
- Blender MCP is listening on port `9876`.
- video-engine reports Blender connected and FFmpeg available.
- Codex MCP config does not currently show Godot/Blender/MCP hints.

## Prototype Track

### Install Dependencies

```powershell
npm install
```

### Verify

```powershell
.\tools\verify.ps1
```

### Run Dev Server

```powershell
npm run dev
```

### Production Build

```powershell
npm run build
```

## Production UE5 Track

### Install Unreal Engine 5

Install through Epic Games Launcher.

Existing Launcher path:

```text
C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe
```

Recommended:

- UE 5.4 or newer.
- Include starter content only if useful for blockout.
- Enable Blueprint and C++ support only if needed.

Expected common install path:

```text
C:\Program Files\Epic Games\UE_5.x\Engine\Binaries\Win64\UnrealEditor.exe
```

After install, verify:

```powershell
Get-ChildItem "C:\Program Files\Epic Games" -Recurse -Filter UnrealEditor.exe -ErrorAction SilentlyContinue |
  Select-Object -First 5 -ExpandProperty FullName
```

### Required UE5 Project Target

The project shell lives at:

```text
D:\Phoenix\nix-code\games\light-warrior\engine\LightWarrior.uproject
```

The first C++ production classes live under:

```text
D:\Phoenix\nix-code\games\light-warrior\engine\Source\LightWarrior
```

### UE5 Verification Gate

Before marking `engine.ue5_shell` complete:

- Unreal project opens.
- Empty arena level runs in Play-In-Editor.
- A screenshot or video exists in `captures/`.
- `PRODUCTION_LOG.md` records engine version and verification result.
- `TASK_GRAPH.json` updates `engine.ue5_shell` from `blocked` to `passed` only after evidence.

## Asset Toolchain

Install/prepare when moving beyond prototype:

- Blender for 3D blockouts/assets. Installed path:

  ```text
  C:\Program Files\Blender Foundation\Blender 4.5\blender.exe
  ```

- Blender MCP or equivalent automation.
- image-engine for 2D concepts/UI/icon explorations.
- audio-engine for SFX/music.
- video-engine or Blender workflow for 3D asset generation.

See `MEDIA_ENGINE_INTEGRATION.md` for the project-specific contract across image-engine, video-engine, audio-engine, and Blender MCP.

Expected ports from ProductOS:

| Service | Port |
|---|---:|
| image-engine | 3100 |
| video-engine | 8200 |
| audio-engine | 8300 |
| Blender MCP | 9876 |

## Validation Commands

From ProductOS:

```powershell
powershell -ExecutionPolicy Bypass -File D:\TE-Code\ProductOS\scripts\validate_gamesos_design_foundation.ps1 -GameProject D:\Phoenix\nix-code\games\light-warrior
```

```powershell
powershell -ExecutionPolicy Bypass -File D:\TE-Code\ProductOS\scripts\validate_gamesos_toolchain.ps1 -ProductRoot D:\TE-Code\ProductOS -CodeRoot D:\TE-Code -GameProject D:\Phoenix\nix-code\games\light-warrior
```

Expected today:

- Design foundation should pass.
- Toolchain will still warn/fail for missing UE5, Whisper/PrusaSlicer, and unconfigured generation providers.
