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

precision = 7

lamps = {}
meshes = {}
materials = {}
textures = {}
images = {}



def escape(x):
	return x.replace('\\', '\\\\')



def matrix_to_list(matrix):
	return [[matrix[i][j] for i in range(4)] for j in range(4)]



def str_float(f):
	format_string = '%%.%ig' % precision
	return format_string % f



def str_no_none(i):
	if i == None:
		return ''
	return str(i)



def compress_vectors_to_string(vectors):
	return '\n'.join([' '.join([str_float(x) for x in v]) for v in vectors])



def compress_faces_to_string(faces):
	return '\n'.join([' '.join([','.join([str_no_none(i) for i in v]) for v in f]) for f in faces])



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



def write_image(out, file_path, texture):
	assert(texture.getType() == 'Image')
	image_name = texture.image.getName()
	image_filename = texture.image.getFilename()
	if not image_name in images:
		images[image_name] = True
		if texture.mipmap:
			anti_aliasing = 'AA_TRILINEAR'
			mip_mapping = 'MM_ANISOTROPIC'
		else:
			anti_aliasing = 'AA_BILINEAR'
			mip_mapping = 'MM_NONE'
		if not texture.interpol:
			anti_aliasing = 'AA_NONE'
		
		out.write("""
# --- image IM:%s ---

image = liar.textures.Image(fix_path(r"%s"))
image.antiAliasing = liar.textures.Image.%s
image.mipMapping = liar.textures.Image.%s
images['IM:%s'] = image
""" % (image_name, image_filename, anti_aliasing, mip_mapping, image_name))

def write_texture(out, file_path, texture):
	if not texture.name in textures:
		tex_type = texture.getType()
		if tex_type == 'Image':
			write_image(out, file_path, texture)		
			out.write("""
# --- texture TE:%s ---

textures['TE:%s'] = images['IM:%s']
""" % (texture.name, texture.name, texture.image.getName()))
		
		else:
			if tex_type != 'None':
				print "WARNING: Cannot handle %s textures!" % tex_type
			out.write("""
# --- texture TE:%s ---

textures['TE:%s'] = liar.Texture.black()
""" % (texture.name, texture.name))



def write_material(out, file_path, material):
	if not material.name in materials:
		materials[material.name] = True
		
		diffuse = "liar.textures.Constant(liar.rgb%s)" % (tuple([x * material.ref for x in material.rgbCol]),)
		specular = "liar.textures.Constant(liar.rgb%s)" % (tuple([x * material.spec for x in material.specCol]),)
		specular_power = "liar.textures.Constant(%s)" % material.hard
		reflectance = "liar.textures.Constant(liar.rgb%s)" % (tuple([x * material.spec for x in material.mirCol]),)

		for texture in material.getTextures():
			if texture:
				write_texture(out, file_path, texture.tex)
				s = "textures['TE:%s']" % texture.tex.name
				if texture.mapto & Blender.Texture.MapTo['COL']:
					diffuse = s
				if texture.mapto & Blender.Texture.MapTo['CSP']:
					specular = s
				if texture.mapto & Blender.Texture.MapTo['HARD']:
					specular_power = s
				if texture.mapto & Blender.Texture.MapTo['CMIR']:
					reflectance = s

		out.write(r"""
# --- mesh MA:%s ---

material = liar.shaders.Simple()
material.diffuse = %s
material.specular = %s
material.specularPower = %s
material.reflectance = %s
materials['MA:%s'] = material
""" % (material.name, diffuse, specular, specular_power, reflectance, material.name))



def write_face_group(out, file_path, vertices, normals, uvs, faces, material):
	vertices_part, normals_part, uvs_part, faces_part = compress_mesh(vertices, normals, uvs, faces)		
	vertices_str = compress_vectors_to_string(vertices_part)
	normals_str = compress_vectors_to_string(normals_part)
	uvs_str = compress_vectors_to_string(uvs_part)
	faces_str = compress_faces_to_string(faces_part)
	out.write(r"""
vertices = '''\
%s'''
normals = '''\
%s'''
uvs = '''\
%s'''
faces = '''\
%s'''
face_group = build_triangle_mesh(vertices, normals, uvs, faces)
face_group.shader = materials['MA:%s']
""" % (vertices_str, normals_str, uvs_str, faces_str, material.name))
	



