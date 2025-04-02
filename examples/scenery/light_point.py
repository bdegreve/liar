# Demonstrates the use of transformations
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 320
height = 240
super_sampling = 4

red = textures.Constant(rgb(1, 0, 0))
black = textures.Constant(rgb(0, 0, 0))
white = textures.Constant(rgb(1, 1, 1))
checkers = textures.CheckerBoard(black, white)
checkers3D = textures.CheckerVolume(red, white)

floor = scenery.Plane((0, 0, 1), 0)
floor.shader = shaders.Lambert(checkers)

sphere = scenery.Sphere((0, 0, 1), 1)
sphere.shader = shaders.Simple(checkers3D, white)
sphere.shader.specularPower = textures.Constant(20)

transformed_sphere = scenery.Transformation(sphere,
	[[2, 0, 0, 3.5],
	 [0, 2, 0, 0],
	 [0, 0, 2, 0],
	 [0, 0, 0, 1]])

light = scenery.LightDirectional()
light.direction = (-1, -1, -1)
light.radiance = rgb(3, 3, 3)

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))

image = output.Display("light_point.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, sphere, transformed_sphere, light])
engine.camera = camera
engine.target = image
engine.render()
