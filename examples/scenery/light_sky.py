#! /usr/bin/python

# demonstrates the use of LightSky and AngularMaping to do images based lighting using a light probe
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

# specify URL of light probe:
probe_url = "http://www.debevec.org/Probes/grace_probe.hdr"

width = 400
height = 300
super_sampling = 1


from liar import *


def auto_downloader(url):
    import os.path

    filename = os.path.basename(url)
    if not os.path.isfile(filename):
        print("Downloading '%s' ..." % url)
        import urllib.error
        import urllib.parse
        import urllib.request

        file(filename, "wb").write(urllib.request.urlopen(url).read())
    return filename


light_probe = textures.AngularMapping(textures.Image(auto_downloader(probe_url)))
light_probe.texture.mipMapping = textures.Image.MM_NONE
light = scenery.LightSky(light_probe)
light.numberOfEmissionSamples = 64
light.shader = shaders.Unshaded(light_probe)


floor = scenery.Parallelogram((5, 5, -1), (-10, 0, 0), (0, -10, 0))
floor.shader = shaders.Lambert(textures.Constant(0.5))

diffuse_sphere = scenery.Sphere((0, 0, 0), 1)
diffuse_sphere.shader = shaders.Lambert(textures.Constant(0.8))

shiny_sphere = scenery.Sphere((2.5, 0, 0), 1)
shiny_sphere.shader = shaders.Mirror(textures.Constant(0.9))

glass_sphere = scenery.Sphere((-2.5, 0, 0), 1)
glass_sphere.shader = shaders.Dielectric()
glass_sphere.shader.innerRefractionIndex = textures.Constant(1.3)
glass_sphere.interior = shaders.Beer(rgb(0.9, 0.9, 0.5))


camera = cameras.PerspectiveCamera()
camera.position = (0, +4, 1.5)
camera.lookAt((0, 0, 0))

photonMapper = tracers.PhotonMapper()
photonMapper.maxNumberOfPhotons = 1000000000
photonMapper.globalMapSize = 1000
photonMapper.causticsQuality = 1000
photonMapper.isVisualizingPhotonMap = False
photonMapper.isRayTracingDirect = True
photonMapper.numFinalGatherRays = 16

image = output.Image("light_sky.hdr", (width, height))
display = output.Display("Imaged base lighting with LightSky", (width, height))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.sampler.jittered = False
engine.scene = scenery.List([floor, diffuse_sphere, shiny_sphere, glass_sphere, light])
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()
