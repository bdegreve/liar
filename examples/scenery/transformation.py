# Demonstrates the use of transformations
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 440
height = 330
super_sampling = 16

red = textures.Constant(rgb(1, 0, 0))
black = textures.Constant(rgb(0, 0, 0))
white = textures.Constant(rgb(1, 1, 1))

checkers = shaders.Lambert(textures.CheckerBoard(black, white))
checkers3D = shaders.Simple(textures.CheckerVolume(red, white), white)
checkers3D.specularPower = textures.Constant(20)

floor = scenery.Plane((0, 0, 1), 0)
floor.shader = checkers

sphere = scenery.Sphere((0, 0, 1), 1)
sphere.shader = checkers3D

transformed_sphere = scenery.Transformation(sphere,
	[[2, 0, 0, 3.5],
	 [0, 2, 0, 0],
	 [0, 0, 2, 0],
	 [0, 0, 0, 1]])

transformed_sphere_forcing_shader = scenery.Transformation(sphere,
	[[2, 0, 0, -3.5],
	[0, 2, 0, 0],
	[0, 0, 2, 0],
	[0, 0, 0, 1]])
transformed_sphere_forcing_shader.shader = checkers3D
transformed_sphere_forcing_shader.isOverridingShader = True

light1 = scenery.LightArea(scenery.Sphere((5, 10, 10), 1))
light1.radiance = rgb(3, 3, 2)
light1.numberOfEmissionSamples = 16

light2 = scenery.LightArea(scenery.Sphere((-5, 10, 10), 1))
light2.radiance = rgb(2, 2, 3)
light2.numberOfEmissionSamples = 16

camera = cameras.PerspectiveCamera()
camera.position = (0, 6, 2)
camera.lookAt((0, 0, 2))
camera.lensRadius = 0.02

image = output.Image("transformation.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, sphere, transformed_sphere, transformed_sphere_forcing_shader, light1, light2])
engine.camera = camera
engine.target = image
engine.render()
