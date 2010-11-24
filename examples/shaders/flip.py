#! /usr/bin/env python

# Demonstrates the use of shaders.Simple
#
# shaders.Simple can be seen as the traditional raytrace shader.
# this scene sets a few spheres, each with a different use of the shader
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

#print tolerance()
setTolerance(10e-6)

width = 320
height = 240
samples_per_pixel = 9

# textures

red = textures.Constant(rgb(1, 0, 0))
blue = textures.Constant(rgb(0, 0, 1))
yellow = textures.Constant(rgb(1, 1, 0))
white = textures.Constant(rgb(1, 1, 1))
blue_checkers = textures.CheckerBoard(blue, white)
yellow_checkers = textures.CheckerBoard(yellow, white)

# shaders 

shader_list = [
	shaders.Flip(shaders.Mirror(red)),
	shaders.Sum([
		shaders.Lambert(textures.Constant(rgb(0.4, 0.4, 0.4))),
		shaders.Flip(shaders.Lambert(textures.Constant(rgb(0.4, 0.4, 0.4))))]),
	shaders.Flip(shaders.Lambert(white)),
]

# scenery

scene_list = []

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Lambert(blue_checkers)
scene_list.append(floor)

backdrop = scenery.Plane()
backdrop.normal = (0, 1, 0)
backdrop.d = 2
backdrop.shader = shaders.Lambert(yellow_checkers)
scene_list.append(backdrop)

dx = -2.5
x0 = -(len(shader_list) - 1.0) * dx / 2.0
for i in range(len(shader_list)):
	disk = scenery.Disk()
	disk.center = (x0 + i * dx, 0, 1)	
	disk.normal = (0, -1, 0)
	disk.radius = 1
	disk.shader = shader_list[i]
	scene_list.append(disk)

light = scenery.LightPoint()
light.position = (0, -1, 2)
light.intensity = rgb(1e4, 1e4, 1e4)
scene_list.append(light)

light = scenery.LightPoint()
light.position = (0, 2, 5)
light.intensity = rgb(1e4, 1e4, 1e4)
scene_list.append(light)

# setup

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), samples_per_pixel)
engine.scene = scenery.List(scene_list)

camera = cameras.PerspectiveCamera()
camera.position = (0, 3, 1)
camera.lookAt((0, 0, 1))
engine.camera = camera

engine.numberOfThreads = 1

engine.target = output.Display("flip", (width, height))

engine.render()
