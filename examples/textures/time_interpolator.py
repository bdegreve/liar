#! /usr/bin/env python

# Demonstrates the use of textures.LinearInterpolator to blend
# between textures using textures.Time as parameter
#
# The result is that the color of the sphere changes in time.
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 320
height = 240
super_sampling = 9
time_length = 3.0
frame_rate = 30.0

red = textures.Constant(rgb(1, 0, 0))
yellow = textures.Constant(rgb(1, 1, 0))
green = textures.Constant(rgb(0, 1, 0))
cyan = textures.Constant(rgb(0, 1, 1))
blue = textures.Constant(rgb(0, 0, 1))
purple = textures.Constant(rgb(1, 0, 1))
black = textures.Constant(rgb(0, 0, 0))
white = textures.Constant(rgb(1, 1, 1))

# time dependent texture
#
key_textures = [red, yellow, green, cyan, blue, purple, red] # the textures to blend
key_times = [k * time_length / (len(key_textures) - 1) for k in range(len(key_textures))]
time_interpolator = textures.LinearInterpolator()
time_interpolator.keys = zip(key_times, key_textures)
time_interpolator.control = textures.Time()

# use time series on sphere
#
sphere = scenery.Sphere()
sphere.center = (0, 0, 1)
sphere.radius = 1
sphere.shader = shaders.Simple(time_interpolator, white)
sphere.shader.specularPower = textures.Constant(20)

plane = scenery.Plane()
plane.normal = (0, 0, 1)
plane.d = 0
plane.shader = shaders.Lambert(textures.GridBoard(black, white))

light = scenery.LightPoint()
light.position = (10, 10, 10)
light.power = rgb(tuple([1000] * 3))

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))

image = output.Image("", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([plane, sphere, light])
engine.camera = camera
engine.target = image

for k in range(int(time_length * frame_rate)):
    t = k / frame_rate
    print "frame %03d: %0.2f s" % (k, t)
    image.filename = "time_series.%03d.tga" % k
    engine.sampler.seed(0)
    engine.render(t)
