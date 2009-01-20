#!BPY

# LiAR isn't a raytracer
# Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# http://liar.bramz.net/

"""
Name: 'LiAR isn`t a raytracer (.py)...'
Blender: 232
Group: 'Export'
Tooltip: 'Exports Blender scene to LiAR script (.py)'
"""

__author__ = "Bram de Greve (bramz@users.sourceforge.net)"
__url__ = ("Project homepage, http://liar.bramz.net")
__version__ = "0.0"
__bpydoc__ = """\
Exports Blender scene to LiAR script.

Usage: run the script from the menu or inside Blender.  
"""

precision = 7


import Blender
import sys
import math

try:
	import os
except ImportError:
	# we're running Blender without an corresponding Python installation
	# maybe we do have a Python installation, but not the same version
	# anyway, we shall have to rely on builtin modules only.
	# so here goes our poor man's os module
	for name in ('posix', 'nt', 'os2', 'mac', 'ce', 'riscos'):
		if name in sys.builtin_module_names:
			os = __import__(name, globals(), locals(), [])
			sys.modules['os'] = os
			os.name = name
			break

try:
	import os.path
except ImportError:
	# again, we're running Blender without Python installation,
	# use replacements from Blender.sys or devise your own
	import Blender.sys
	os.path = Blender.sys
	sys.modules['os.path'] = os.path
	def _isdir(path):
		try:
			s = os.stat(path)
		except os.error:
			return False
		return stat.S_ISDIR(s.st_mode)
	os.path.isdir = _isdir
	if os.name in ('nt', 'os2', 'ce'):
		def _split(path):
			if path[1:2] == ':':
				drive, path = path[:2], path[2:]
			else:
				drive = ''
			i = len(path) - 1
			while i >= 0 and not path[i] in '\\/':
				i -= 1
			i += 1
			parent, child = path[:i], path[i:]
			if parent:
				i = len(parent) - 1
				while i >= 0 and parent[i] in '\\/':
					i -= 1
				i += 1
				if i > 0:
					parent = parent[:i]
			return drive + parent, child
	else:
		def _split(path):
			i = path.rfind('/') + 1
			parent, child = path[:i], path[i:]
			if parent and parent != '/' * len(parent):
				parent = parent.rstrip('/')
			return parent, child
	os.path.split = _split
	




def escape(x):
	return x.replace('\\', '\\\\')



def str_float(f):
	format_string = '%%.%ig' % precision
	return format_string % f



def str_index(i):
	if i == None:
		return ''
	return str(i + 1)



def matrix_to_list(matrix):
	return [[matrix[i][j] for i in range(4)] for j in range(4)]



def is_identity_matrix(matrix):
	for i in range(4):
		for j in range(4):
			if i == j:
				if matrix[i][j] != 1:
					return False
			else:
				if matrix[i][j] != 0:
					return False
	return True


def make_dir(dirname):
	if not os.path.exists(dirname):
		parent, child = os.path.split(dirname)
		make_dir(parent)
		os.mkdir(dirname)



def compress_mesh(vertices, uvs, normals, face_groups):
	components = [vertices, uvs, normals]
	# determine used components
	used_indices = [{}, {}, {}]
	for face_group in face_groups:
		for face in face_group:
			for vertex in face:
				for i, UIs in zip(vertex, used_indices):
					if i != None and not i in UIs:
						UIs[i] = len(UIs)
	# compress components
	sorted_indices = [sorted(zip(UIs.values(), UIs.keys())) for UIs in used_indices]
	compressed_components = [[Cs[j] for i, j in SIs] for Cs, SIs in zip(components, sorted_indices)]
	# rebuild faces
	for UIs in used_indices:
		UIs[None] = None
	rebuilt_face_groups = [[[[UIs[i] for i, UIs in zip(v, used_indices)] for v in F] for F in FG] for FG in face_groups]
	# done!
	return compressed_components + [rebuilt_face_groups]



def encode_mesh_to_obj(vertices, normals, uvs, face_groups):
	vertices, uvs, normals, face_groups = compress_mesh(vertices, normals, uvs, face_groups)

	lines = ["# --- vertices ---"]
	for vert in vertices:
		lines += ["v %s" % ' '.join(map(str_float, vert))]
	
	lines += ["", "# --- uvs ---"]
	for uv in uvs:
		lines += ["vt %s" % ' '.join(map(str_float, uv))]
	
	lines += ["", "# --- normals ---"]
	for normal in normals:
		lines += ["vn %s" % ' '.join(map(str_float, normal))]
	
	lines += ["", "# --- face groups ---"]
	for face_group in face_groups:
		lines += ["g"]
		for face in face_group:
			verts = ['/'.join(map(str_index, vert)).rstrip('/') for vert in face]
			lines += ["f %s" % ' '.join(verts)]
			
	return '\n'.join(lines)




