'''
Geometry for the output examples

LiAR isn't a raytracer
Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
http://liar.bramz.net/
'''

from liar import *

width = 400
height = 300
super_sampling = 9

def scene():
    red = textures.Constant(rgb(1, 0, 0))
    blue = textures.Constant(rgb(0, 0, 1))
    white = textures.Constant(rgb(1, 1, 1))

    sphere = scenery.Sphere()
    sphere.center = (0, 0, 1)
    sphere.radius = 1
    sphere.shader = shaders.Lambert(red)

    floor = scenery.Plane()
    floor.normal = (0, 0, 1)
    floor.d = 0
    floor.shader = shaders.Lambert(textures.CheckerBoard(blue, white))  

    back = scenery.Plane()
    back.normal = (0, 1, 0)
    back.d = 4
    back.shader = shaders.Lambert(textures.CheckerBoard(blue, white))
    
    light_shape = scenery.Disk()
    light_shape.center = (0, 0, 6)
    light_shape.normal = (0, 0, -1)
    light_shape.radius = 2

    light = scenery.LightArea(light_shape)
    light.radiance = rgb(tuple([1000000] * 3))
    
    return scenery.List([floor, back, sphere, light])

def camera():
    cam = cameras.PerspectiveCamera()
    cam.position = (0, 4, 2)
    cam.lookAt((0, 0, 1))
    return cam

def engine():
    eng = RenderEngine()
    eng.tracer = tracers.DirectLighting()
    eng.sampler = samplers.Stratifier((width, height), super_sampling)
    eng.scene = scene()
    eng.camera = camera()
    return eng

