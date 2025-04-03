#! /usr/bin/env python

# geometry of cornell box.
# used by cornell box examples
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/
#
# Homepage of cornell box:
# http://www.graphics.cornell.edu/online/box/

from liar import *
from liar.tools import spd
import math


def scene():
    return scenery.List(lights() + walls() + blocks())


def camera():
    camera = cameras.PerspectiveCamera()
    camera.aspectRatio = 1
    camera.fovAngle = 2 * math.atan((0.025 / 0.035) / 2)
    camera.position = _scale(278, 273, -800)
    camera.sky = (0, 1, 0)
    camera.direction = (0, 0, 1)
    return camera


def lights():
    support = _scale(213, 548.79, 227)
    dx = _scale(343 - 213, 0, 0)
    dy = _scale(0, 0, 332 - 227)
    surface = scenery.Parallelogram(support, dx, dy)
    light = scenery.LightArea(surface)
    light.radiance = _LIGHT
    light.numberOfEmissionSamples = 16
    light.shader = shaders.Sum(
        [
            shaders.Unshaded(textures.Constant(light.radiance)),
            shaders.Lambert(textures.Constant(0.78)),
        ]
    )
    return [light]


def walls():
    white = shaders.Lambert(textures.Constant(_WHITE))
    red = shaders.Lambert(textures.Constant(_RED))
    green = shaders.Lambert(textures.Constant(_GREEN))

    floor = scenery.Parallelogram(
        _scale(0, 0, 559.2), _scale(552.8, 0, 0), _scale(0, 0, -559.2)
    )
    floor.shader = white

    ceiling = scenery.Parallelogram(
        _scale(0.0, 548.8, 0), _scale(556.0, 0, 0), _scale(0, 0, 559.2)
    )
    ceiling.shader = white

    backwall = scenery.Parallelogram(
        _scale(556, 0, 559.2), _scale(-556.0, 0, 0), _scale(0, 548.8, 0)
    )
    backwall.shader = white

    rightwall = scenery.Parallelogram(
        _scale(0, 0, 559.2), _scale(0, 0, -559.2), _scale(0, 548.8, 0)
    )
    rightwall.shader = green

    leftwall = scenery.Parallelogram(
        _scale(552.8, 0, 0), _scale(0, 0, 559.2), _scale(0, 548.8, 0)
    )
    leftwall.shader = red

    return [floor, ceiling, backwall, rightwall, leftwall]


def blocks():
    white = shaders.Lambert(textures.Constant(_WHITE))
    short = _block(
        [(130.0, 65.0), (82.0, 225.0), (240.0, 272.0), (290.0, 114.0)], [0.05, 165.0]
    )
    short.shader = white
    tall = _block(
        [(423.0, 247.0), (265.0, 296.0), (314.0, 456.0), (472.0, 406.0)], [0.05, 330.0]
    )
    tall.shader = white
    return [short, tall]


def _block(ps, hs):
    def normal(a, b, c):
        u = [bx - ax for ax, bx in zip(a, b)]
        v = [cx - ax for ax, cx in zip(a, c)]
        n = (
            u[1] * v[2] - u[2] * v[1],
            u[2] * v[0] - u[0] * v[2],
            u[0] * v[1] - u[1] * v[0],
        )
        length = math.sqrt(sum([x * x for x in n]))
        return tuple([x / length for x in n])

    verts = [_scale(p[0], h, p[1]) for h in hs for p in ps]

    normal_groups = [(0, 3, 1), (4, 5, 7), (0, 1, 4), (1, 2, 5), (2, 3, 6), (3, 0, 7)]
    normals = [normal(verts[a], verts[b], verts[c]) for a, b, c in normal_groups]

    triangle_groups = [
        ((0, 2, 1), 0),
        ((0, 3, 2), 0),
        ((4, 6, 7), 1),
        ((4, 5, 6), 1),
        ((0, 5, 4), 2),
        ((0, 1, 5), 2),
        ((1, 6, 5), 3),
        ((1, 2, 6), 3),
        ((2, 7, 6), 4),
        ((2, 3, 7), 4),
        ((3, 4, 7), 5),
        ((3, 0, 4), 5),
    ]
    triangles = [((t[0], n), (t[1], n), (t[2], n)) for t, n in triangle_groups]

    return scenery.TriangleMesh(verts, normals, [], triangles)


