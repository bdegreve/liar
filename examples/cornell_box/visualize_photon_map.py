# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2006  Bram de Greve

from liar import *
import geometry

if False:
	width = 800
	height = 800
	super_sampling = 4
	global_size = 200000
	raytrace_direct = True
	gather_rays = 36
else:
	width = 320
	height = 320
	super_sampling = 1
	global_size = 200000
	raytrace_direct = True
	gather_rays = 4

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()

image = output.Image("visualize_photon_map_noindirect.hdr", (width, height))

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.setRequestedMapSize("global", global_size)
photonMapper.setEstimationRadius("global", 0.2)
photonMapper.setEstimationSize("global", 100)
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.numFinalGatherRays = gather_rays

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks + lights)
engine.camera = camera
engine.target = image
engine.numberOfThreads = 2
engine.render()