def write_mesh(out, file_path, obj):
	mesh = obj.getData(False, True)
	
	if not mesh.name in meshes:
		meshes[mesh.name] = True

		num_materials = len(mesh.materials)
		if num_materials == 0:
			print 'WARNING: no material assigned to mesh, skipping mesh'
			return
		
		# extract vertices, normals and uvs
		vertices = [tuple([mvert.co[k] for k in range(3)]) for mvert in mesh.verts]
		normals = [tuple([mvert.no[k] for k in range(3)]) for mvert in mesh.verts]
		if mesh.vertexUV:
			uvs = [tuple([mvert.uvco[k] for k in range(3)]) for mvert in mesh.verts]
		else:
			uvs = []
		
		# extract faces per material
		faces = [[] for i in xrange(num_materials)]
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
					uvs.append(tuple([muv[k] for k in range(2)]))
					faceUvs.append(len(uvs) - 1)
			else:
				faceUvs = size * [None]
			
			indices = zip(faceVerts, faceNormals, faceUvs)
			
			assert(face.mat >= 0 and face.mat < num_materials)
			for k in range(size - 2):
				faces[face.mat].append((indices[0], indices[k + 1], indices[k + 2]))
				
		# determine used face groups
		used_groups = [i for i in xrange(num_materials) if len(faces[i]) > 0]
		if not used_groups:
			print 'WARNING: mesh has no faces, skipping mesh'
			return
				
		# write used materials
		for i in used_groups:
			write_material(out, file_path, mesh.materials[i])
			
		out.write("""
# --- mesh ME:%s ---
""" % mesh.name)

		if len(used_groups) == 1:
			i = used_groups[0]
			write_face_group(out, file_path, vertices, normals, uvs, faces[i], mesh.materials[i])
			out.write("""
meshes['ME:%s'] = face_group
""" % mesh.name)
			
		else:
			assert(len(used_groups) > 1)
			out.write("""
face_groups = []
""")
			for i in used_groups:
				write_face_group(out, file_path, vertices, normals, uvs, faces[i], mesh.materials[i])
				out.write("""
face_groups.append(face_group)
""")

			out.write("""
meshes['ME:%s'] = liar.scenery.AabbTree(face_groups)
""")
				
	obj_matrix = matrix_to_list(obj.matrix)
	out.write(r"""
# --- object OB:%s ---

matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(meshes['ME:%s'], matrix)
""" % (obj.name, obj_matrix, obj.name, mesh.name))



def write_lamp(out, file_path, obj):
	lamp = obj.getData()
	if not lamp.name in lamps:
		lamps[lamp.name] = True
		intensity = (lamp.energy * lamp.R, lamp.energy * lamp.G, lamp.energy * lamp.B)
		out.write("""
# --- lamp LA:%s ---
""" % lamp.name)

		if lamp.type == 0: # point light
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			out.write(r"""
light = liar.scenery.LightPoint()
light.position = (0, 0, 0)
light.intensity = liar.rgb%s
light.attenuation = liar.scenery.Attenuation(0, %s, %s)
lamps['LA:%s'] = light
""" % (intensity, attLin, attSqr, lamp.name))

		elif lamp.type == 1: # sun
			out.write(r"""
light = liar.scenery.LightDirectional()
light.direction = (0, 0, -1)
light.radiance = liar.rgb%s
lamps['LA:%s'] = light
""" % (intensity, lamp.name))

		elif lamp.type == 2: # spot
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			outer_angle = lamp.spotSize / 2.0
			inner_angle = outer_angle * (1 - lamp.spotBlend)
			out.write(r"""
light = liar.scenery.LightSpot()
light.position = (0, 0, 0)
light.direction = (0, 0, -1)
light.intensity = liar.rgb%s
light.attenuation = liar.scenery.Attenuation(0, %s, %s)
light.outerAngle = liar.radians(%s)
light.innerAngle = liar.radians(%s)
lamps['LA:%s'] = light
""" % (intensity, attLin, attSqr, outer_angle, inner_angle, lamp.name))

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
			out.write(r"""
sizeX = %s
sizeY = %s
surface = liar.scenery.Parallelogram((-sizeX / 2., sizeY / 2., 0), (sizeX, 0, 0), (0, -sizeY, 0))
light = liar.scenery.LightArea(surface)
light.radiance = liar.rgb%s
light.numberOfEmissionSamples = %s
lamps['LA:%s'] = light
""" % (size_x, size_y, intensity, number_of_samples, lamp.name))

		else:
			print "WARNING: Did not recognize lamp type '%s'" % lamp.type
			return
		
	obj_matrix = matrix_to_list(obj.matrix)
	out.write(r"""
# --- object OB:%s ---

matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(lamps['LA:%s'], matrix)
""" % (obj.name, obj_matrix, obj.name, lamp.name))