def _scale(*i):
    return tuple(0.01 * x for x in i)


_WHITE, _RED, _GREEN, _LIGHT = spd.loads("""
400.0 0.343 0.040 0.092 3.591
404.0 0.445 0.046 0.096 3.614
408.0 0.551 0.048 0.098 3.806
412.0 0.624 0.053 0.097 3.906
416.0 0.665 0.049 0.098 4.344
420.0 0.687 0.050 0.095 4.693
424.0 0.708 0.053 0.095 4.951
428.0 0.723 0.055 0.097 5.425
432.0 0.715 0.057 0.095 5.967
436.0 0.710 0.056 0.094 6.192
440.0 0.745 0.059 0.097 6.936
444.0 0.758 0.057 0.098 7.713
448.0 0.739 0.061 0.096 8.315
452.0 0.767 0.061 0.101 8.955
456.0 0.777 0.060 0.103 9.552
460.0 0.765 0.062 0.104 10.218
464.0 0.751 0.062 0.107 10.736
468.0 0.745 0.062 0.109 11.627
472.0 0.748 0.061 0.112 12.513
476.0 0.729 0.062 0.115 13.204
480.0 0.745 0.060 0.125 13.893
484.0 0.757 0.059 0.140 14.402
488.0 0.753 0.057 0.160 15.210
492.0 0.750 0.058 0.187 16.369
496.0 0.746 0.058 0.229 17.251
500.0 0.747 0.058 0.285 18.063
504.0 0.735 0.056 0.343 18.843
508.0 0.732 0.055 0.390 19.409
512.0 0.739 0.056 0.435 20.213
516.0 0.734 0.059 0.464 21.241
520.0 0.725 0.057 0.472 21.801
524.0 0.721 0.055 0.476 21.817
528.0 0.733 0.059 0.481 22.199
532.0 0.725 0.059 0.462 23.176
536.0 0.732 0.058 0.447 24.245
540.0 0.743 0.059 0.441 24.690
544.0 0.744 0.061 0.426 24.852
548.0 0.748 0.061 0.406 25.695
552.0 0.728 0.063 0.373 27.480
556.0 0.716 0.063 0.347 29.534
560.0 0.733 0.067 0.337 31.057
564.0 0.726 0.068 0.314 32.077
568.0 0.713 0.072 0.285 33.152
572.0 0.740 0.080 0.277 34.085
576.0 0.754 0.090 0.266 34.783
580.0 0.764 0.099 0.250 35.241
584.0 0.752 0.124 0.230 35.964
588.0 0.736 0.154 0.207 37.293
592.0 0.734 0.192 0.186 38.289
596.0 0.741 0.255 0.171 39.317
600.0 0.740 0.287 0.160 40.105
604.0 0.732 0.349 0.148 40.907
608.0 0.745 0.402 0.141 41.706
612.0 0.755 0.443 0.136 42.386
616.0 0.751 0.487 0.130 42.980
620.0 0.744 0.513 0.126 43.455
624.0 0.731 0.558 0.123 43.920
628.0 0.733 0.584 0.121 44.466
632.0 0.744 0.620 0.122 45.114
636.0 0.731 0.606 0.119 45.715
640.0 0.712 0.609 0.114 46.473
644.0 0.708 0.651 0.115 46.828
648.0 0.729 0.612 0.117 47.186
652.0 0.730 0.610 0.117 47.469
656.0 0.727 0.650 0.118 47.961
660.0 0.707 0.638 0.120 48.414
664.0 0.703 0.627 0.122 48.702
668.0 0.729 0.620 0.128 48.903
672.0 0.750 0.630 0.132 49.180
676.0 0.760 0.628 0.139 49.254
680.0 0.751 0.642 0.144 49.172
684.0 0.739 0.639 0.146 48.831
688.0 0.724 0.657 0.150 48.218
692.0 0.730 0.639 0.152 47.056
696.0 0.740 0.635 0.157 45.183
700.0 0.737 0.642 0.159 42.236
""")
