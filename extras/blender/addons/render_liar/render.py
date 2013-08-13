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

import bpy
from render_liar import LiarAddon, liar_log


@LiarAddon.addon_register_class
class RENDERENGINE_liar(bpy.types.RenderEngine):
    '''
    LiAR Engine Exporter/Integration class
    '''
    
    bl_idname = LiarAddon.BL_IDNAME
    bl_label = 'LiAR'
    bl_use_preview = False
    
    def render(self, scene):
        #pydevd.settrace('localhost', port=1234, stdoutToServer=True, stderrToServer=True)
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        if scene.name == 'preview':
            liar_log("render preview not supported")
            return
        else:
            self.__render_scene(scene)

    def __render_scene(self, scene):
        from render_liar import render_helper
        from imp import reload
        reload(render_helper)
        render_helper.render_helper(self, scene)

        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        blue_rect = [[0.0, 0.0, 1.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0]
        layer.rect = blue_rect

        self.end_result(result)

# RenderEngines also need to tell UI Panels that they are compatible
# Otherwise most of the UI will be empty when the engine is selected.
# In this example, we need to see the main render image button and
# the material preview panel.
from bl_ui import properties_render
properties_render.RENDER_PT_render.COMPAT_ENGINES.add(LiarAddon.BL_IDNAME)
del properties_render

from bl_ui import properties_material
properties_material.MATERIAL_PT_preview.COMPAT_ENGINES.add(LiarAddon.BL_IDNAME)
del properties_material

