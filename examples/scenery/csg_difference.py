# Demonstrates the use of textures.LinearInterpolator to blend
# between textures using textures.Time as parameter
#
# The result is that the color of the sphere changes in time.
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.sourceforge.net

from liar import *

width = 320
height = 240
super_sampling = 9


red = textures.Constant(rgb(1, 0, 0))
blue = textures.Constant(rgb(0, 0, 1))
black = textures.Constant(rgb(0, 0, 0))
white = textures.Constant(rgb(1, 1, 1))

sphereA = scenery.Sphere()
sphereA.center = (0, 0, 1)
sphereA.radius = 1
sphereA.shader = shaders.Simple(red, white)
sphereA.shader.specularPower = textures.Constant(20)

sphereB = scenery.Sphere()
sphereB.center = (0, 1, 1)
sphereB.radius = 0.5
sphereB.shader = shaders.Simple(blue, white)
sphereB.shader.specularPower = textures.Constant(20)

csg = scenery.Csg(sphereA, sphereB, scenery.Csg.UNION)

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Simple()
floor.shader.diffuse = textures.GridBoard(black, white)
floor.shader.specular = textures.GridBoard(white, black)
floor.shader.specularPower = textures.Constant(20)

light = scenery.LightPoint()
light.position = (10, 10, 10)
light.power = rgb(tuple([1000] * 3))

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))

image = output.Image("csg_difference.tga", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, csg, light])
engine.camera = camera
engine.target = image
engine.render()
