import math

from liar import RenderEngine, RgbSpace, Transformation2D, Transformation3D, rgb, sRGB
from liar.cameras import PerspectiveCamera
from liar.mediums import Fog
from liar.output import Display
from liar.output import Image as OutputImage
from liar.output import Splitter
from liar.samplers import Halton, LatinHypercube
from liar.scenery import (
    AabbTree,
    Box,
    LightArea,
    List,
    MotionTranslation,
    Parallelogram,
    Sphere,
    Transformation,
)
from liar.shaders import (
    AshikhminShirley,
    BumpMapping,
    Dielectric,
    Lambert,
    LinearInterpolator,
    Mirror,
    Unshaded,
)
from liar.spectra import Cauchy
from liar.textures import (
    Abs,
    Constant,
    FBm,
    Global,
    Image,
    Product,
    Sin,
    Sum,
    TransformationLocal,
    TransformationUv,
    Xyz,
)
from liar.tools.download import download
from liar.tools.geometry import negate
from liar.tracers import AdjointPhotonTracer

from _drand48 import DRand48

nx = 800
ny = 800
ns = 100


RgbSpace.setDefaultSpace(sRGB.linearSpace())


def final():
    drand48 = DRand48(0x1234ABCD)

    white = Lambert(Constant(rgb(0.73, 0.73, 0.73)))
    ground = Lambert(Constant(rgb(0.48, 0.83, 0.53)))

    lst = []

    nb = 20
    boxlist = []
    drand48.seed48 = (0x3C2D, 0x9C1E, 0x5BEF)
    for i in range(nb):
        for j in range(nb):
            w = 100
            x0 = -1000 + i * w
            z0 = -1000 + j * w
            y0 = 0
            x1 = x0 + w
            y1 = 100 * (drand48() + 0.01)
            z1 = z0 + w
            box = Box((x0, y0, z0), (x1, y1, z1))
            box.shader = ground
            boxlist.append(box)
    lst.append(AabbTree(boxlist))

    x0, x1, z0, z1, k = 123, 423, 147, 412, 554
    xz_rect = Parallelogram((x0, k, z0), (0, 0, z1 - z0), (x1 - x0, 0, 0))
    light = LightArea(xz_rect)
    light.radiance = rgb(7, 7, 7)
    light.shader = Unshaded(Constant(light.radiance))
    light.isDoubleSided = True
    lst.append(light)

    center = (400, 400, 200)
    speed = (30, 0, 0)
    radius = 50
    sphere = Sphere((0, 0, 0), radius)
    sphere.shader = Lambert(Constant(rgb(0.7, 0.3, 0.1)))
    lst.append(MotionTranslation(sphere, center, speed))

    center = (260, 150, 45)
    radius = 50
    sphere = Sphere(center, radius)
    sphere.shader = Dielectric(Constant(Cauchy(1.5, 0.1)))
    lst.append(sphere)

    center = (0, 150, 145)
    radius = 50
    sphere = Sphere(center, radius)
    sphere.shader = Mirror(Constant(rgb(0.8, 0.8, 0.9)))
    # sphere.shader.fuzz = Constant(1)
    sphere.shader.fuzz = Constant(0.8)
    lst.append(sphere)

    center = (400, 200, 400)
    radius = 100
    sphere = Sphere(center, radius)

    url = "http://planetpixelemporium.com/download/download.php?earthmap1k.jpg"
    diffuse = Image(download(url, "earthmap.jpg"))
    diffuse = TransformationUv(diffuse, Transformation2D.scaler((-1, 1)))
    url = "http://planetpixelemporium.com/download/download.php?earthspec1k.jpg"
    specular = Image(download(url, "earthspec.jpg"))
    specular = TransformationUv(specular, Transformation2D.scaler((-1, 1)))
    specular = Product(specular, Constant(0.2))
    sphere.shader = AshikhminShirley(diffuse, specular)
    sphere.shader.specularPowerU = sphere.shader.specularPowerV = Constant(50)

    url = "http://planetpixelemporium.com/download/download.php?earthbump1k.jpg"
    earthbump = Image(download(url, "earthbump.jpg"))
    earthbump = TransformationUv(earthbump, Transformation2D.scaler((-1, 1)))
    sphere.shader = BumpMapping(sphere.shader, earthbump)
    sphere.shader.scale = -1  # because we flipped the uv coordinates?

    url = "http://planetpixelemporium.com/images/mappreviews/earthcloudmapthumb.jpg"
    cloudmap = Image(download(url, "cloudmap.jpg"))
    cloudmap = TransformationUv(cloudmap, Transformation2D.scaler((-1, 1)))
    url = "http://planetpixelemporium.com/download/download.php?earthcloudmaptrans.jpg"
    cloudtrans = Image(download(url, "cloudtrans.jpg"))
    cloudtrans = TransformationUv(cloudtrans, Transformation2D.scaler((-1, 1)))
    sphere.shader = LinearInterpolator(
        [(0, Lambert(cloudmap)), (1, sphere.shader)], cloudtrans
    )

    transf = Transformation3D.translation(negate(center))
    transf = transf.concatenate(Transformation3D.rotation((0, 0, 1), math.radians(120)))
    transf = transf.concatenate(Transformation3D.rotation((1, 0, 0), math.radians(90)))
    transf = transf.concatenate(Transformation3D.translation(center))
    sphere = Transformation(sphere, transf)
    lst.append(sphere)

    center = (220, 280, 300)
    radius = 80
    pertext = Product(
        [
            Constant(0.5),
            Sum(
                [
                    Constant(1),
                    Sin(
                        Global(
                            TransformationLocal(
                                Sum(
                                    [
                                        TransformationLocal(
                                            Xyz(
                                                Constant(1000),
                                                Constant(0),
                                                Constant(0),
                                            ),
                                            Transformation3D.scaler(0.001),
                                        ),
                                        Product(
                                            [
                                                Constant(5),
                                                Abs(FBm(7)),
                                            ]
                                        ),
                                    ]
                                ),
                                Transformation3D.scaler(0.1),
                            )
                        )
                    ),
                ]
            ),
        ]
    )
    sphere = Sphere(center, radius)
    sphere.shader = Lambert(pertext)
    lst.append(sphere)

    center = (360, 150, 145)
    radius = 70
    sphere = Sphere(center, radius)
    sphere.shader = Dielectric(Constant(1.5))
    sphere.interior = Fog(0.2, 0)
    sphere.interior.color = rgb(0.2, 0.4, 0.9)
    lst.append(sphere)

    center = (0, 0, 0)
    radius = 5000
    sphere = Sphere(center, radius)
    sphere.interior = Fog(0.0001, 0)
    lst.append(sphere)

    n = 1000
    spherelist = []
    drand48.seed48 = (52986, 43265, 25132)
    for j in range(n):
        x = 165 * drand48()
        y = 165 * drand48()
        z = 165 * drand48()
        sphere = Sphere((x, y, z), 10)
        sphere.shader = white
        spherelist.append(sphere)
    transf = Transformation3D.translation((-0.5 * 165, -0.5 * 165, -0.5 * 165))
    transf = transf.concatenate(Transformation3D.rotation((0, 1, 0), math.radians(15)))
    transf = transf.concatenate(
        Transformation3D.translation((+0.5 * 165, +0.5 * 165, +0.5 * 165))
    )
    transf = transf.concatenate(Transformation3D.translation((-100, 270, 395)))
    lst.append(Transformation(AabbTree(spherelist), transf))

    scene = List(lst)
    scene.interior = Fog(0.0001, 0)
    return scene


camera = PerspectiveCamera()
camera.position = (478, 278, -600)
camera.sky = (0, 1, 0)
camera.lookAt((278, 278, 0))
camera.focusDistance = 10
camera.aspectRatio = float(nx) / ny
vfov = 40.0
camera.focalLength = (camera.height / 2) / math.tan(math.radians(vfov) / 2)
aperture = 0.0
camera.lensRadius = aperture / 2
camera.shutterTime = 1

image = OutputImage("raytracingthenextweek.pfm", (nx, ny))
display = Display("Raytracing the next week", (nx, ny))
image.rgbSpace = display.rgbSpace = RgbSpace.defaultSpace().withGamma(2)

engine = RenderEngine()
engine.tracer = AdjointPhotonTracer()
engine.sampler = LatinHypercube((nx, ny), ns)
engine.sampler = Halton()
engine.scene = final()
engine.camera = camera
engine.target = engine.target = Splitter([image, display])
engine.render()
