import math

from _drand48 import DRand48 as DRand48new

from liar import RenderEngine, RgbSpace, rgb, sRGB
from liar.cameras import PerspectiveCamera
from liar.output import Display, Image, Splitter
from liar.samplers import LatinHypercube
from liar.scenery import LightSky, List, Sphere
from liar.shaders import Dielectric, Lambert, Mirror, Unshaded
from liar.textures import Constant, SkyBlend
from liar.tools.geometry import norm, subtract
from liar.tracers import AdjointPhotonTracer

nx = 1200
ny = 800
ns = 10

RgbSpace.setDefaultSpace(sRGB.linearSpace())


def random_scene():
    _i = 0
    drand48 = DRand48new(0x1234ABCD)
    n = 500
    list_ = [sphere((0, -1000, 0), 1000, Lambert(Constant(rgb(0.5, 0.5, 0.5))))]
    for a in range(-11, 11):
        for b in range(-11, 11):
            choose_mat = drand48()
            x = a + 0.9 * drand48()
            z = b + 0.9 * drand48()
            center = (x, 0.2, z)
            if distance(center, (4, 0.2, 0)) > 0.9:
                if choose_mat < 0.8:  # diffuse
                    r = drand48() * drand48()
                    g = drand48() * drand48()
                    b = drand48() * drand48()
                    mat = Lambert(Constant(rgb(r, g, b)))
                elif choose_mat < 0.95:  # metal
                    r = 0.5 * (1 + drand48())
                    g = 0.5 * (1 + drand48())
                    b = 0.5 * (1 + drand48())
                    f = 0.5 * drand48()
                    mat = Mirror(Constant(rgb(r, g, b)))
                    mat.fuzz = Constant(f)
                else:  # glass
                    mat = Dielectric(Constant(1.5))
                list_.append(sphere(center, 0.2, mat))
    list_.append(sphere((0, 1, 0), 1, Dielectric(Constant(1.5))))
    list_.append(sphere((-4, 1, 0), 1, Lambert(Constant(rgb(0.4, 0.2, 0.1)))))
    list_.append(sphere((+4, 1, 0), 1, Mirror(Constant(rgb(0.7, 0.6, 0.5)))))

    sky_tex = SkyBlend(Constant(rgb(0.5, 0.7, 1)), Constant(rgb(1, 1, 1)))
    sky_tex.zenith = (0, 1, 0)
    sky = LightSky(sky_tex)
    sky.shader = Unshaded(sky_tex)
    sky.numberOfEmissionSamples = 64
    list_.append(sky)

    return List(list_)


def distance(a, b):
    return norm(subtract(a, b))


def sphere(center, radius, material):
    obj = Sphere(center, radius)
    obj.shader = material
    return obj


camera = PerspectiveCamera()
camera.position = (13, 2, 3)
camera.sky = (0, 1, 0)
camera.lookAt((0, 0, 0))
camera.focusDistance = 10
camera.aspectRatio = float(nx) / ny
camera.focalLength = (camera.height / 2) / math.tan(math.radians(20) / 2)
aperture = 0.1
camera.lensRadius = aperture / 2

image = Image("raytracinginoneweekend.pfm", (nx, ny))
display = Display("Raytracing in one weekend", (nx, ny))
display.rgbSpace = image.rgbSpace = RgbSpace.defaultSpace().withGamma(2)

engine = RenderEngine()
engine.tracer = AdjointPhotonTracer()
engine.sampler = LatinHypercube((nx, ny), ns)
engine.scene = random_scene()
engine.camera = camera
engine.target = engine.target = Splitter([image, display])
engine.render()
