#!BPY

# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
	format_string = '%r'
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
	if os.path.exists(dirname):
		return
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


def const_texture(colour, value = 1):
	try:
		tuple(colour)
	except TypeError:
		return "liar.textures.Constant(%r)" % (value * colour)
	else:
		colour = tuple(x * value for x in colour)
		return "liar.textures.Constant(liar.rgb%r)" % (colour,)


class LiarExporter(object):
	def __init__(self, script_path):
		self.script_path = script_path
		self.archive_path = os.path.splitext(script_path)[0]
		try:
			import zipfile
		except ImportError:
			self.is_compressing_archive = False
		else:
			self.is_compressing_archive = True
		self.lamps = {}
		self.meshes = {}
		self.materials = {}
		self.textures = {}
		self.images = {}
	
	def write_to_archive(self, filename, content): 
		if self.is_compressing_archive:
			try:
				self.zip_file
			except AttributeError:
				import zipfile
				zip_path = self.archive_path + ".zip"
				if os.path.exists(zip_path):
					os.remove(zip_path)
				self.zip_file = zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED)
			self.zip_file.writestr(filename, content)
		else:
			make_dir(self.archive_path)
			f = file(os.path.join(self.archive_path, filename), 'w')
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
		
		self.script_file.write('''
# --- image IM:%(image_name)s ---
image = liar.textures.Image(fix_path(%(filename)r))
image.antiAliasing = liar.textures.Image.%(anti_aliasing)s
image.mipMapping = liar.textures.Image.%(mip_mapping)s
images['IM:%(image_name)s'] = image
''' % vars())
	
	def write_texture(self, texture):
		name = texture.name
		if name in self.textures:
			return
		self.textures[name] = True
		
		tex_type = texture.getType()
		if tex_type == 'Image':
			self.write_image(texture)
			image_name = texture.image.getName()
			self.script_file.write('''
# --- texture TE:%(name)s ---
textures['TE:%(name)s'] = images['IM:%(image_name)s']
''' % vars())
		elif tex_type != 'None':
			print("WARNING: Cannot handle %s textures!" % tex_type)
			self.script_file.write('''
# --- texture TE:%(name)s ---
textures['TE:%(name)s'] = liar.Texture.black()
''' % vars())
		return "textures['TE:%(name)s']" % vars()
	
	def write_material(self, material):
		name = material.name
		if name in self.materials:
			return
		self.materials[name] = True
		
		channels = ""
		for index in material.enabledTextures:
			mtex = material.textures[index]
			if not mtex:
				continue
			channel = self.write_texture(mtex.tex)
			if mtex.texco == Blender.Texture.TexCo['UV']:
				ox, oy = mtex.ofs[:2]
				sx, sy = mtex.size[:2]
				ox += (1 - sx) / 2
				oy += (1 - sy) / 2
				#sy, oy = -sx, -oy
				if ox != 0 or oy != 0 or sx != 1 or sy != 1:
					channel = "liar.textures.TransformationUv(%(channel)s, [(%(sx)r, 0, %(ox)r), (0, %(sy)r, %(oy)r), (0, 0, 1)])" % vars()
			else:
				size = [s / 2 for s in mtex.size]
				ofs = [o + s for o, s in zip(mtex.ofs, size)]
				if mtex.mapping == Blender.Texture.Mappings['CUBE']:
					channel = "liar.textures.CubeMapping(%s)" % channel
					#size = [s / 2 for s in size]
					#ofs = [(o + 1) / 2 for o in ofs]
				sx, sy, sz = size
				ox, oy, oz = ofs
				if ox != 0 or oy != 0 or oz != 0 or sx != 1 or sy != 1 or sz != 1:
					channel = "liar.textures.TransformationLocal(%(channel)s, [(%(sx)r, 0, 0, %(ox)r), (0, %(sy)r, 0, %(oy)r), (0, 0, %(sz)r, %(oz)r), (0, 0, 0, 1)])" % vars()
				if mtex.texco == Blender.Texture.TexCo['ORCO']:
					channel = "liar.textures.OrCo(%s)" % channel
			channels += "\n\t%(index)d: %(channel)s," % vars()
		
		if material.alpha == 0:
			ior = material.IOR
			self.script_file.write(r'''
# --- material MA:%(name)s ---
material = liar.shaders.Dielectric()
material.innerRefractionIndex = liar.textures.Constant(%(ior)r)
materials['MA:%(name)s'] = material
''' % vars())
			return
		
		diffuse = material.ref and const_texture(material.rgbCol, material.ref)
		specular = material.spec and const_texture(material.specCol, material.spec)
		specular_power = "liar.textures.Constant(%r)" % material.hard
		mirror = material.rayMirr and const_texture(material.mirCol, material.rayMirr)
		bumpmap = None
		
		for channel in material.enabledTextures:
			mtex = material.textures[channel]
			if not mtex:
				continue
			tex = "texture_channels[%d]" % channel
			if mtex.mapto & Blender.Texture.MapTo['COL']:
				diffuse = tex
			if mtex.mapto & Blender.Texture.MapTo['CSP']:
				specular = tex
			if mtex.mapto & Blender.Texture.MapTo['HARD']:
				specular_power = tex
			if mtex.mapto & Blender.Texture.MapTo['NOR']:
				bumpmap = tex
				
		self.script_file.write(r'''
# --- material MA:%(name)s ---
texture_channels = {%(channels)s
	}
material_components = []
''' % vars())
		
		if specular:
			diff = diffuse or const_texture(0)
			self.script_file.write(r'''
component = liar.shaders.AshikhminShirley()
component.diffuse = %(diff)s
component.specular = %(specular)s
component.specularPowerU = component.specularPowerV = %(specular_power)s
material_components.append(component)
''' % vars())
		elif diffuse:
			self.script_file.write(r'''
component = liar.shaders.Lambert()
component.diffuse = %(diffuse)s
material_components.append(component)
''' % vars())
		if mirror:
			self.script_file.write(r'''
component = liar.shaders.Mirror()
component.reflectance = %(mirror)s
material_components.append(component)
''' % vars())
		
		self.script_file.write(r'''
materials['MA:%(name)s'] = make_material(material_components, %(bumpmap)s)
''' % vars())
	
	def write_mesh(self, obj):
		mesh = obj.getData(False, True)
		name = mesh.name
		if name in self.meshes:
			return
		self.meshes[name] = True
		
		num_materials = len(mesh.materials)
		if num_materials == 0:
			print('WARNING: no material assigned to mesh, skipping mesh')
			return
		
		for k, v in mesh.properties.iteritems():
			print("property %s: %s" % (k, v))
		
		# extract vertices, normals and uvs
		vertices = [tuple(mvert.co) for mvert in mesh.verts]
		if mesh.vertexUV:
			uvs = [tuple(mvert.uvco) for mvert in mesh.verts]
		else:
			uvs = []
		normals = [tuple(mvert.no) for mvert in mesh.verts]
		
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
				normals.append(tuple(face.no))
				faceNormals = size * [len(normals) - 1]
			
			indices = list(zip(faceVerts, faceUvs, faceNormals))
			assert(face.mat >= 0 and face.mat < num_materials)
			face_groups[face.mat].append(indices)
				
		# determine used face groups
		used_groups = [i for i in xrange(num_materials) if len(face_groups[i]) > 0]
		if not used_groups:
			print('WARNING: mesh has no faces, skipping mesh')
			return
				
		# write used materials
		for i in used_groups:
			self.write_material(mesh.materials[i])
		
		# write groups to zip
		file_name = mesh.name + ".obj"
		content = encode_mesh_to_obj(vertices, uvs, normals, [face_groups[i] for i in used_groups])
		self.write_to_archive(file_name, "# --- %(name)s ---\n\n%(content)s" % vars())
		
		if mesh.mode & Blender.Mesh.Modes['AUTOSMOOTH']:
			auto_smooth_angle = ", auto_smooth_angle=%r" % mesh.maxSmoothAngle
		else:
			auto_smooth_angle = ""
		
		self.script_file.write('''
# --- mesh ME:%(name)s ---
face_groups = load_face_groups_from_archive(%(file_name)r%(auto_smooth_angle)s)
''' % vars())

		for i in used_groups:
			mat = "materials['MA:%s']" % mesh.materials[i].name
			self.script_file.write("face_groups[%(i)d].shader = %(mat)s\n" % vars())
		self.script_file.write("meshes['ME:%s'] = group_scenery(face_groups)\n" % name)

		obj_matrix = matrix_to_list(obj.matrix)
		obj_name = obj.name
		if is_identity_matrix(obj_matrix):
			self.script_file.write(r'''
# --- object OB:%(obj_name)s ---
objects['OB:%(obj_name)s'] = meshes['ME:%(name)s']
''' % vars())
		else:
			self.script_file.write(r'''
# --- object OB:%(obj_name)s ---
matrix = %(obj_matrix)s
objects['OB:%(obj_name)s'] = liar.scenery.Transformation(meshes['ME:%(name)s'], matrix)
''' % vars())
	
	def write_lamp(self, obj):
		lamp = obj.getData()
		name = lamp.name
		if name in self.lamps:
			return
		self.lamps[name] = True
		intensity = (lamp.energy * lamp.R, lamp.energy * lamp.G, lamp.energy * lamp.B)
		if lamp.type == 0: # point light
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light = liar.scenery.LightPoint()
light.position = (0, 0, 0)
light.intensity = liar.rgb%(intensity)r
light.attenuation = liar.Attenuation(0, %(attLin)r, %(attSqr)r)
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 1: # sun
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light = liar.scenery.LightDirectional()
light.direction = (0, 0, -1)
light.radiance = liar.rgb%(intensity)r
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 2: # spot
			if lamp.mode & Blender.Lamp.Modes['Quad']:
				attLin = lamp.Quad1
				attSqr = lamp.Quad3
			else:
				attLin = 0
				attSqr = 1
			outer_angle = lamp.spotSize / 2.0
			inner_angle = outer_angle * (1 - lamp.spotBlend)
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light.position = (0, 0, 0)
light.direction = (0, 0, -1)
light.intensity = liar.rgb%(intensity)r
light.attenuation = liar.Attenuation(0, %(attLin)r, %(attSqr)r)
light.outerAngle = liar.radians(%(outer_angle)r)
light.innerAngle = liar.radians(%(inner_angle)r)
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 4: # area
			is_square = lamp.mode & Blender.Lamp.Modes['Square']
			if is_square:
				size_x = size_y = lamp.areaSizeX
				number_of_samples = lamp.raySamplesX
			else:
				size_x = lamp.areaSizeX
				size_y = lamp.areaSizeY
				number_of_samples = lamp.raySamplesX * lamp.raySamplesY
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
sizeX = %(size_x)r
sizeY = %(size_y)r
surface = liar.scenery.Parallelogram((-sizeX / 2., sizeY / 2., 0), (sizeX, 0, 0), (0, -sizeY, 0))
light = liar.scenery.LightArea(surface)
light.radiance = liar.rgb%(intensity)r
light.numberOfEmissionSamples = %(number_of_samples)r
lamps['LA:%(name)s'] = light
''' % vars())

		else:
			print("WARNING: Did not recognize lamp type '%s'" % lamp.type)
			return
		
		obj_matrix = matrix_to_list(obj.matrix)
		obj_name = obj.name
		self.script_file.write(r'''
