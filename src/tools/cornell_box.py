#! /usr/bin/env python

# geometry of cornell box.
# used by cornell box examples
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/
#
# Homepage of cornell box:
# http://www.graphics.cornell.edu/online/box/

from liar import *
import math

_s = 0.01

def _scale(*i):
	return tuple(_s * x for x in i)

def lights():
	I = 20
	support = _scale(213, 548.79, 227)
	dx = _scale(343 - 213, 0, 0)
	dy = _scale(0, 0, 332 - 227)
	surface = scenery.Parallelogram(support, dx, dy)
	light = scenery.LightArea(surface)
	light.radiance = rgb(I, I, I)
	light.numberOfEmissionSamples = 16
	light.shader = shaders.Unshaded(textures.Constant(light.radiance))
	return [light]

def walls():
	hi = .7
	lo = .1
	white = shaders.Lambert(textures.Constant(rgb(hi, hi, hi)))
	red = shaders.Lambert(textures.Constant(rgb(hi, lo, lo)))
	green = shaders.Lambert(textures.Constant(rgb(lo, hi, lo)))
	
	floor = scenery.Parallelogram(_scale(0, 0, 559.2), _scale(552.8, 0, 0), _scale(0, 0, -559.2))
	floor.shader = white

	ceiling = scenery.Parallelogram(_scale(0.0, 548.8, 0), _scale(556.0, 0, 0), _scale(0, 0, 559.2))
	ceiling.shader = white
	
	backwall = scenery.Parallelogram(_scale(556, 0, 559.2), _scale(-556.0, 0, 0), _scale(0, 548.8, 0))
	backwall.shader = white
	
	rightwall = scenery.Parallelogram(_scale(0, 0, 559.2), _scale(0, 0, -559.2), _scale(0, 548.8, 0))
	rightwall.shader = green
	
	leftwall = scenery.Parallelogram(_scale(552.8, 0, 0), _scale(0, 0, 559.2), _scale(0, 548.8, 0))
	leftwall.shader = red

	return [floor, ceiling, backwall, rightwall, leftwall]

def _block(ps, hs):
	def normal(a, b, c):
		u = [bx - ax for ax, bx in zip(a, b)]
		v = [cx - ax for ax, cx in zip(a, c)]
		n = (u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0])
		length = math.sqrt(sum([x * x for x in n]))
		return tuple([x / length for x in n])
	
	verts = [_scale(p[0], h, p[1]) for h in hs for p in ps]
	
	normal_groups = [
		(0, 3, 1),
		(4, 5, 7),
		(0, 1, 4),
		(1, 2, 5),
		(2, 3, 6),
		(3, 0, 7)
	]
	normals = [normal(verts[a], verts[b], verts[c]) for a, b, c in normal_groups]

	triangle_groups = [
		((0, 2, 1), 0), ((0, 3, 2), 0),
		((4, 6, 7), 1), ((4, 5, 6), 1),
		((0, 5, 4), 2), ((0, 1, 5), 2),
		((1, 6, 5), 3), ((1, 2, 6), 3),
		((2, 7, 6), 4), ((2, 3, 7), 4),
		((3, 4, 7), 5), ((3, 0, 4), 5)
	]	
	triangles = [((t[0], n), (t[1], n), (t[2], n))for t, n in triangle_groups]
	
	return scenery.TriangleMesh(verts, normals, [], triangles)	
	
def blocks():
	white = shaders.Lambert(textures.Constant(rgb(.7, .7, .7)))
	short = _block([(130.0, 65.0), (82.0, 225.0), (240.0, 272.0), (290.0, 114.0)], [0.05, 165.0])
	short.shader = white
	tall = _block([(423.0, 247.0), (265.0, 296.0), (314.0, 456.0), (472.0, 406.0)], [0.05, 330.0])
	tall.shader = white
	return [short, tall]

def camera():
	camera = cameras.PerspectiveCamera()
	camera.aspectRatio = 1
	camera.fovAngle = 2 * math.atan((0.025 / 0.035) / 2)
	camera.position = _scale(278, 273, -800)
	camera.sky = (0, 1, 0)
	camera.direction = (0, 0, 1)
	return camera

	
