from render_liar import LIAR
import bpy
from bpy import props
import sys


enum_aperture_types = (
    ('RADIUS', "Radius", "Directly change the size of the aperture"),
    ('FSTOP', "F/stop", "Change the size of the aperture by f/stops"),
    )


class LiarCameraSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        import math

        bpy.types.Camera.liar = props.PointerProperty(
                name="LiAR Camera Settings",
                description="LiAR camera settings",
                type=cls,
                )

        cls.aperture_type = props.EnumProperty(
                name="Aperture Type",
                description="Use F/stop number or aperture radius",
                items=enum_aperture_types,
                default='RADIUS',
                )
        cls.aperture_fstop = props.FloatProperty(
                name="Aperture F/stop",
                description="F/stop ratio (lower numbers give more defocus, higher numbers give a sharper image)",
                min=0.0, soft_min=0.1, soft_max=64.0,
                default=5.6,
                step=10,
                precision=1,
                )
        cls.aperture_size = props.FloatProperty(
                name="Aperture Size",
                description="Radius of the aperture for depth of field (higher values give more defocus)",
                min=0.0, soft_max=10.0,
                default=0.0,
                step=1,
                precision=4,
                subtype='DISTANCE',
                )

    @classmethod
    def unregister(cls):
        del bpy.types.Camera.liar



def register():
    bpy.utils.register_class(LiarCameraSettings)
    
    
def unregister():
    bpy.utils.unregister_class(LiarCameraSettings)