# Demonstrates the use of point light
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *

width = 320
height = 240
super_sampling = 9


black = textures.Constant(rgb(0, 0, 0))
white = textures.Constant(rgb(1, 1, 1))

sphere = scenery.Sphere()
sphere.center = (0, 0, 1)
sphere.radius = 1
sphere.shader = shaders.Simple(white, white)
sphere.shader.specularPower = textures.Constant(20)

floor = scenery.Plane()
floor.normal = (0, 0, 1)
floor.d = 0
floor.shader = shaders.Simple()
floor.shader.diffuse = textures.GridBoard(black, white)
floor.shader.specular = textures.GridBoard(white, black)
floor.shader.specularPower = textures.Constant(20)

r = 10
hi = 500
lo = 125
lights = [scenery.LightSpot() for i in range(2)]
lookAts = [(-r, r, r), (+r, r, r)]
intensities = [rgb(hi, lo, lo), rgb(lo, lo, hi)]
for light, lookAt, intensity in zip(lights, lookAts, intensities):
    light.position = lookAt
    light.lookAt(sphere.center)
    light.intensity = intensity
    light.outerAngle = radians(7)
    light.innerAngle = radians(6.5)

camera = cameras.PerspectiveCamera()
camera.position = (0, 4, 2)
camera.lookAt((0, 0, 1))

image = output.Image("light_spot.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, sphere] + lights)
engine.camera = camera
engine.target = image
engine.render()
