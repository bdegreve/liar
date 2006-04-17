# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2006  Bram de Greve

from liar import *
import geometry

if 1:
	width = 800
	height = 800
	super_sampling = 9
else:
	width = 320
	height = 320
	super_sampling = 1

camera = geometry.getCamera()
walls = geometry.getWalls()
blocks = geometry.getBlocks()
lights = geometry.getLights()


I = 0.1
sky = scenery.LightArea(scenery.Sky())
sky.radiance = rgb(0.5 * I, 0.5 * I, I)
sky.numberOfEmissionSamples = 36
sky.shader = shaders.Unshaded(textures.Constant(sky.radiance))


image = output.Image("direct_lighting.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks + lights)
engine.camera = camera
engine.target = image
engine.render()

