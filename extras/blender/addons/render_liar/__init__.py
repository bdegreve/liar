# LiAR isn't a raytracer
# Copyright (C) 2004-2011  Bram de Greve (bramz@users.sourceforge.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# http://liar.bramz.net/

LIAR = "LIAR"

# http://wiki.blender.org/index.php/Dev:2.5/Py/Scripts/Guidelines/Addons
bl_info = {
    "name": "LiAR isn't A Raytracer",
    "description": "LiAR render engine integration",
    "author": "Bram de Greve",
    "version": (0, 0, 1),
    "blender": (2, 67, 0),
    "location": "Render > Engine > LiAR isn't A Raytracer",
    "warning": "alpha alert!",
    "category": "Render"
    }

import bpy
import sys

#import sys
#sys.path.append(r"C:\local\winpdb-1.4.8")
#import rpdb2; 
#rpdb2.start_embedded_debugger("liar")


class LiarPreferences(bpy.types.AddonPreferences):
    bl_idname = __name__
    python_binary = bpy.props.StringProperty(name="Python Binary",
                                             description="Filepath of Python binary that can run LiAR engine",
                                             subtype='FILE_PATH',
                                             default='python.exe' if 'win' in sys.platform else 'python')
    def draw(self, context):
        layout = self.layout
        layout.prop(self, "python_binary")


class LiarRender(bpy.types.RenderEngine):   
    bl_idname = LIAR
    bl_label = 'LiAR Render'
    bl_use_shading_nodes = True 
    bl_use_preview = False
    bl_use_exclude_layers = True
    bl_use_save_buffers = True
    
    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        if scene.name == 'preview':
            self.report({'ERROR'}, "render preview not supported")
            return
        else:
            self._render_scene(scene)

    def _render_scene(self, scene):
        from render_liar import render, export
        
        from imp import reload
        reload(render)
        reload(export)
        
        render.render_scene(self, scene)

        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        blue_rect = [[0.0, 0.0, 1.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0]
        layer.rect = blue_rect

        self.end_result(result)


def register():
    from render_liar import ui, properties
    
    from imp import reload
    reload(ui)
    reload(properties)
        
    properties.register()
    ui.register()
    bpy.utils.register_module(__name__)


def unregister():
    from render_liar import ui, properties
    ui.unregister()
    properties.unregister()
    bpy.utils.unregister_module(__name__)

