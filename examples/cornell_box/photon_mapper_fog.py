#! /usr/bin/env python

# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
import geometry

if False:
	width = 1024
	height = 1024
	super_sampling = 9
	global_size = 200000
	caustics_quality = 1
	raytrace_direct = True
	scatter_direct = True
	gather_rays = 64
else:
	width = 400
	height = 400
	super_sampling = 1
	global_size = 10000
	caustics_quality = 1
	raytrace_direct = False#True
	scatter_direct = True
	gather_rays = 0

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()


fog = shaders.Fog()
fog.extinction = .45
fog.assymetry = 0
fog.color = rgb(.5, .5, .5)
fog.numScatterSamples = 100

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.globalMapSize = global_size
photonMapper.causticsQuality = caustics_quality
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.isScatteringDirect = scatter_direct
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25
#photonMapper.setEstimationSize("volume", 100)
photonMapper.setEstimationSize("global", 50)

image = output.Image("photon_mapper_fog.hdr", (width, height))
display = output.Display("Participating Median in Cornell box with PhotonMapper", (width, height))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks +  lights)
engine.scene.interior = shaders.BoundedMedium(fog, engine.scene.boundingBox())
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()
