#! /usr/bin/env python

# cornell box with direct lighting only
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
from liar.tools import scripting, cornell_box

options = scripting.renderOptions(width=800, height=800, super_sampling=9)

engine = RenderEngine()
engine.tracer = tracers.DirectLighting()
engine.sampler = samplers.Stratifier((options.width, options.height), options.super_sampling)
engine.scene = cornell_box.scene()
engine.camera = cornell_box.camera()
engine.target = scripting.makeRenderTarget(options.width, options.height, "direct_lighting.hdr", "Cornell Box with Direct Lighting only")
engine.render()

