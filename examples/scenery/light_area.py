# Demonstrates the use of area light
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.sourceforge.net

from liar import *

if 0:
	width = 800
	height = 600
	super_sampling = 9
else:
	width = 320
	height = 240
	super_sampling = 4


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

I = 40
s = .5
h = 4
surface = scenery.Sphere((0, 0, h), s)
light = scenery.LightArea(surface)
light.radiance = rgb(I, I, 0.5 * I)
light.numberOfEmissionSamples = 16
light.shader = shaders.Unshaded(textures.Constant(light.radiance))

I = 0.1
sky = scenery.LightArea(scenery.Sky())
sky.radiance = rgb(0.5 * I, 0.5 * I, I)
sky.numberOfEmissionSamples = 36
sky.shader = shaders.Unshaded(textures.Constant(sky.radiance))

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))
camera.direction = (0, 0, 1)

image = output.Image("light_area.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, sphere, light, sky])
engine.camera = camera
engine.target = image
engine.render()
