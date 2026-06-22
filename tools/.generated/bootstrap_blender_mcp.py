import bpy
import os

addon_path = r"D:\\TE-Code\\blender-mcp\\addon.py"
port = 9876

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
