'''
Geometry for the output examples

LiAR isn't a raytracer
Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
http://liar.bramz.net/
'''

from liar import *

if False:
    width = 1350
    height = 900
    super_sampling = 100
else:
    width = 800
    height = 600
    super_sampling = 1

def scene():
    red = textures.Constant(rgb(0.9, 0.1, 0.1))
    blue = textures.Constant(rgb(0.1, 0.1, .9))
    white = textures.Constant(rgb(.9, .9, .9))

    parts = []
    sphere = scenery.Sphere()
    sphere.center = (0, 0, 1)
    sphere.radius = 1
    sphere = scenery.Goursat(0, -5, +11.8)
    sphere.useSphereTracing = False
    sphere.shader = shaders.Lambert(red)

    sphere.shader = shaders.AshikhminShirley()
    sphere.shader.diffuse = red
    sphere.shader.specular = textures.Constant(0.05)
    sphere.shader.specularPowerU = sphere.shader.specularPowerV = textures.Constant(1000)

    #sphere.shader = shaders.Dielectric()
    #sphere.shader.innerRefractionIndex = textures.Constant(2)
    #sphere.interior = mediums.Beer(rgb(0.5, 0.5, 0))


    #sphere = scenery.MotionTranslation(sphere, (0, 0, 0), (1, 0, 0))


    parts.append(sphere)

    floor = scenery.Plane()
    floor.normal = (0, 0, 1)
    floor.d = 2.3
    floor.shader = shaders.Lambert(textures.CheckerBoard(blue, white))  
    parts.append(floor)

    back = scenery.Plane()
    back.normal = (0, 1, 0)
    back.d = 5
    back.shader = shaders.Lambert(textures.CheckerBoard(blue, white))
    parts.append(back)

    #floor.shader = back.shader = shaders.AshikhminShirley()
    #floor.shader.diffuse = textures.CheckerBoard(blue, white)
    #floor.shader.specular = textures.Constant(0.05)
    #floor.shader.specularPowerU = floor.shader.specularPowerV = textures.Constant(100)

    
    light_shape = scenery.Disk()
    light_shape.center = (0, 0, 6)
    light_shape.normal = (0, 0, -1)
    light_shape.radius = 2
    light = scenery.LightArea(light_shape)
    light.radiance = rgb(tuple([1000000] * 3))
    parts.append(light)
    
    return scenery.List(parts)

def camera():
    cam = cameras.PerspectiveCamera()
    cam.position = (0, 5, 3)
    cam.lookAt((0, 0, 0))
    cam.farLimit = 15
    cam.shutterCloseDelta = 0.3
    return cam

def engine():
    eng = RenderEngine()
    eng.tracer = tracers.DirectLighting()
    eng.sampler = samplers.Stratifier((width, height), super_sampling)
    eng.scene = scene()
    eng.camera = camera()
    return eng

