param(
  [string]$BlenderExe = "C:\Program Files\Blender Foundation\Blender 4.5\blender.exe",
  [string]$AddonPath = "D:\TE-Code\blender-mcp\addon.py",
  [int]$Port = 9876
)

$ErrorActionPreference = "Stop"

if (!(Test-Path -LiteralPath $BlenderExe)) {
  throw "Blender executable not found: $BlenderExe"
}

if (!(Test-Path -LiteralPath $AddonPath)) {
  throw "Blender MCP addon not found: $AddonPath"
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$scriptDir = Join-Path $projectRoot "tools\.generated"
New-Item -ItemType Directory -Force -Path $scriptDir | Out-Null

$bootstrapPath = Join-Path $scriptDir "bootstrap_blender_mcp.py"
$addonPathForPython = $AddonPath.Replace("\", "\\")

@"
import bpy
import os

addon_path = r"$addonPathForPython"
port = $Port

bpy.ops.preferences.addon_install(filepath=addon_path, overwrite=True)
bpy.ops.preferences.addon_enable(module="addon")

bpy.context.scene.blendermcp_port = port
bpy.context.scene.blendermcp_auto_start_server = True

if not hasattr(bpy.types, "blendermcp_server") or not bpy.types.blendermcp_server:
    bpy.ops.blendermcp.start_server()
elif not bpy.types.blendermcp_server.running:
    bpy.types.blendermcp_server.start()

bpy.ops.wm.save_userpref()
print(f"Blender MCP requested on 127.0.0.1:{port}")
"@ | Set-Content -LiteralPath $bootstrapPath -Encoding UTF8

Start-Process -FilePath $BlenderExe -ArgumentList @("--python", $bootstrapPath) -WindowStyle Minimized
Start-Sleep -Seconds 8

$connection = Test-NetConnection -ComputerName 127.0.0.1 -Port $Port -WarningAction SilentlyContinue
if ($connection.TcpTestSucceeded) {
  Write-Output "Blender MCP is listening on 127.0.0.1:$Port"
  exit 0
}

Write-Error "Blender MCP did not start on 127.0.0.1:$Port. Blender may need manual addon enable/start from the UI."
