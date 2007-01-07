# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.sourceforge.net

from liar import *
import geometry

if True:
	width = 800
	height = 800
	super_sampling = 4
	global_size = 200000
	caustics_quality = 1
	raytrace_direct = True
	gather_rays = 36
else:
	width = 400
	height = 400
	super_sampling = 1
	global_size = 10000
	caustics_quality = 1
	raytrace_direct = True
	gather_rays = 9

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.globalMapSize = global_size
photonMapper.causticsQuality = caustics_quality
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25

image = output.Image("photon_mapper_classic.hdr", (width, height))
display = output.Display("Classic Cornell box with PhotonMapper", (width, height))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.sampler.jittered = False
engine.scene = scenery.List(walls + blocks +  lights)
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()

