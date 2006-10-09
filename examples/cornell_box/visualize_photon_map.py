# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2006  Bram de Greve

from liar import *
import geometry

if True:
	width = 800
	height = 800
	super_sampling = 4
else:
	width = 320
	height = 320
	super_sampling = 1

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()

image = output.Image("visualize_photon_map.hdr", (width, height))

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 1000000
photonMapper.setRequestedMapSize("global", 2000)
photonMapper.setEstimationRadius("global", 0.05)
photonMapper.setEstimationSize("global", 100)
photonMapper.isVisualizingPhotonMap = True
photonMapper.isRayTracingDirect = True
photonMapper.numFinalGatherRays = 0

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks + lights)
engine.camera = camera
engine.target = image
engine.numberOfThreads = 1
engine.render()

