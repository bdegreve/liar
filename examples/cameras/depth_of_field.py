#! /usr/bin/env python

# Demonstrates the use of depth of field
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
#
# http://liar.bramz.net/


from liar import *
from liar.tools import scripting

options = scripting.renderOptions(width=900, height=600, samples_per_pixel=36)

lens_radius = 0.5
sphere_radius = 0.8
grid_size = 20
grid_step = 2

# textures

red = textures.Constant(rgb(1, 0, 0))
blue = textures.Constant(rgb(0, 0, 1))
white = textures.Constant(rgb(1, 1, 1))
blue_checkers = textures.CheckerBoard(blue, white)

# scenery

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Lambert(blue_checkers)

sphere = scenery.Sphere((0, 0, sphere_radius), sphere_radius)
sphere.shader = shaders.AshikhminShirley(red, textures.Constant(.01))
sphere.shader.specularPowerU = sphere.shader.specularPowerV = textures.Constant(20)

spheres = []
for i in range(0, grid_size, grid_step):
	for j in range(0, grid_size, grid_step):
		spheres.append(scenery.Translation(sphere, (i, j, 0)))

keyLight = scenery.LightPoint()
keyLight.position = (0, -10, 10)
keyLight.intensity = rgb(250, 250, 150)

fillLight = scenery.LightPoint()
fillLight.position = (2 * grid_size, -10, 10)
fillLight.intensity = rgb(100, 100, 150)

backLight = scenery.LightPoint()
backLight.position = (10, 10, 10)
backLight.intensity = rgb(50, 50, 100)

# setup

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((options.width, options.height), options.samples_per_pixel)
engine.scene = scenery.List([scenery.AabbTree(spheres), floor, keyLight, fillLight, backLight])

camera = cameras.PerspectiveCamera()
camera.position = (grid_size / 2, -6, 6)
camera.lookAt((grid_size / 2, grid_size / 3, 1))
engine.camera = camera

engine.target = scripting.makeRenderTarget(options.width, options.height, "depth_of_field.hdr", "Camera Depth Of Field")

camera.lensRadius = 0
engine.render(((0, 0), (0.5, 1)))
camera.lensRadius = lens_radius
engine.render(((0.5, 0), (1, 1)))
