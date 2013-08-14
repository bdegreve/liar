from render_liar import LIAR

import bpy
from bpy.types import Panel, Menu, Operator


class LiarButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine == LIAR


class LiarCamera_PT_dof(LiarButtonsPanel, Panel):
    bl_label = "Depth of Field"
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        return context.camera and LiarButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout

        cam = context.camera
        lcam = cam.liar

        split = layout.split()

        col = split.column()
        col.label("Focus:")
        col.prop(cam, "dof_object", text="")

        sub = col.row()
        sub.active = cam.dof_object is None
        sub.prop(cam, "dof_distance", text="Distance")

        col = split.column()

        col.label("Aperture:")
        sub = col.column(align=True)
        sub.prop(lcam, "aperture_type", text="")
        if lcam.aperture_type == 'RADIUS':
            sub.prop(lcam, "aperture_size", text="Size")
        elif lcam.aperture_type == 'FSTOP':
            sub.prop(lcam, "aperture_fstop", text="Number")


def get_panels():
    from bpy import types
    return (
        types.RENDER_PT_render,
        types.DATA_PT_context_camera,
        )


def register():
    for panel in get_panels():
        panel.COMPAT_ENGINES.add(LIAR)


def unregister():
    for panel in get_panels():
        panel.COMPAT_ENGINES.remove(LIAR)