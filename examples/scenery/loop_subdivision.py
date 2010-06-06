#! /usr/bin/python

# demonstrates the use of subdivision surfaces (and crease levels) on the TriangleMesh
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from liar import *
import math

if True:
	width = 1600
	height = 1200
	super_sampling = 9
else:
	width = 400
	height = 300
	super_sampling = 1

cube_material = shaders.Lambert(textures.Constant(rgb(0.8, 0.8, 0.7)))
floor_material = shaders.Lambert(textures.Constant(0.5))

# This function makes a cube and applies some subdivision to it
#
def make_cube(subdiv_level, crease_level):
	
	# First, a triangle mesh of a cube is made =)
	# The cube has an extra vertex in the middle of each face, to keep it nicely symmetric 
	# (instead of having to triangles per face, you now have four)
	#
	verts = [
		(-1, -1, -1), (-1, -1, +1),
		(-1, +1, -1), (-1, +1, +1),
		(+1, -1, -1), (+1, -1, +1),
		(+1, +1, -1), (+1, +1, +1),
		(-1, 0, 0), (+1, 0, 0),
		(0, -1, 0), (0, +1, 0),
		(0, 0, -1), (0, 0, +1)]
	faces = [
		(2, 0, 8), (0, 1, 8), (1, 3, 8), (3, 2, 8),
		(4, 6, 9), (6, 7, 9), (7, 5, 9), (5, 4, 9),
		(0, 4,10), (4, 5,10), (5, 1,10), (1, 0,10), 
		(6, 2,11), (2, 3,11), (3, 7,11), (7, 6,11),
		(0, 2,12), (2, 6,12), (6, 4,12), (4, 0,12),
		(3, 1,13), (1, 5,13), (5, 7,13), (7, 3,13)]
	full_faces = [[(i, None, None) for i in face]for face in faces]	
	cube = scenery.TriangleMesh(verts, [], [], full_faces)
	
	# Secondly, you must set the crease levels of the edges.
	# Currently you can only do that using the autoCrease function, setting the crease level of sharp edges
	#
	cube.autoCrease(crease_level)
	
	# At last, the subdivion surface algorithm is applied
	#
	cube.loopSubdivision(subdiv_level)
	
	# Now, the only thing left to do, is to give the cube some material properties, and to give it a position.
	cube.shader = cube_material
	theta = (subdiv_level + crease_level) * math.pi / 6
	r = 5
	return scenery.Translation(cube, (r * math.cos(theta), r * math.sin(theta), 0))



# here we go, make scene, set camera, set light, and render ...

cubes = [make_cube(subdiv_level, 0) for subdiv_level in range(6)]
creased_cubes = [make_cube(6, crease_level) for crease_level in range(6)]

floor = scenery.Plane((0, 0, 1), 1)
floor.shader = floor_material

light = scenery.LightSky(textures.Constant(1))
#light.radiance = textures.AngularMapping(textures.Image(r"probe_14-00_anglemap.hdr")) # Probe from http://gl.ict.usc.edu/skyprobes/
light.numberOfEmissionSamples = 64
light.shader = shaders.Unshaded(light.radiance)

camera = cameras.PerspectiveCamera()
camera.position = (0, -8, 4)
camera.lookAt((0, -3, 0))
camera.lensRadius = 0.01

# ok, we still need to set the DirectLighting back, but for now, use this =)
photonMapper = tracers.PhotonMapper()
photonMapper.globalMapSize = 0
photonMapper.isRayTracingDirect = True

image = output.Image("loop_subdivision.hdr", (width, height))
display = output.Display("Loop Subdivision Surfaces", (400, 300))

engine = RenderEngine()
engine.tracer = photonMapper
engine.sampler = samplers.Stratifier((width, height), super_sampling)
engine.scene = scenery.List([floor, light, scenery.AabbTree(cubes + creased_cubes)])
engine.camera = camera
engine.target = output.Splitter([image, display])
engine.render()

