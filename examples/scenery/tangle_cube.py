#! /usr/bin/env python

# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
from liar.tools import cornell_box, scripting
import math

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

if True:
    options = scripting.renderOptions(
        width=800,
        height=800,
        super_sampling=16,
        global_size = 200000,
        caustics_quality=1.,
        gather_rays = 81
        )
else:
    options = scripting.renderOptions(
        width=400,
        height=400,
        super_sampling=1,
        global_size = 20000,
        caustics_quality=1.,
        gather_rays = 25
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

objects = cornell_box.walls() + cornell_box.lights()

goursat = scenery.Goursat(0, -5, +11.8)
goursat.useSphereTracing = False
goursat.shader = shaders.AshikhminShirley()
goursat.shader.diffuse = textures.Constant(rgb(0.1, 0.1, 0.9))
goursat.shader.specular = textures.Constant(0.05)
goursat.shader.specularPowerU = goursat.shader.specularPowerV = textures.Constant(1000)

goursat = scenery.MotionRotation(goursat, (0, 0, 1), math.radians(15), math.radians(5))
#goursat = scenery.MotionTranslation(goursat, (0, 0, 0), (0, 1, 0))

y = 2.26
objects.append(scenery.Translation(goursat, (2.78, 2.26, 2.795)))


engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((options.width, options.height), options.super_sampling)
engine.sampler.jittered = True
engine.scene = scenery.List(objects)
engine.camera = cornell_box.camera()
engine.camera.shutterTime = 1
engine.target = scripting.makeRenderTarget(options.width, options.height, "photon_mapper_classic.hdr", "Classic Cornell box with PhotonMapper")
engine.render()

