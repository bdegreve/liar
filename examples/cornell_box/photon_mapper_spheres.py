#! /usr/bin/env python

# Demonstration of a photon mapped Cornell box with some spheres ...
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.sourceforge.net

from liar import *
import geometry

setTolerance(tolerance() * 1000)

if True:
	width = 800
	height = 800
	super_sampling = 4
	global_size = 200000
	caustics_quality = 20
	raytrace_direct = True
	gather_rays = 36
else:
	width = 400
	height = 400
	super_sampling = 1
	global_size = 10000
	caustics_quality = 5
	raytrace_direct = True
	gather_rays = 9

camera = geometry.getCamera()
walls = geometry.getWalls()
lights = geometry.getLights()

glass_sphere = scenery.Sphere((1.86, 1, 1.20), 1)
glass_sphere.shader = shaders.Dielectric()
glass_sphere.shader.innerRefractionIndex = textures.Constant(2)
glass_sphere.interior = shaders.Beer(rgb(0.5, 0.5, 0))

blue_sphere = scenery.Sphere((3.69, 1, 3.51), 1)
blue_sphere.shader = shaders.AshikhminShirley()
blue_sphere.shader.diffuse = textures.Constant(rgb(0, 0, 0.9))
blue_sphere.shader.specular = textures.Constant(0.05)
blue_sphere.shader.specularPowerU = blue_sphere.shader.specularPowerV = textures.Constant(1000)

spheres = [glass_sphere, blue_sphere]

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.globalMapSize = global_size
photonMapper.causticsQuality = caustics_quality
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25

image = output.Image("photon_mapper_spheres.hdr", (width, height))
display = output.Display("photon mapped Cornell box with sppheres", (width, height))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.sampler.jittered = False
engine.scene = scenery.List(walls + spheres +  lights)
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()

