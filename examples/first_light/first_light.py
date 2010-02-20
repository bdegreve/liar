# http://en.wikipedia.org/wiki/First_light
#
# This script is used to generate the very first image rendered by LiAR.
# It's kept in its original form for its "historical value", so some day in
# the future it may not work anymore.  That's why we include the image
# as well ;)
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2005  Bram de Greve

from liar import *

red = textures.Constant((1, 0, 0))
blue = textures.Constant((0, 0, 1))
white = textures.Constant((1, 1, 1))
checkers = textures.CheckerBoard(blue, white)

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier()

plane = scenery.Plane()
plane.normal = (0, 0, 1)
plane.d = 0
plane.shader = shaders.Lambert(checkers)

sphere = scenery.Sphere()
sphere.center = (0, 0, 1)
sphere.radius = 1
sphere.shader = shaders.Lambert(red)

light = scenery.LightPoint()
light.position = (10, 10, 10)
light.power = tuple([1000] * 3)

engine.scene = scenery.List([plane, sphere, light])

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))
engine.camera = camera

engine.target = output.Image("first_light.tga", (320, 240))

engine.render()
