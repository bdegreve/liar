from render_liar import LIAR, auto_register, export
import bpy
from bpy_extras.io_utils import ExportHelper
import os

@auto_register
class EXPORT_OT_liar(bpy.types.Operator, ExportHelper):
    """Export scene as a LiAR python script"""
    bl_idname = 'export.liar'
    bl_label = "Export script"

    filename_ext = "_liar.py"
    filter_glob = bpy.props.StringProperty(default='*.py', options={'HIDDEN'})
    scene = bpy.props.StringProperty(options={'HIDDEN'}, default='')  # Specify scene to export
    check_extension = True

    def execute(self, context):
        if not self.properties.scene:
            scene = context.scene
        else:
            scene = bpy.data.scenes[self.properties.scene]
        script_path = self.properties.filepath
        export.export_scene(scene, script_path)
        return {'FINISHED'}


def draw(self, context):
    self.layout.operator("export.liar", text="LiAR isn't A Raytracer (.py)")

bpy.types.INFO_MT_file_export.append(draw)
