#!BPY

#
#	LiAR isn't a raytracer
#	Copyright (C) 2004-2005  Bram de Greve (bramz@sourceforge.net)
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	http://liar.sourceforge.net
#

"""
Name: 'LiAR isn`t a raytracer (.py)...'
Blender: 232
Group: 'Export'
Tooltip: 'Exports Blender scene to LiAR script (.py)'
"""

__author__ = "Bram de Greve (bramz@users.sourceforge.net)"
__url__ = ("Project homepage, http://liar.sourceforge.net")
__version__ = "0.0"
__bpydoc__ = """\
Exports Blender scene to LiAR script.

Usage: run the script from the menu or inside Blender.  
"""

import Blender
import os.path
import math

lamps = {}
meshes = {}
materials = {}

def escape(x):
	return x.replace('\\', '\\\\')

def matrix_to_list(matrix):
	return [[matrix[i][j] for i in range(4)] for j in range(4)]

def compress_mesh(vertices, normals, uvs, faces):
	components = [vertices, normals, uvs]
	# determine used components
	used_components = [{}, {}, {}]
	for face in faces:
		for vertex in face:
			for k in range(3):
				if vertex[k] != None and not vertex[k] in used_components[k]:
					used_components[k][vertex[k]] = len(used_components[k])
	# compress components
	sorted_indices = [[(used_components[k][index], index) for index in used_components[k]] for k in range(3)]
	for k in range(3):
		sorted_indices[k].sort()
	compressed_components = [[components[k][index[1]] for index in sorted_indices[k]] for k in range(3)]
	# rebuild faces
	for k in range(3):
		used_components[k][None] = None
	rebuilt_faces = [tuple([tuple([used_components[k][vertex[k]] for k in range(3)]) for vertex in face]) for face in faces]
	# done!
	return compressed_components + [rebuilt_faces]
			

def write_none(out, file_path, obj):
	pass

def write_mesh(out, file_path, obj):
	mesh = obj.getData(False, True)
	
	material = mesh.materials[0]
	if not material.name in materials:
		materials[material.name] = material
		diffuse = tuple([x * material.ref for x in material.rgbCol])
		specular = tuple([x * material.spec for x in material.specCol])
		specular_power = material.hard
		out.write('''
material = liar.shaders.Simple()
material.diffuse = liar.textures.Constant(liar.rgb%s)
material.specular = liar.textures.Constant(liar.rgb%s)
material.specularPower = liar.textures.Constant(%s)
materials['MA:%s'] = material
''' % (diffuse, specular, specular_power, material.name))

	if not mesh.name in meshes:
		meshes[mesh.name] = mesh
		vertices = [tuple([mvert.co[k] for k in range(3)]) for mvert in mesh.verts]
		normals = [tuple([mvert.no[k] for k in range(3)]) for mvert in mesh.verts]
		if mesh.vertexUV:
			uvs = [tuple([mvert.uvco[k] for k in range(3)]) for mvert in mesh.verts]
		else:
			uvs = []
		faces = []
		for face in mesh.faces:
			size = len(face.verts)
			assert(size == 3 or size == 4)
			
			faceVerts = [mvert.index for mvert in face.verts]
			
			if face.smooth:
				faceNormals = faceVerts
			else:
				normals.append(tuple([face.no[k] for k in range(3)]))
				faceNormals = size * [len(normals) - 1]
			
			if mesh.vertexUV:
				faceUvs = faceVerts
			elif mesh.faceUV:
				assert(len(face.uv) == size)
				faceUvs = []
				for muv in face.uv:
					uvs.append(tuple([muv[k] for k in range(3)]))
					faceUvs.append(len(uvs) - 1)
			else:
				faceUvs = size * [None]
			
			indices = zip(faceVerts, faceNormals, faceUvs)
			for k in range(size - 2):
				faces.append((indices[0], indices[k + 1], indices[k + 2]))
				
		vertices, normals, uvs, faces = compress_mesh(vertices, normals, uvs, faces)
		
		vertices_str = ',\n\t'.join([str(vert) for vert in vertices])
		normals_str = ',\n\t'.join([str(normal) for normal in normals])
		uvs_str = ',\n\t'.join([str(uv) for uv in uvs])
		faces_str = ',\n\t'.join([str(face) for face in faces])
		out.write('''
vertices = [%s]
normals = [%s]
uvs = [%s]
faces = [%s]
mesh = liar.scenery.TriangleMesh(vertices, normals, uvs, faces)
mesh.shader = materials['MA:%s']
meshes['ME:%s'] = mesh
''' % (vertices_str, normals_str, uvs_str, faces_str, material.name, mesh.name))

	obj_matrix = matrix_to_list(obj.matrix)
	out.write('''
matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(meshes['ME:%s'], matrix)
''' % (obj_matrix, obj.name, mesh.name))