class LiarExporter(object):
	def __init__(self, script_path):
		self.script_path = script_path
		self.archive_name = os.path.splitext(script_path)[0]
		self.is_compressing_archive = True

		self.lamps = {}
		self.meshes = {}
		self.materials = {}
		self.textures = {}
		self.images = {}



	def write_to_archive(self, filename, content): 
		print "ARCHIVE NAME", self.archive_name
		if self.is_compressing_archive:
			try:
				import zipfile
				if not isset(self.zip_file):
					zip_name = self.archive_name + ".zip"
					if os.path.exists(self.zip_name):
						os.remove(self.zip_name)
					self.zip_file = zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED)
				self.zip_file.writestr(filename, content)
				return
			except ImportError:
				pass
		
		make_dir(self.archive_name)
		f = file(os.path.join(self.archive_name, filename), 'w')
		f.write(content)
		f.close()           
		


	def write_none(self, obj):
		pass



	def write_image(self, texture):
		assert(texture.getType() == 'Image')
		image_name = texture.image.getName()
		if image_name in self.images:
			return
		self.images[image_name] = True
		
		filename = texture.image.getFilename()
		if texture.mipmap:
			anti_aliasing = 'AA_TRILINEAR'
			mip_mapping = 'MM_ANISOTROPIC'
		else:
			anti_aliasing = 'AA_BILINEAR'
			mip_mapping = 'MM_NONE'
		if not texture.interpol:
			anti_aliasing = 'AA_NONE'
		
		self.script_file.write("""
# --- image IM:%s ---

image = liar.textures.Image(fix_path(r"%s"))
image.antiAliasing = liar.textures.Image.%s
image.mipMapping = liar.textures.Image.%s
images['IM:%s'] = image
""" % (image_name, filename, anti_aliasing, mip_mapping, image_name))



	def write_texture(self, texture):
		if texture.name in self.textures:
			return
		self.textures[texture.name] = True
		
		tex_type = texture.getType()
		if tex_type == 'Image':
			self.write_image(texture)		
			self.script_file.write("""
# --- texture TE:%s ---

textures['TE:%s'] = images['IM:%s']
""" % (texture.name, texture.name, texture.image.getName()))
		
		else:
			if tex_type != 'None':
				print "WARNING: Cannot handle %s textures!" % tex_type
			self.script_file.write("""
# --- texture TE:%s ---

textures['TE:%s'] = liar.Texture.black()
""" % (texture.name, texture.name))



	def write_material(self, material):
		if material.name in self.materials:
			return
		self.materials[material.name] = True
			
		diffuse = tuple([x * material.ref for x in material.rgbCol])
		specular = tuple([x * material.spec for x in material.specCol])
		specular_power = material.hard
		reflectance = tuple([x * material.spec for x in material.mirCol])
		
		diffuse_tex = "liar.textures.Constant(liar.rgb%s)" % (diffuse,)
		if max(specular) > 0:
			specular_tex = "liar.textures.Constant(liar.rgb%s)" % (specular,)
		else:
			specular_tex = None
		specular_power_tex = "liar.textures.Constant(%s)" %specular_power
		#reflectance_tex = "liar.textures.Constant(liar.rgb%s)" % (reflectance,)

		for texture in material.getTextures():
			if texture:
				self.write_texture(texture.tex)
				s = "textures['TE:%s']" % texture.tex.name
				if texture.mapto & Blender.Texture.MapTo['COL']:
					diffuse_tex = s
				if texture.mapto & Blender.Texture.MapTo['CSP']:
					specular_tex = s
				if texture.mapto & Blender.Texture.MapTo['HARD']:
					specular_power_tex = s
				if texture.mapto & Blender.Texture.MapTo['CMIR']:
					reflectance_tex = s

		if specular_tex:
			self.script_file.write(r"""
# --- material MA:%s ---

material = liar.shaders.AshikhminShirley()
material.diffuse = %s
material.specular = %s
material.specularPowerU = material.specularPowerV = %s
materials['MA:%s'] = material
""" % (material.name, diffuse_tex, specular_tex, specular_power_tex, material.name))
		else:
			self.script_file.write(r"""
# --- material MA:%s ---

material = liar.shaders.Lambert()
material.diffuse = %s
materials['MA:%s'] = material
""" % (material.name, diffuse_tex, material.name))		
				


	def write_mesh(self, obj):
		mesh = obj.getData(False, True)
		if mesh.name in self.meshes:
			return
		print dir(mesh)
		self.meshes[mesh.name] = True

		num_materials = len(mesh.materials)
		if num_materials == 0:
			print 'WARNING: no material assigned to mesh, skipping mesh'
			return
		
		for k, v in mesh.properties.iteritems():
			print "property %s: %s" % (k, v)
		
		# extract vertices, normals and uvs
		vertices = [tuple([mvert.co[k] for k in range(3)]) for mvert in mesh.verts]
		if mesh.vertexUV:
			uvs = [tuple([mvert.uvco[k] for k in range(3)]) for mvert in mesh.verts]
		else:
			uvs = []
		normals = [tuple([mvert.no[k] for k in range(3)]) for mvert in mesh.verts]
		
		# extract faces per material
		face_groups = [[] for i in xrange(num_materials)]
		for face in mesh.faces:
			size = len(face.verts)
			assert(size == 3 or size == 4)
			
			faceVerts = [mvert.index for mvert in face.verts]
						
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

			if face.smooth:
				faceNormals = faceVerts
			else:
				normals.append(tuple([face.no[k] for k in range(3)]))
				faceNormals = size * [len(normals) - 1]
			
			indices = zip(faceVerts, faceUvs, faceNormals)
			
			assert(face.mat >= 0 and face.mat < num_materials)
			for k in range(size - 2):
				face_groups[face.mat].append((indices[0], indices[k + 1], indices[k + 2]))
				
		# determine used face groups
		used_groups = [i for i in xrange(num_materials) if len(face_groups[i]) > 0]
		if not used_groups:
			print 'WARNING: mesh has no faces, skipping mesh'
			return
				
		# write used materials
		for i in used_groups:
			self.write_material(mesh.materials[i])
		
		# write groups to zip
		file_name = mesh.name + ".obj"
		content = encode_mesh_to_obj(vertices, uvs, normals, [face_groups[i] for i in used_groups])
		self.write_to_archive(file_name, "# --- %s ---\n\n%s" % (mesh.name, content))
		
		self.script_file.write("""
# --- mesh ME:%s ---

face_groups = load_face_groups_from_archive("%s")
""" % (mesh.name, file_name))

		for i in used_groups:
			self.script_file.write("face_groups[%d].shader = materials['MA:%s']\n" % (i, mesh.materials[i].name))
		self.script_file.write("meshes['ME:%s'] = group_scenery(face_groups)\n" % mesh.name)

		obj_matrix = matrix_to_list(obj.matrix)
		if is_identity_matrix(obj_matrix):
			self.script_file.write(r"""
# --- object OB:%s ---

objects['OB:%s'] = meshes['ME:%s']
""" % (obj.name, obj.name, mesh.name))
		else:
			self.script_file.write(r"""
# --- object OB:%s ---

matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(meshes['ME:%s'], matrix)
""" % (obj.name, obj_matrix, obj.name, mesh.name))



	def write_lamp(self, obj):
		lamp = obj.getData()
		if lamp.name in self.lamps:
			return
		self.lamps[lamp.name] = True
		
		intensity = (lamp.energy * lamp.R, lamp.energy * lamp.G, lamp.energy * lamp.B)
		self.script_file.write("""
# --- lamp LA:%s ---
""" % lamp.name)

		if lamp.type == 0: # point light
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			self.script_file.write(r"""
light = liar.scenery.LightPoint()
light.position = (0, 0, 0)
light.intensity = liar.rgb%s
light.attenuation = liar.Attenuation(0, %s, %s)
lamps['LA:%s'] = light
""" % (intensity, attLin, attSqr, lamp.name))

		elif lamp.type == 1: # sun
			self.script_file.write(r"""
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
			self.script_file.write(r"""
light = liar.scenery.LightSpot()
light.position = (0, 0, 0)
light.direction = (0, 0, -1)
light.intensity = liar.rgb%s
light.attenuation = liar.Attenuation(0, %s, %s)
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
			self.script_file.write(r"""
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
		self.script_file.write(r"""
# --- object OB:%s ---

matrix = %s
objects['OB:%s'] = liar.scenery.Transformation(lamps['LA:%s'], matrix)
""" % (obj.name, obj_matrix, obj.name, lamp.name))



	def write_camera(self, obj):
		camera = obj.getData()
		type = ('PerspectiveCamera', 'OrthographicCamera')[camera.getType()]
		position = obj.getLocation()
		matrix = obj.matrix
		direction = tuple([-matrix[2][k] for k in range(3)])
		right = tuple([matrix[0][k] for k in range(3)])
		down = tuple([-matrix[1][k] for k in range(3)])
		fov = camera.angle * math.pi / 180
		focus_dist = camera.dofDist
		f_stop = 5 # when will this be available from the Python API?
		self.script_file.write(r"""
# --- camera CA:%s ---

camera = liar.cameras.%s()
camera.position = %s
camera.direction = %s
camera.right = %s
camera.down = %s
camera.fovAngle = %s
camera.focusDistance = %s
camera.fNumber = %s
cameras['CA:%s'] = camera
""" % (camera.name, type, position, direction, right, down, fov, focus_dist, f_stop, camera.name))



	def write_context(self, context):
		width = context.imageSizeX()
		height = context.imageSizeY()
		aspect_ratio = float(width * context.aspectRatioX()) / (height * context.aspectRatioY())
		samples_per_pixel = 1
		out_file = os.path.splitext(self.script_path)[0] + ".hdr"	
		self.script_file.write(r"""
width = %s
height = %s
samples_per_pixel = %s
aspect_ratio = %s
out_file = fix_path(r"%s")

for cam in cameras.values():
	cam.aspectRatio = aspect_ratio
sampler = liar.samplers.Stratifier((width, height), samples_per_pixel)
target = liar.output.Image(out_file, (width, height))

try:
	Display = liar.output.Display
except:
	print "no display output possible, unsupported by liar installation"
else:
	w = min(width, 400)
	h = int(w * height / float(width))
	display = Display(out_file, (w, h))
	target = liar.output.Splitter([target, display])
""" % (width, height, samples_per_pixel, aspect_ratio, out_file))



	def write_header(self):
		self.script_file.write(r"""#! /usr/bin/python

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

archive_dir = r"%s"
zip_path = archive_dir + ".zip"
if os.path.exists(zip_path):
	import ziplib
	zip_file = zipfile.ZipFile(zip_path, 'r')
else:
	zip_file = None

def load_from_archive(file_name):
	file_path = os.path.join(archive_dir, file_name)
	if os.path.exists(file_path):
		return file(file_path).read()
	if zip_file:
		return zip_file.read(file_name)
	raise IOError, "file '%%s' not found, nor in archive directory '%%s', nor in zip file '%%s'" %% (file_name, archive_dir, zip_path)

def load_face_groups_from_archive(mesh_file):
	import liar.tools.wavefront_obj
	obj = load_from_archive(mesh_file).splitlines()
	return liar.tools.wavefront_obj.decode(obj)

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

def group_scenery(scenery_objects):
	assert len(scenery_objects) > 0
	if len(scenery_objects) == 1:
		return scenery_objects[0]
	else:
		return liar.scenery.AabpTree(scenery_objects)
""" % self.archive_name)



	def write_body(self):
		object_writers = {
			'Mesh': LiarExporter.write_mesh,
			'Lamp': LiarExporter.write_lamp,
			'Camera': LiarExporter.write_camera
			}
			
		scene = Blender.Scene.GetCurrent()
		print dir(scene)
		children = scene.getChildren()
		num_children = len(children)
		
		for obj, i in zip(children, range(num_children)):
			obj_type = obj.getType()
			obj_name = obj.getName()
			print "- object '%s' of type '%s'" % (obj_name, obj_type)
			obj_materials = obj.getMaterials()
			if obj_type in object_writers:              
				object_writers[obj_type](self, obj)
			else:
				print "WARNING: Did not recognize object type '%s'" % obj_type
				
			ipo = obj.getIpo()
			if ipo:
				print ipo
				print ipo.getName()
				for curve in ipo.getCurves():
					print curve
					print curve.getPoints()
					
			self.draw_progress(float(i+1) / num_children)

				
		context = scene.getRenderingContext()
		self.write_context(context)
			

			
	def write_footer(self):
		scene = Blender.Scene.GetCurrent()
		active_camera = scene.objects.camera.getData().name
		self.script_file.write('''
engine = liar.RenderEngine()
engine.tracer = liar.tracers.DirectLighting()
engine.sampler = sampler
engine.scene = liar.scenery.AabpTree(objects.values())
engine.camera = cameras['CA:%s']
engine.target = target

if __name__ == "__main__":
	engine.render()
''' % active_camera)



	def draw_progress(self, progress):
		Blender.Window.DrawProgressBar(float(progress), "Exporting scene to LiAR [%3d%%]" % int(100 * progress))



	def export(self):
		if os.path.exists(self.script_path):
			if not Blender.Draw.PupMenu("Overwrite File?%t|Yes%x1|No%x0"):
				return
			
		print "Exporting to LiAR '%s' ..." % self.script_path
		self.draw_progress(0)
		
		self.script_file = file(self.script_path, 'w')
		self.write_header()
		self.write_body()
		self.write_footer()
		self.script_file.close()

		self.draw_progress(1)
		print "Done ..."



def export_callback(script_path):   
	exporter = LiarExporter(script_path)
	exporter.export()


suggested_basename = os.path.splitext(Blender.Get('filename'))[0]
if suggested_basename == '<memory>':
	suggested_basename = 'untitled'
suggested_filename = suggested_basename + "_liar.py"
Blender.Window.FileSelector(export_callback, "Export to LiAR", suggested_filename)
