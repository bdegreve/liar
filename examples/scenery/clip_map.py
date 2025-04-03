#! /usr/bin/env python

# Demonstrates the use of ClipMap
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
import math

if True:
    width = 800
    height = 600
    super_sampling = 9
    global_size = 100000
    gather_rays = 16
else:
    width = 400
    height = 300
    super_sampling = 1
    global_size = 10000
    gather_rays = 9

clip_map = textures.CheckerVolume(textures.Constant(0), textures.Constant(1))
sphere = scenery.Sphere((0, 0, 0), 1)
sphere.shader = shaders.Lambert(textures.Constant(rgb(0.8, 0.8, 0.7)))
clipped_sphere = scenery.ClipMap(sphere, clip_map)

floor = scenery.Plane((0, 0, 1), 1)
floor.shader = shaders.Lambert(textures.Constant(0.5))

wall1 = scenery.Plane((-1, 0, 0), 4)
wall1.shader = shaders.Lambert(textures.Constant(rgb(0.8, 0.2, 0.2)))

wall2 = scenery.Plane((0, -1, 0), 4)
wall2.shader = shaders.Lambert(textures.Constant(rgb(0.2, 0.8, 0.2)))

center = (-4, -4, 4)
size = 0.25
right = [size * x for x in (-math.sqrt(2) / 2, math.sqrt(2) / 2, 0)]
up = [size * x for x in (0.5, 0.5, math.sqrt(2) / 2)]
origin = [c - x / 2 - y / 2 for c, x, y in zip(center, right, up)]
L = 1000
light = scenery.LightArea(scenery.Parallelogram(origin, right, up))
light.radiance = rgb(L, L, L)
light.numberOfEmissionSamples = 16
light.shader = shaders.Unshaded(textures.Constant(light.radiance))

camera = cameras.PerspectiveCamera()
camera.position = (0, -2, 1.5)
camera.lookAt((0, 0, 0))
camera.lensRadius = 0.02

photonMapper = tracers.PhotonMapper()
photonMapper.globalMapSize = global_size
photonMapper.isRayTracingDirect = True
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25

image = output.Image("clip_map.hdr", (width, height))
display = output.Display("Fun with Clip Mapping", (400, 300))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([clipped_sphere, floor, wall1, wall2, light])
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()
