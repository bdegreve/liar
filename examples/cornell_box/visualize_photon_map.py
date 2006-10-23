# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2006  Bram de Greve

from liar import *
import geometry

setTolerance(tolerance() * 1000)

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
	global_size = 80000
	raytrace_direct = True
	gather_rays = 9

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()

r = 1
sphere1 = scenery.Sphere((4.5, r, 4.5), r)
sphere1.shader = shaders.AshikhminShirley()
sphere1.shader.diffuse = textures.Constant(rgb(0, 0, 0.0))
sphere1.shader.specular = textures.Constant(0.5)
sphere1.shader.specularPowerU = textures.Constant(100)
sphere1.shader.specularPowerV = textures.Constant(100)

sphere2 = scenery.Sphere((1, r, 4.5), r)
sphere2.shader = shaders.AshikhminShirley()
sphere2.shader.diffuse = textures.Constant(rgb(0, 0, 0.9))
sphere2.shader.specular = textures.Constant(0.05)
sphere2.shader.specularPowerU = textures.Constant(1000)
sphere2.shader.specularPowerV = textures.Constant(1000)

spheres = [sphere1, sphere2]

#I = 20
#sphereLight = scenery.LightArea(sphere)
#sphereLight.radiance = rgb(I, I, I)
#sphereLight.numberOfEmissionSamples = 16
#sphereLight.shader = shaders.Unshaded(textures.Constant(sphereLight.radiance))

image = output.Image("visualize_photon_map.hdr", (width, height))

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 10000000
photonMapper.setRequestedMapSize("global", global_size)
photonMapper.setEstimationRadius("global", 0.2)
photonMapper.setEstimationSize("global", 100)
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = raytrace_direct
photonMapper.numFinalGatherRays = gather_rays
photonMapper.ratioPrecomputedIrradiance = 0.25

camera.focalDistance = 13
camera

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + spheres +  lights)
engine.camera = camera
engine.target = image
engine.numberOfThreads = 2
engine.render()

