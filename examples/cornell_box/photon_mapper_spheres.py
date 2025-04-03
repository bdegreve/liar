#! /usr/bin/env python

# Demonstration of a photon mapped Cornell box with some spheres ...
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
from liar.tools import cornell_box, scripting

setTolerance(tolerance() * 1000)

options = scripting.renderOptions(
    size=800,
    super_sampling=4,
    photon_mapping=True,
    global_size=200000,
    caustics_quality=20,
    raytrace_direct=True,
    gather_rays=36,
)

camera = cornell_box.camera()
walls = cornell_box.walls()
lights = cornell_box.lights()

glass_sphere = scenery.Sphere((1.86, 1, 1.20), 1)
glass_sphere.shader = shaders.Dielectric()
glass_sphere.shader.innerRefractionIndex = textures.Constant(2)
glass_sphere.interior = mediums.Beer(rgb(0.5, 0.5, 0))
# glass_sphere.interior = mediums.Beer(rgb(1, 1, 0))

blue_sphere = scenery.Sphere((3.69, 1, 3.51), 1)
blue_sphere.shader = shaders.AshikhminShirley()
blue_sphere.shader.diffuse = textures.Constant(rgb(0, 0, 0.9))
blue_sphere.shader.specular = textures.Constant(0.05)
blue_sphere.shader.specularPowerU = blue_sphere.shader.specularPowerV = (
    textures.Constant(1000)
)

spheres = [glass_sphere, blue_sphere]

if options.photon_mapping:
    tracer = tracers.PhotonMapper()
    tracer.maxNumberOfPhotons = 10000000
    tracer.globalMapSize = options.global_size
    tracer.causticsQuality = options.caustics_quality
    tracer.isVisualizingPhotonMap = False
    tracer.isRayTracingDirect = options.raytrace_direct
    tracer.numFinalGatherRays = options.gather_rays
    tracer.ratioPrecomputedIrradiance = 0.25
else:
    tracer = tracers.DirectLighting()

image = output.Image("photon_mapper_spheres.hdr", (options.size, options.size))
display = output.Display(
    "photon mapped Cornell box with sppheres", (options.size, options.size)
)

engine = RenderEngine()
engine.tracer = tracer
engine.sampler = samplers.Stratifier(
    (options.size, options.size), options.super_sampling
)
engine.sampler.jittered = False
engine.scene = scenery.List(walls + spheres + lights)
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()
