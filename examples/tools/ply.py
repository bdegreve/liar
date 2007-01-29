#! /usr/bin/env python

# Demonstrates the use module liar.tools.wavefront_obj to load
# a wavefront OBJ file
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.sourceforge.net

from liar import *
from liar.tools import ply

width = 320
height = 240
super_sampling = 4

red = textures.Constant(rgb(1, 0, 0))
white = textures.Constant(rgb(1, 1, 1))
black = textures.Constant(rgb(0, 0, 0))

cube = ply.load("ply.ply")
cube.shader = shaders.Lambert(red)

floor = scenery.Plane((0, 0, 1), 1)
floor.shader = shaders.Lambert(textures.CheckerBoard(black, white))

light1 = scenery.LightPoint()
light1.position = (5, 10, 10)
light1.power = rgb(1, 1, .5)

light2 = scenery.LightPoint()
light2.position = (10, -10, 10)
light2.power = rgb(.5, .5, .1)

light3 = scenery.LightPoint()
light3.position = (-10, -10, 10)
light3.power = rgb(.5, .5, .5)

camera = cameras.PerspectiveCamera()
camera.position = (1.5, 2.5, 1.5)
camera.lookAt((0, 0, 0))

image = output.Image("ply.tga", (width, height))
image.exposureTime = 2500

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([cube, floor, light1, light2, light3])
engine.camera = camera
engine.target = image
engine.render()
