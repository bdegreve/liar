# LiAR isn't a raytracer
# Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# http://liar.bramz.net/

import os
import math

from liar import *

# Compare this with Fig. 2 of Ashikhmin and Shirley's technical report:
#   Michael Ashikmin and Peter Shirley. An anisotropic phong light reﬂection model.
#   Technical Report UUCS-00-014, Computer Science Department, University of Utah,
#   June 2000.

Rd = 0
Rs = 1
nu = 10000
nv = 10000

# Set Rs = 0.05 and Rd = 1 to compare with Fig. 4 of the same report.
#
# Rs = 0.05
# Rd = 1

material = liar.shaders.AshikhminShirley(liar.textures.Constant(Rd), liar.textures.Constant(Rs))
material.specularPowerU = liar.textures.Constant(nu)
material.specularPowerV = liar.textures.Constant(nv)

tracer = tracers.AdjointPhotonTracer()
#tracer = tracers.DirectLighting()

light = scenery.LightSky(textures.Constant(1))
light.shader = shaders.Unshaded(light.radiance)

camera = cameras.PerspectiveCamera()
camera.position = (0, -10, 5)
camera.aspectRatio = 1
camera.lookAt((0, 0, 0))
camera.fovAngle = math.radians(11)

sphere = scenery.Sphere((0, 0, 0), 1)
sphere.shader = material

resolution = 800

fname = os.path.splitext(__file__)[0] + ".exr"
outputs = [
    output.Image(fname, (resolution, resolution)),
    output.Display(fname, (resolution, resolution))
]
for out in outputs:
    out.exposureStops = -1
    out.autoExposure = False

engine = RenderEngine()
engine.tracer = tracer
engine.sampler = samplers.Halton()
engine.scene = scenery.List([sphere, light])
engine.camera = camera
engine.target = output.Splitter(outputs)
#engine.numberOfThreads = 1
engine.render()
