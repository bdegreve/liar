#! /usr/bin/env python

# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
from liar.tools import cornell_box, scripting

#if False:
#	width = 800
#	height = 800
#	super_sampling = 4
#	global_size = 200000
#	caustics_quality = 1
#	gather_rays = 36
#else:
#	width = 400
#	height = 400
#	super_sampling = 1
#	global_size = 10000
#	caustics_quality = 1
#	raytrace_direct = True
#	gather_rays = 9

options = scripting.renderOptions(
    size=800,
    super_sampling=4,
    global_size = 200000,
    caustics_quality=1.,
    gather_rays = 36
    )

raytrace_direct = True

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.globalMapSize = options.global_size
photonMapper.causticsQuality = options.caustics_quality
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.numFinalGatherRays = options.gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((options.size, options.size), options.super_sampling)
engine.sampler.jittered = False
engine.scene = cornell_box.scene()
engine.camera = cornell_box.camera()
engine.target = scripting.makeRenderTarget(options.size, options.size, "photon_mapper_classic.hdr", "Classic Cornell box with PhotonMapper")
engine.render()

