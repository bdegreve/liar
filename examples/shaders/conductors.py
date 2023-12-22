"""
Demonstration of different ways to model metallic surfaces.

Using tabulated data for the refractive index of copper, we model a copper
sphere using three different shaders:

- Conductor: takes the refractive index and absorption coefficient as input,
    and uses the Fresnel equations to compute the reflectance. Does not model
    surface roughness.
- CookTorrance: Also takes the refractive index and absorption coefficient as
    input, but uses the Cook-Torrance model to model surface roughness. However,
    we set the roughness to zero, so this shader is equivalent to the Conductor
    shader.
- AshikhminShirley: a more general shader that takes two reflectance values as
    input, one for diffuse reflection and one for specular reflection. To model
    a metallic surface, we set the diffuse reflectance to zero and the specular
    reflectance to the Fresnel reflectance at normal incidence, which we get
    from the R0Conductor spectrum.

The first two shaders are physically based using the Fresnel equations and can
vary the reflectance color with angle of incidence. The AshikhminShirley shader
has a fixed reflectance color, and is less accurate for grazing angles, but in
practice, this is hardly noticeable.

The uffizi_latlong.exr image used for the light source can be downloaded from
https://vgl.ict.usc.edu/Data/HighResProbes/
"""

import os
import sys

import liar
from liar.tools.spd import load_builtin_refractive_index

nx = 1200
ny = 1200

copperN, copperK = load_builtin_refractive_index("Cu")

texN, texK = liar.textures.Constant(copperN), liar.textures.Constant(copperK)
conductor = liar.shaders.Conductor(texN, texK)

cook_torrance = liar.shaders.CookTorrance(texN, texK)
cook_torrance.roughnessU = cook_torrance.roughnessV = liar.textures.Constant(0)
cook_torrance.numberOfSamples = 1

Rd = liar.textures.Constant(0)
Rs = liar.textures.Constant(liar.spectra.R0Conductor(copperN, copperK))
ashikhmin_shirley = liar.shaders.AshikhminShirley(Rd, Rs)
ashikhmin_shirley.roughnessU = ashikhmin_shirley.roughnessV = liar.textures.Constant(0)
ashikhmin_shirley.numberOfSamples = 1

lambert = liar.shaders.Lambert(Rs)

SHADERS = {
    "Conductor": conductor,
    "CookTorrance": cook_torrance,
    "AshikhminShirley": ashikhmin_shirley,
}

light = liar.scenery.LightSky(liar.textures.Image("uffizi_latlong.exr"))
light.shader = liar.shaders.Unshaded(light.radiance)
light.samplingResolution = light.radiance.resolution

sphere = liar.scenery.Sphere((0, 0, 0), 1)

objects = [
    sphere,
    light,
]

camera = liar.cameras.PerspectiveCamera()
camera.position = (0, 2, 0)
camera.lookAt((0, 1, 0))
camera.focalLength = 0.03
camera.aspectRatio = nx / ny

for name, shader in SHADERS.items():
    fname = f"{os.path.splitext(__file__)[0]}_{name}.exr"
    print(f"--- {fname}")

    sphere.shader = shader
    image = liar.output.Image(fname, (nx, ny))

    engine = liar.RenderEngine()
    engine.tracer = liar.tracers.DirectLighting()
    engine.sampler = liar.samplers.Stratifier((nx, ny), 4)
    engine.camera = camera
    engine.scene = liar.scenery.List(objects)
    engine.target = image
    # engine.target = liar.output.Splitter([image, liar.output.Display(name, (nx, ny))])
    if hasattr(sys, "gettotalrefcount"):
        engine.numberOfThreads = 1
    engine.render()