# --- object OB:%(obj_name)s ---
matrix = %(obj_matrix)r
objects['OB:%(obj_name)s'] = liar.scenery.Transformation(lamps['LA:%(name)s'], matrix)
''' % vars())
	
	def write_camera(self, obj):
		camera = obj.getData()
		name = camera.name
		type = ('PerspectiveCamera', 'OrthographicCamera')[camera.getType()]
		position = obj.getLocation()
		matrix = obj.matrix
		direction = tuple([-matrix[2][k] for k in range(3)])
		right = tuple([matrix[0][k] for k in range(3)])
		down = tuple([-matrix[1][k] for k in range(3)])
		fov = camera.angle
		focus_dist = camera.dofDist
		f_stop = 4 # when will this be available from the Python API?
		self.script_file.write(r'''
# --- camera CA:%(name)s ---
camera = liar.cameras.%(type)s()
camera.position = %(position)s
camera.direction = %(direction)s
camera.right = %(right)s
camera.down = %(down)s
camera.fovAngle = math.radians(%(fov)r)
camera.focusDistance = %(focus_dist)s
camera.fNumber = %(f_stop)s
cameras['CA:%(name)s'] = camera
''' % vars())
	
	def write_context(self, context):
		from Blender.Scene import Render
		image_types = {
			Render.HDR: ".hdr",
			Render.OPENEXR: ".exr",
			Render.JPEG: ".jpg",
			Render.TARGA: ".tga"
			}
		renderSize = 0.01 * context.renderwinSize
		width = int(math.ceil(renderSize * context.sizeX))
		height = int(math.ceil(renderSize * context.sizeY))
		aspect_ratio = float(width * context.aspectRatioX()) / (height * context.aspectRatioY())
		samples_per_pixel = (1, context.OSALevel)[context.oversampling]
		out_file = '//' + os.path.basename(os.path.splitext(self.script_path)[0]) + image_types.get(context.imageType, ".jpg")
		options = ""
		if context.imageType in (Render.JPEG,):
			options += " --quality=%d" % context.quality()
		self.script_file.write(r'''