def write_lamp(out, file_path, obj):
	lamp = obj.getData()
	if not lamp.name in lamps:
		lamps[lamp.name] = lamp
		intensity = (lamp.energy * lamp.R, lamp.energy * lamp.G, lamp.energy * lamp.B)
		if lamp.type == 0: # point light
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			out.write('''
light = liar.scenery.LightPoint()
light.position = (0, 0, 0)
light.power = liar.rgb%s
light.attenuation = liar.scenery.LightPoint.Attenuation(0, %s, %s)
lamps['LA:%s'] = light
''' % (intensity, attLin, attSqr, lamp.name))

		elif lamp.type == 1: # sun
			out.write('''
light = liar.scenery.LightDirectional()
light.direction = (0, 0, -1)
light.intensity = liar.rgb%s
lamps['LA:%s'] = light
''' % (intensity, lamp.name))

		elif lamp.type == 2: # spot
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			outer_angle = lamp.spotSize / 2.0
			inner_angle = outer_angle * (1 - lamp.spotBlend)
			out.write('''
light = liar.scenery.LightSpot()
light.position = (0, 0, 0)
light.direction = (0, 0, -1)
light.intensity = liar.rgb%s
light.attenuation = liar.scenery.LightSpot.Attenuation(0, %s, %s)
light.outerAngle = liar.radians(%s)
light.innerAngle = liar.radians(%s)
lamps['LA:%s'] = light
''' % (intensity, attLin, attSqr, outer_angle, inner_angle, lamp.name))

		elif lamp.type == 4: # area
			print lamp.getMode()
			print Blender.Lamp.Modes['Square']
			is_square = lamp.mode & Blender.Lamp.Modes['Square']
			print 'is_square', is_square
			size_x = lamp.areaSizeX
			size_y = (lamp.areaSizeY, size_x)[is_square]
			samples_x = lamp.raySamplesX
			samples_y = (lamp.raySamplesY, 1)[is_square]
			number_of_samples = samples_x * samples_y
			out.write('''
sizeX = %s
sizeY = %s
surface = liar.scenery.Parallelogram((-sizeX / 2., sizeY / 2., 0), (sizeX, 0, 0), (0, -sizeY, 0))
light = liar.scenery.LightArea(surface)
light.intensity = liar.rgb%s
light.numberOfEmissionSamples = %s
lamps['LA:%s'] = light
''' % (size_x, size_y, intensity, number_of_samples, lamp.name))

		else:
			print "WARNING: Did not recognize lamp type '%s'" % lamp.type
			return
		
	obj_matrix = matrix_to_list(obj.matrix)
	out.write('''
matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(lamps['LA:%s'], matrix)
''' % (obj_matrix, obj.name, lamp.name))


def write_camera(out, file_path, obj):
	camera = obj.getData()
	type = ('PerspectiveCamera', 'OrthographicCamera')[camera.getType()]
	position = obj.getLocation()
	matrix = obj.matrix
	direction = [-matrix[2][k] for k in range(3)]
	right = [matrix[0][k] for k in range(3)]
	down = [-matrix[1][k] for k in range(3)]
	fov = 2. * math.atan(16. / camera.lens)
	out.write("""
camera = liar.cameras.%s()
camera.position = %s
camera.direction = %s
camera.right = %s
camera.down = %s
camera.fovAngle = %s
""" % (type, position, direction, right, down, fov))


def write_context(out, file_path, context):
	width = context.imageSizeX()
	height = context.imageSizeY()
	aspect_ratio = float(height * context.aspectRatioY()) / (width * context.aspectRatioX())
	samples_per_pixel = 1
	out_file = os.path.splitext(file_path)[0] + ".hdr"	
	out.write("""
width = %s
height = %s
samples_per_pixel = %s
aspect_ratio = %s
out_file = "%s"

camera.aspectRatio = aspect_ratio
sampler = liar.samplers.Stratifier((width, height), samples_per_pixel)
target = liar.output.Image(out_file, (width, height))
""" % (width, height, samples_per_pixel, aspect_ratio, escape(out_file)))



def write_header(out, file_path):
	out.write("""
# Exported from Blender by liar_export
import liar

objects = {}
cameras = {}
lamps = {}
meshes = {}
materials = {}
""")

def write_body(out, file_path):
	object_writers = {
		'Mesh': write_mesh,
		'Lamp': write_lamp,
		'Camera': write_camera
		}
		
	scene = Blender.Scene.GetCurrent()
	
	for obj in scene.getChildren():
		obj_type = obj.getType()
		obj_name = obj.getName()
		print "- object '%s' of type '%s'" % (obj_name, obj_type)
		obj_materials = obj.getMaterials()
		if obj_type in object_writers:              
			object_writers[obj_type](out, file_path, obj)
		else:
			print "WARNING: Did not recognize object type '%s'" % obj_type
			
	context = scene.getRenderingContext()
	write_context(out, file_path, context)
		
		
def write_footer(out, file_path):
	out.write('''
engine = liar.RenderEngine()
engine.tracer = liar.tracers.DirectLighting()
engine.sampler = sampler
engine.scene = liar.scenery.List(objects.values())
engine.camera = camera
engine.target = target

if __name__ == "__main__":
	engine.render()
''')

def export(file_path):
	print "Exporting to LiAR '%s' ..." % file_path
	out = open(file_path, 'w')

	write_header(out, file_path)
	write_body(out, file_path)
	write_footer(out, file_path)

	out.close()

def export_callback(file_path):
	if os.path.exists(file_path) :
		if not Blender.Draw.PupMenu("Overwrite File?%t|Yes%x1|No%x0"):
			return
	export(file_path)

suggested_filename = os.path.splitext(Blender.Get('filename'))[0] + ".liar.py"
Blender.Window.FileSelector(export_callback, "Export to LiAR", suggested_filename)