def write_camera(out, file_path, obj):
	camera = obj.getData()
	type = ('PerspectiveCamera', 'OrthographicCamera')[camera.getType()]
	position = obj.getLocation()
	matrix = obj.matrix
	direction = [-matrix[2][k] for k in range(3)]
	right = [matrix[0][k] for k in range(3)]
	down = [-matrix[1][k] for k in range(3)]
	fov = 2. * math.atan(16. / camera.lens)
	out.write(r"""
# --- camera ---

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
	out.write(r"""
width = %s
height = %s
samples_per_pixel = %s
aspect_ratio = %s
out_file = fix_path(r"%s")

camera.aspectRatio = aspect_ratio
sampler = liar.samplers.Stratifier((width, height), samples_per_pixel)
target = liar.output.Image(out_file, (width, height))
""" % (width, height, samples_per_pixel, aspect_ratio, out_file))



def write_header(out, file_path):
	out.write(r"""
#! /usr/bin/python

# Exported from Blender by liar_export
import liar
import os.path

objects = {}
cameras = {}
lamps = {}
meshes = {}
materials = {}
textures = {}
images = {}

def int_or_none(s):
	try:
		return int(s)
	except:
		return None

def fix_path(path):
	if path.startswith('//'):
		path = os.path.join(os.path.curdir, path[2:])
	if os.path.altsep:
		path = path.replace(os.path.altsep, os.path.sep)
	return path

def expand_vector_string(s):
	if not s:
		return []
	return [tuple([float(x) for x in line.split(' ')]) for line in s.split('\n')]

def expand_face_string(s):
	return [tuple([tuple([int_or_none(i) for i in v.split(',')]) for v in line.split(' ')]) for line in s.split('\n')]
	
def build_triangle_mesh(vertices_string, normals_string, uvs_string, faces_string):
	vertices = expand_vector_string(vertices_string)
	normals = expand_vector_string(normals_string)
	uvs = expand_vector_string(uvs_string)
	faces = expand_face_string(faces_string)
	return liar.scenery.TriangleMesh(vertices, normals, uvs, faces)
""")



def write_body(out, file_path):
	object_writers = {
		'Mesh': write_mesh,
		'Lamp': write_lamp,
		'Camera': write_camera
		}
		
	scene = Blender.Scene.GetCurrent()
	children = scene.getChildren()
	num_children = len(children)
	
	for obj, i in zip(children, range(num_children)):
		obj_type = obj.getType()
		obj_name = obj.getName()
		print "- object '%s' of type '%s'" % (obj_name, obj_type)
		obj_materials = obj.getMaterials()
		if obj_type in object_writers:              
			object_writers[obj_type](out, file_path, obj)
		else:
			print "WARNING: Did not recognize object type '%s'" % obj_type
			
		ipo = obj.getIpo()
		if ipo:
			print ipo
			print ipo.getName()
			for curve in ipo.getCurves():
				print curve
				print curve.getPoints()
				
		draw_progress(float(i+1) / num_children)

			
	context = scene.getRenderingContext()
	write_context(out, file_path, context)
		

		
def write_footer(out, file_path):
	out.write('''
engine = liar.RenderEngine()
engine.tracer = liar.tracers.DirectLighting()
engine.sampler = sampler
engine.scene = liar.scenery.AabbTree(objects.values())
engine.camera = camera
engine.target = target

if __name__ == "__main__":
	engine.render()
''')



def draw_progress(progress):
	Blender.Window.DrawProgressBar(float(progress), "Exporting scene to LiAR [%3d%%]" % int(100 * progress))


def export(file_path):
	print "Exporting to LiAR '%s' ..." % file_path
	out = open(file_path, 'w')
	
	draw_progress(0)

	write_header(out, file_path)
	write_body(out, file_path)
	write_footer(out, file_path)

	draw_progress(1)

	out.close()
	print "Done ..."



def export_callback(file_path):
	if os.path.exists(file_path) :
		if not Blender.Draw.PupMenu("Overwrite File?%t|Yes%x1|No%x0"):
			return
	export(file_path)



suggested_filename = os.path.splitext(Blender.Get('filename'))[0] + ".liar.py"
Blender.Window.FileSelector(export_callback, "Export to LiAR", suggested_filename)