width = %(width)r
height = %(height)r
samples_per_pixel = %(samples_per_pixel)r
aspect_ratio = %(aspect_ratio)r
out_file = fix_path(%(out_file)r)
options = %(options)r

for cam in cameras.values():
	cam.aspectRatio = aspect_ratio
sampler = liar.samplers.Stratifier((width, height), samples_per_pixel)
target = liar.output.Image(out_file, (width, height))
target.options = options
target = liar.output.FilterMitchell(target)

try:
	Display = liar.output.Display
except AttributeError:
	print("no display output possible, unsupported by liar installation")
else:
	w = min(width, 1000)
	h = int(w * height / float(width))
	display = Display(out_file, (w, h))
	target = liar.output.Splitter([target, liar.output.FilterMitchell(display)])
''' % vars())

	def write_header(self):
		archive_path = '//' + os.path.basename(self.archive_path)
		self.script_file.write(r'''#! /usr/bin/python

# Exported from Blender by liar_export
import liar
import os.path
import math

objects = {}
cameras = {}
lamps = {}
meshes = {}
materials = {}
textures = {}
images = {}

self_dir = os.path.dirname(os.path.abspath(__file__))
def fix_path(path):
	if path.startswith('//'):
		path = os.path.join(self_dir, path[2:])
	if os.path.altsep:
		path = path.replace(os.path.altsep, os.path.sep)
	return path

