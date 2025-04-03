#! /usr/bin/env python

# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
    raytrace_direct = True
    scatter_direct = True
    gather_rays = 16

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()

fog = mediums.Fog()
fog = mediums.ExponentialFog()
fog.extinction = 4.5  # .45
fog.assymetry = 0
fog.color = rgb(0.5, 0.5, 0.5)
fog.numScatterSamples = 100
try:
    fog.decay = 2
    fog.up = camera.sky
except AttributeError:
    pass

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.globalMapSize = global_size
photonMapper.causticsQuality = caustics_quality
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.isScatteringDirect = scatter_direct
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25
# photonMapper.setEstimationSize("volume", 100)
photonMapper.setEstimationSize("global", 50)

image = output.Image("photon_mapper_fog.hdr", (width, height))
display = output.Display("Exponential Fog", (width, height))

engine = RenderEngine()
engine.tracer = photonMapper
# engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks + lights)
engine.scene.interior = mediums.Bounded(fog, engine.scene.boundingBox())
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()
