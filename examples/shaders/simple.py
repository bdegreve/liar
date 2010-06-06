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

shader_list = []

diffuseShader = shaders.Simple()
diffuseShader.diffuse = red
shader_list.append(diffuseShader)

specularShader = shaders.Simple()
diffuseShader.diffuse = red
specularShader.specular = white
specularShader.specularPower = textures.Constant(20)
shader_list.append(specularShader)

reflectiveShader = shaders.Simple()
reflectiveShader.reflectance = white
shader_list.append(reflectiveShader)

transmittiveShader = shaders.Simple()
transmittiveShader.transmittance = white
transmittiveShader.refractionIndex = textures.Constant(1.1)
shader_list.append(transmittiveShader)

# scenery

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Simple(blue_checkers)

backdrop = scenery.Plane()
backdrop.normal = (0, 1, 0)
backdrop.d = 2
backdrop.shader = shaders.Simple(yellow_checkers)

dx = -2.5
x0 = -(len(shader_list) - 1.0) * dx / 2.0
sphere_list = []
for i in range(len(shader_list)):
	sphere = scenery.Sphere()
	sphere.center = (x0 + i * dx, 0, 1)	
	sphere.radius = 1
	sphere.shader = shader_list[i]
	sphere_list.append(sphere)

light = scenery.LightPoint()
light.position = (10, 10, 10)
light.power = rgb(tuple([10**7] * 3))

# setup

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), samples_per_pixel)
engine.scene = scenery.List(sphere_list + [floor, backdrop, light])

camera = cameras.PerspectiveCamera()
camera.position = (0, 6, 1)
camera.lookAt((0, 0, 1))
engine.camera = camera

engine.target = output.Image("simple.tga", (width, height))
engine.target.exposureTime = .01/30

engine.render()