archive_dir = fix_path(%(archive_path)r)
zip_path = archive_dir + ".zip"
if os.path.exists(zip_path):
	import zipfile
	zip_file = zipfile.ZipFile(zip_path, 'r')
else:
	zip_file = None

def load_from_archive(file_name):
	file_path = os.path.join(archive_dir, file_name)
	if os.path.exists(file_path):
		return file(file_path).read()
	if zip_file:
		return zip_file.read(file_name)
	raise IOError("file %%(file_name)r not found, nor in archive directory %%(archive_dir)r, nor in zip file %%(zip_path)r" %% vars())

def load_face_groups_from_archive(mesh_file, auto_smooth_angle=None):
	import liar.tools.wavefront_obj
	obj = load_from_archive(mesh_file).splitlines()
	parts, materials = liar.tools.wavefront_obj.decode(obj, filename=mesh_file)
	if auto_smooth_angle:
		for part in parts:
			part.autoCrease(1, math.radians(auto_smooth_angle))
			part.smoothNormals()
	return parts

def int_or_none(s):
	try:
		return int(s)
	except:
		return None

def make_material(material_components, bumpmap=None):
	if not material_components:
		return None
	try:
		shader, = material_components
	except ValueError:
		shader = liar.shaders.Sum(material_components)
	if bumpmap:
		shader = liar.shaders.BumpMapping(shader, bumpmap)
	return shader

def group_scenery(scenery_objects):
	assert len(scenery_objects) > 0
	if len(scenery_objects) == 1:
		return scenery_objects[0]
	else:
		return liar.scenery.AabpTree(scenery_objects)
