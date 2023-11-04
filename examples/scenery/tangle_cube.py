#! /usr/bin/env python

# Demonstration of the classic cornell box using the PhotonMapper
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
from liar.tools import cornell_box, scripting, rgb_spaces
import math
import sys
no_display =  "--no-display" in sys.argv

fps = 25.0
num_frames = 25
t_max = num_frames / fps

num_rotations_x = 2
num_rotations_z = 3
num_rotations_y = 5

speed_x = (2 * math.pi * num_rotations_x) / t_max
speed_y = (2 * math.pi * num_rotations_y) / t_max
speed_z = (2 * math.pi * num_rotations_z) / t_max


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

if False:
    options = scripting.renderOptions(
        width=400,
        height=400,
        super_sampling=16,
        global_size = 200000,
        caustics_quality=1.,
        gather_rays = 81
        )
else:
    options = scripting.renderOptions(
        frame=12,
        width=400,
        height=400,
        super_sampling=4,
        global_size = 5000,
        caustics_quality=1.,
        gather_rays = 16,
        display=1
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

a, b, c = 0, -5, 11.8
#a, b, c = 1, 0, -1
#a, b, c = 0, 0, -1
#a, b, c = 0, -2, -1
#a, b, c = 0, -2, 1
#a, b, c = 0, -1, .5
#a, b, c = -1, 1, -1
goursat = scenery.Goursat(a, -10, c)
goursat.parameterSpeed = (0, 10, 0)
goursat.useSphereTracing = False
goursat.shader = shaders.AshikhminShirley()
goursat.shader.diffuse = textures.Constant(rgb(0.1, 0.1, 0.9))
goursat.shader.specular = textures.Constant(0.05)
goursat.shader.specularPowerU = goursat.shader.specularPowerV = textures.Constant(1000)


# scale down
goursat = scenery.Transformation(goursat, liar.Transformation3D.scaler(.75))

if False:
    goursat = scenery.MotionRotation(goursat, (1, 0, 0), 0, speed_x)
    goursat = scenery.MotionRotation(goursat, (0, 0, 1), 0, speed_z)
    goursat = scenery.MotionRotation(goursat, (0, 1, 0), 0, speed_y)




#goursat = scenery.MotionTranslation(goursat, (0, 0, 0), (0, 1, 0))

y = 2.26 # ground floor
y = 5.4879 / 2 # center of room
objects.append(scenery.Translation(goursat, (2.78, y, 2.795)))


engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((options.width, options.height), options.super_sampling)
engine.sampler.jittered = True
engine.scene = scenery.List(objects)
engine.camera = cornell_box.camera()
engine.camera.shutterTime = .8 * (1 / fps)
engine.target = scripting.makeRenderTarget(options.width, options.height, "tangle_cube_b_%d.jpg" % options.frame, display=options.display, fStops=0)
engine.render((options.frame - .4) / fps)
