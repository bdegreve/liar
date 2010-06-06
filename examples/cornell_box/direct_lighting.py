#! /usr/bin/env python

# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
import geometry

if True:
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

image = output.Image("direct_lighting.hdr", (width, height))

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List(walls + blocks + lights)
engine.camera = camera
engine.target = image
engine.render()