''' % vars())

	def write_world(self, context):
		world = Blender.World.GetCurrent()
		if not world:
			return
		name = world.name
		horiz = None#const_texture
		for mtex in world.textures:
			if not mtex:
				continue
			texture = orig_texture = self.write_texture(mtex.tex)
			print mtex.texco
			if mtex.texco == Blender.Texture.TexCo['ANGMAP']:
				print "been here"
				texture = "liar.textures.AngularMapping(%s)" % texture
			if mtex.mtHoriz:
				horiz = texture
		if not horiz:
			return
		shader = None
		if context.alphaMode == 0:
			shader = "liar.shaders.Unshaded(world.radiance)"
		self.script_file.write(r'''
# --- world WO:%(name)s ---
world = liar.scenery.LightSky(%(horiz)s)
try:
	world.samplingResolution = %(orig_texture)s.resolution
except AttributeError:
	pass
world.numberOfEmissionSamples = 64
world.shader = %(shader)s
objects['WO:%(name)s'] = world
''' % vars())

	def write_body(self):
		object_writers = {
			'Mesh': LiarExporter.write_mesh,
			'Lamp': LiarExporter.write_lamp,
			'Camera': LiarExporter.write_camera,
			'Empty': LiarExporter.write_none,
			}
			
		scene = Blender.Scene.GetCurrent()
		visible_layers = set(scene.getLayers())
		print "scene layers", visible_layers
		
		for i, obj in enumerate(scene.objects):
			if not visible_layers & set(obj.layers):
				continue
			obj_type = obj.getType()
			obj_name = obj.getName()
			print("- object '%(obj_name)s' of type '%(obj_type)s'" % vars())
			obj_materials = obj.getMaterials()
			if obj_type in object_writers:              
				object_writers[obj_type](self, obj)
			else:
				print("WARNING: Did not recognize object type '%(obj_type)s'" % vars())
				
			for key, value in obj.properties.iteritems():
				print key, value
				
			ipo = obj.getIpo()
			if ipo:
				print(ipo)
				print(ipo.getName())
				for curve in ipo.getCurves():
					print(curve)
					print(curve.getPoints())
					
			self.draw_progress(float(i+1) / len(scene.objects))
			
		self.write_world(scene.render)		
		self.write_context(scene.render)
	
	def write_footer(self):
		scene = Blender.Scene.GetCurrent()
		active_camera = scene.objects.camera.getData().name
		if scene.render.mode & Blender.Scene.Render.Modes["FIXEDTHREADS"]:
			threads = scene.render.threads
		else:
			threads = "liar.RenderEngine.AUTO_NUMBER_OF_THREADS"
		if scene.render.borderRender:
			x1, y1, x2, y2 = scene.render.border
			x1, x2 = min(x1, x2), max(x1, x2)
			y1, y2 = min(y1, y2), max(y1, y2)
			window = ((x1, 1-y2), (x2, 1-y1))
		else:
			window = ((0, 0), (1, 1))
		self.script_file.write('''
engine = liar.RenderEngine()
engine.tracer = liar.tracers.DirectLighting()
engine.sampler = sampler
engine.scene = liar.scenery.AabpTree(list(objects.values()))
engine.camera = cameras['CA:%(active_camera)s']
engine.target = target
engine.numberOfThreads = %(threads)s

def render():
	engine.render(%(window)r)

if __name__ == "__main__":
	render()
''' % vars())



	def draw_progress(self, progress):
		Blender.Window.DrawProgressBar(float(progress), "Exporting scene to LiAR [%3d%%]" % int(100 * progress))



	def export(self):
		if os.path.exists(self.script_path):
			if not Blender.Draw.PupMenu("Overwrite File?%t|Yes%x1|No%x0"):
				return
			
		print("Exporting to LiAR '%s' ..." % self.script_path)
		self.draw_progress(0)
		
		self.script_file = file(self.script_path, 'w')
		self.write_header()
		self.write_body()
		self.write_footer()
		self.script_file.close()

		self.draw_progress(1)
		print("Done ...")



def export_callback(script_path):   
	exporter = LiarExporter(script_path)
	exporter.export()


suggested_basename = os.path.splitext(Blender.Get('filename'))[0]
if suggested_basename == '<memory>':
	suggested_basename = 'untitled'
suggested_filename = suggested_basename + "_liar.py"
Blender.Window.FileSelector(export_callback, "Export to LiAR", suggested_filename)
