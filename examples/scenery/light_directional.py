#! /usr/bin/python

# Demonstrates the use of point light
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 320
height = 240
super_sampling = 9


red = textures.Constant(rgb(1, 0, 0))
blue = textures.Constant(rgb(0, 0, 1))
white = textures.Constant(rgb(1, 1, 1))

sphere = scenery.Sphere()
sphere.center = (0, 0, 1)
sphere.radius = 1
sphere.shader = shaders.Simple(red, white)
sphere.shader.specularPower = textures.Constant(20)

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Simple()
floor.shader.diffuse = textures.CheckerBoard(blue, white)
floor.shader.specular = white
floor.shader.specularPower = textures.Constant(20)

light = scenery.LightDirectional()
light.direction = (-1, -1, -1)
light.radiance = rgb(tuple([1] * 3))

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))

image = output.Image("light_directional.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, sphere, light])
engine.camera = camera
engine.target = image
engine.render()
