#! /usr/bin/env python

# Demonstrates the use of texture anti aliasing to improve render quality.
#
# The result is that the color of the sphere changes in time.
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 320
height = 240
super_sampling = 4

red = textures.Constant(rgb(1, 0, 0))
blue = textures.Constant(rgb(0, 0, 1))
white = textures.Constant(rgb(1, 1, 1))

checkers = textures.CheckerBoard(blue, white)
checkers.antiAliasing = "bilinear"

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)

plane = scenery.Plane()
plane.normal = (0, 0, 1)
plane.d = 0
plane.shader = shaders.Simple(checkers)

sphereR = scenery.Sphere()
sphereR.center = (1.2, 0, 1)
sphereR.radius = 1
sphereR.shader = shaders.Simple()
sphereR.shader.reflectance = textures.Constant(rgb(0.9, 0.9, 0.9))

sphereT = scenery.Sphere()
sphereT.center = (-1.2, 0, 1)
sphereT.radius = 1
sphereT.shader = shaders.Simple()
sphereT.shader.transmittance = textures.Constant(rgb(0.9, 0.9, 0.9))
sphereT.shader.refractionIndex = textures.Constant(1.5)

light1 = scenery.LightPoint()
light1.position = (10, 10, 10)
light1.power = rgb(tuple([x * 10**6 for x in(1, 1, 0.5)]))

light2 = scenery.LightPoint()
light2.position = (10, -20, 10)
light2.power = rgb(tuple([x * 10**6 for x in(1, 1, 0.5)]))

engine.scene = scenery.List([plane, sphereR, sphereT, light1, light2])

camera = cameras.PerspectiveCamera()
camera.position = (0, 3, 2)
camera.lookAt((0, 0, 1))
engine.camera = camera

engine.target = output.Image("anti_aliasing.tga", (width, height))
engine.target.exposureTime = .005

engine.render()
