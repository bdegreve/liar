import math


def triangulate_3d(*verts):
	normal = polygon_normal_3d(*verts)
	try:
		return triangulate_3d_with_normal(normal, *verts)
	except AssertionError as error:
		raise AssertionError("Failed to triangulate %(verts)r, normal=%(normal)r: %(error)s" % vars())


def polygon_normal_3d(*verts):
	try:
		from functools import reduce
	except ImportError:
		pass
	assert len(verts) >= 3
	a = verts[0]
	pairs = zip(verts[1:-1], verts[2:])
	return reduce(add, [cross_product(subtract(b, a), subtract(c, a)) for b, c in pairs])


def triangulate_3d_with_normal(normal, *verts):
	verts_2d = project_polygon_with_normal(normal, *verts)
	k = major_axis(normal)
	if normal[k] < 0:
		return reversed(triangulate_2d(*reversed(verts_2d)))
	else:
		return triangulate_2d(*verts_2d)


def project_polygon_with_normal(normal, *verts):
	assert len(normal) == 3
	k = major_axis(normal)
	i, j = (k + 1) % 3, (k + 2) % 3
	verts = [(v[i], v[j]) for v in verts]
	return verts


def major_axis(normal):
	return max(zip(map(abs, normal), range(len(normal))))[1]


def triangulate_2d(*verts):
	'''
	triangulate((x1, y1), (x2, y2), ...) -> (a1, b1, c1), (a2, b2, c2), ...
	
	naive O(n**2) ear clipper.
	'''
	vert_indices = list(range(len(verts)))
	area = signed_area_2d(*verts)
	assert area >= 0, "polygon %r has negative area %r" % (verts, area)
	if area == 0:
		return list(zip(vert_indices, vert_indices[1:] + vert_indices[:1], vert_indices[2:] + vert_indices[:2]))
	triangles = []
	inf_loop_guard = vert_indices[0]
	while len(vert_indices) > 3:
		candidate = tuple(vert_indices[:3])
		others = vert_indices[3:]
		a, b, c = [verts[k] for k in candidate]
		is_ear = signed_area_2d(a, b, c) > 0 and not any(in_triangle_2d(a, b, c, verts[k]) for k in others)
		if is_ear:
			triangles.append(candidate)
			del vert_indices[1] # the 2nd vert is the actual ear
			inf_loop_guard = vert_indices[0]
		else:
			vert_indices = vert_indices[1:] + vert_indices[:1]
			assert vert_indices[0] != inf_loop_guard, "oops, infinity loop!"
	assert len(vert_indices) == 3
	triangles.append(tuple(vert_indices))
	return triangles


def in_triangle_2d(a, b, c, d):
	'''
	triangulate((x1, y1), (x2, y2), (x3, y3), (x4, y4)) -> True/False
	
	Returns true if (x4, y4) is a point within triangle (x1, y1), (x2, y2), (x3, y3)
	'''
	n = signed_area_2d(a, b, c)
	assert n != 0, "a, b and c are colinear"
	if n < 0:
		b, c = c, b # force CCW
	for p, q in ((a, b), (b, c), (c, a)):
		if signed_area_2d(p, q, d) < 0:
			return False
	return True


def signed_area_2d(*verts):
	if len(verts) < 3:
		return 0
	a = verts[0]
	pairs = zip(verts[1:-1], verts[2:])
	return .5 * sum(perp_dot(subtract(b, a), subtract(c, a)) for b, c in pairs)


def perp_dot(a, b):
	ax, ay = a
	bx, by = b
	return ax * by - bx * ay


def cross_product(a, b):
	ax, ay, az = a
	bx, by, bz = b
	return (ay * bz - by * az, az * bx - bz * ax, ax * by - bx * ay)


def add(a, b):
	return tuple(x1 + x2 for x1, x2 in zip(a, b))


def subtract(a, b):
	return tuple(x1 - x2 for x1, x2 in zip(a, b))


def normalize(v):
	scale = 1. / norm(v)
	return tuple(scale * x for x in v)


def norm(v):
	return math.sqrt(sum(x * x for x in v))


if __name__ == "__main__":
	print(signed_area_2d((0, 0), (1, 0), (1, 1)))
	print(triangulate_2d((0, 0), (1, 0), (1, 1), (0, 1)))
	print(triangulate_2d((0, 0), (1, 0), (0.1, 0.1), (0, 1)))
	print(triangulate_2d((1, 0), (0.1, 0.1), (0, 1), (0, 0)))