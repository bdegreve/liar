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

precision = 7


import bpy
import sys
import math
import os
import extensions_framework as ef
from . import liar_log


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
				if matrix[i][j]:
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
	if value <= 0:
		return None
	colour = tuple(x * value for x in colour)
	return "liar.textures.Constant(liar.rgb%r)" % (colour,)


def _matching_layers(As, Bs):
	return any(a and b for a, b in zip(As, Bs))


def export_scene(scene=None, script_path=None):
	exporter = LiarExporter()
	return exporter.export(scene, script_path)


class LiarExporter(object):
	def __init__(self):
		try:
			import zipfile
		except ImportError:
			self.is_compressing_archive = False
		else:
			self.is_compressing_archive = True
	
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
		assert(texture.type == 'IMAGE')
		name = texture.image.name
		if name in self.images:
			return
		self.images[name] = True
		
		filename = texture.image.filepath
		if texture.use_mipmap or texture.use_mipmap_gauss:
			anti_aliasing = 'AA_TRILINEAR'
			mip_mapping = 'MM_ANISOTROPIC'
		else:
			anti_aliasing = 'AA_BILINEAR'
			mip_mapping = 'MM_NONE'
		if not texture.use_interpolation:
			anti_aliasing = 'AA_NONE'
		
		self.script_file.write('''
# --- image IM:%(name)s ---
image = liar.textures.Image(fix_path(%(filename)r))
image.antiAliasing = liar.textures.Image.%(anti_aliasing)s
image.mipMapping = liar.textures.Image.%(mip_mapping)s
images['IM:%(name)s'] = image
''' % vars())
		return name
	
	def write_texture(self, texture):
		name = texture.name
		if name in self.textures:
			return
		self.textures[name] = True
		
		if texture.type == 'IMAGE':
			image_name = self.write_image(texture)
			self.script_file.write('''
# --- texture TE:%(name)s ---
textures['TE:%(name)s'] = images['IM:%(image_name)s']
''' % vars())
		else:
			print("WARNING: Cannot handle %s textures!" % texture.type)
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
		for index, slot in enumerate(material.texture_slots):
			if not slot or not slot.use:
				continue
			channel = self.write_texture(slot.texture)
			if slot.texture_coords == 'UV':
				ox, oy = slot.offset[:2]
				sx, sy = slot.scale[:2]
				ox += (1 - sx) / 2
				oy += (1 - sy) / 2
				#sy, oy = -sx, -oy
				if ox != 0 or oy != 0 or sx != 1 or sy != 1:
					channel = "liar.textures.TransformationUv(%(channel)s, [(%(sx)r, 0, %(ox)r), (0, %(sy)r, %(oy)r), (0, 0, 1)])" % vars()
			else:
				size = [s / 2 for s in slot.scale]
				offset = [o + s for o, s in zip(slot.offset, size)]
				if slot.mapping == 'CUBE':
					channel = "liar.textures.CubeMapping(%s)" % channel
					#size = [s / 2 for s in size]
					#offset = [(o + 1) / 2 for o in offset]
				sx, sy, sz = size
				ox, oy, oz = offset
				if ox != 0 or oy != 0 or oz != 0 or sx != 1 or sy != 1 or sz != 1:
					channel = "liar.textures.TransformationLocal(%(channel)s, [(%(sx)r, 0, 0, %(ox)r), (0, %(sy)r, 0, %(oy)r), (0, 0, %(sz)r, %(oz)r), (0, 0, 0, 1)])" % vars()
				if slot.texture_coords == 'ORCO':
					channel = "liar.textures.OrCo(%s)" % channel
			channels += "\n\t%(index)d: %(channel)s," % vars()
		
		if material.alpha == 0:
			ior = material.specular_ior
			self.script_file.write(r'''
# --- material MA:%(name)s ---
material = liar.shaders.Dielectric()
material.innerRefractionIndex = liar.textures.Constant(%(ior)r)
materials['MA:%(name)s'] = material
''' % vars())
			return
		
		diffuse = const_texture(material.diffuse_color, material.diffuse_intensity)
		specular = const_texture(material.specular_color, material.specular_intensity)
		specular_power = "liar.textures.Constant(%r)" % material.specular_hardness
		bumpmap = None
		
		for index, slot in enumerate(material.texture_slots):
			if not slot or not slot.use:
				continue
			channel = "texture_channels[%d]" % index
			if slot.use_map_color_diffuse:
				diffuse = channel
			if slot.use_map_color_spec:
				specular = channel
			if slot.use_map_hardness:
				specular_power = channel
			if slot.use_map_normal:
				bumpmap = channel
		
		if specular:
			self.script_file.write(r'''
# --- material MA:%(name)s ---
material = liar.shaders.AshikhminShirley()
texture_channels = {%(channels)s
	}
material.diffuse = %(diffuse)s
material.specular = %(specular)s
material.specularPowerU = material.specularPowerV = %(specular_power)s
''' % vars())
		else:
			self.script_file.write(r'''
# --- material MA:%(name)s ---
material = liar.shaders.Lambert()
texture_channels = {%(channels)s
	}
material.diffuse = %(diffuse)s
''' % vars())
		if bumpmap:
			self.script_file.write("material = liar.shaders.BumpMapping(material, %(bumpmap)s)\n" % vars())
		self.script_file.write("materials['MA:%(name)s'] = material\n" % vars())
	
	def write_mesh(self, obj, mesh):
		name = mesh.name
		if name in self.meshes:
			return
		self.meshes[name] = True
		
		if not mesh.materials:
			print('WARNING: no material assigned to mesh, skipping mesh')
			return
		
		#for k, v in mesh.properties.iteritems():
		#    print("property %s: %s" % (k, v))
		
		# extract vertices, normals and uvs
		vertices = [tuple(v.co) for v in mesh.vertices]
		normals = [tuple(v.normal) for v in mesh.vertices]
		uvs = mesh.sticky
		
		# extract faces per material
		face_groups = [[]] * len(mesh.materials)
		for face in mesh.faces:
			size = len(face.vertices)
			assert(size == 3 or size == 4)
			
			faceVerts = face.vertices
			
			if mesh.sticky:
				faceUvs = faceVerts
			#elif mesh.faceUV:
			#    assert(len(face.uv) == size)
			#    faceUvs = []
			#    for muv in face.uv:
			#        uvs.append(tuple([muv[k] for k in range(2)]))
			#        faceUvs.append(len(uvs) - 1)
			else:
				faceUvs = size * [None]

			if face.use_smooth:
				faceNormals = faceVerts
			else:
				normals.append(tuple(face.normal))
				faceNormals = size * [len(normals) - 1]
			
			indices = tuple(zip(faceVerts, faceUvs, faceNormals))
			face_groups[face.material_index].append(indices)
				
		# determine used face groups
		used_groups = [i for i, group in enumerate(face_groups) if group]
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
		
		if mesh.use_auto_smooth:
			auto_smooth_angle = ", auto_smooth_angle=%r" % mesh.auto_smooth_angle
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
		return "meshes['ME:%s']" % name
	
	def write_lamp(self, obj, lamp):
		name = lamp.name
		if name in self.lamps:
			return
		self.lamps[name] = True
		
		def attenuation(lamp):
			if lamp.falloff_type == 'CONSTANT':
				return 0, 0
			if lamp.falloff_type == 'INVERSE_LINEAR':
				return 1, 0
			if lamp.falloff_type == 'INVERSE_SQUARE':
				return 0, 1
			if lamp.falloff_type == 'LINEAR_QUADRATIC_WEIGHTED':
				return lamp.linear_attenuation, quadratic_attenuation
			else:
				return 0, 1
		intensity = tuple(lamp.energy * x for x in lamp.color)
		
		if lamp.type == 'POINT':
			attLin, attSqr = attenuation(lamp)
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light = liar.scenery.LightPoint()
light.position = (0, 0, 0)
light.intensity = liar.rgb%(intensity)r
light.attenuation = liar.Attenuation(0, %(attLin)r, %(attSqr)r)
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 'SUN':
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light = liar.scenery.LightDirectional()
light.direction = (0, 0, -1)
light.radiance = liar.rgb%(intensity)r
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 'SPOT':
			attLin, attSqr = attenuation(lamp)
			outer_angle = lamp.spot_size / 2.0
			inner_angle = outer_angle * (1 - lamp.spot_blend)
			self.script_file.write(r'''
# --- lamp LA:%(name)s ---
light = liar.scenery.LightSpot()
light.position = (0, 0, 0)
light.direction = (0, 0, -1)
light.intensity = liar.rgb%(intensity)r
light.attenuation = liar.Attenuation(0, %(attLin)r, %(attSqr)r)
light.outerAngle = %(outer_angle)r
light.innerAngle = %(inner_angle)r
lamps['LA:%(name)s'] = light
''' % vars())

		elif lamp.type == 'AREA':
			if lamp.shape == 'SQUARE':
				size_x = size_y = lamp.size
				number_of_samples = lamp.shadow_ray_samples_x
			else:
				size_x = lamp.size
				size_y = lamp.size_y
				number_of_samples = lamp.shadow_ray_samples_x * lamp.shadow_ray_samples_y
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
			return None

		return "lamps['LA:%s']" % name

	def write_camera(self, obj, camera):
		name = camera.name
		type = {'PERSP':'PerspectiveCamera', 'ORTHO':'OrthographicCamera'}[camera.type]
		position = tuple(obj.location)
		matrix = obj.matrix_world
		direction = tuple(-matrix[2])[:3]
		right = matrix[0][:3]
		down = tuple(-matrix[1])[:3]
		fov = camera.angle
		focus_dist = camera.dof_distance
		f_stop = 4 # when will this be available from the Python API?
		self.script_file.write(r'''
# --- camera CA:%(name)s ---
camera = liar.cameras.%(type)s()
camera.position = %(position)s
camera.direction = %(direction)s
camera.right = %(right)s
camera.down = %(down)s
camera.fovAngle = %(fov)r
camera.focusDistance = %(focus_dist)s
camera.fNumber = %(f_stop)s
cameras['CA:%(name)s'] = camera
''' % vars())
 
	def write_render(self, render):
		file_formats = {
			'HDR': ".hdr",
			'OPEN_EXR': ".exr",
			'JPEG': ".jpg",
			'TARGA': ".tga"
			}
		render_size = 0.01 * render.resolution_percentage
		width = int(math.ceil(render_size * render.resolution_x))
		height = int(math.ceil(render_size * render.resolution_y))
		aspect_ratio = float(width * render.pixel_aspect_x) / (height * render.pixel_aspect_y)
		samples_per_pixel = int(render.use_antialiasing and render.antialiasing_samples or 1)
		extension = file_formats.get(render.file_format, ".tga")
		out_file = '//' + os.path.basename(os.path.splitext(self.script_path)[0]) + extension
		options = ""
		if extension in (".jpg",):
			options += " --quality=%d" % render.file_quality
		self.script_file.write(r'''
width = %(width)r
height = %(height)r
samples_per_pixel = %(samples_per_pixel)r
aspect_ratio = %(aspect_ratio)r
out_file = fix_path(%(out_file)r)

for cam in cameras.values():
	cam.aspectRatio = aspect_ratio
sampler = liar.samplers.Stratifier((width, height), samples_per_pixel)

def make_target(width=width, height=height, display=True, remote=None):
	targets = []

	target = liar.output.Image(out_file, (width, height))
	target.options = %(options)r
	targets.append(liar.output.FilterMitchell(target))

	if display:
		try:
			Display = liar.output.Display
		except AttributeError:
			print("no display output possible, unsupported by liar installation")
		else:
			w = min(width, 1000)
			h = int(w * height / float(width))
			display = Display(out_file, (w, h))
			targets.append(liar.output.FilterMitchell(display))

	if remote:
		targets.append(liar.output.Socket(*remote))

	if len(targets) == 1:
		return targets[0]
	return liar.output.Splitter(targets)

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

def group_scenery(scenery_objects):
	assert len(scenery_objects) > 0
	if len(scenery_objects) == 1:
		return scenery_objects[0]
	else:
		return liar.scenery.AabpTree(scenery_objects)
''' % vars())
	
	def write_world(self, world, render):
		if not world:
			return
		name = world.name
		horiz = None#const_texture
		for slot in world.texture_slots:
			if not slot:
				continue
			texture = orig_texture = self.write_texture(slot.tex)
			if slot.texture_coords == 'ANGMAP':
				texture = "liar.textures.AngularMapping(%s)" % texture
			if slot.use_map_horizon:
				horiz = texture
		if not horiz:
			return
		shader = None
		if render.alpha_mode == 'SKY':
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
	
	def write_body(self, scene):
			
		for i, obj in enumerate(scene.objects):
			if not _matching_layers(scene.layers, obj.layers):
				continue
			self.__write_object(obj)
			self.draw_progress(float(i+1) / len(scene.objects))
			
		self.write_world(scene.world, scene.render)
		self.write_render(scene.render)
		
	def __write_object(self, obj):
		object_writers = {
			'MESH': LiarExporter.write_mesh,
			'LAMP': LiarExporter.write_lamp,
			'CAMERA': LiarExporter.write_camera
			}
		if obj.name in self.objects:
			return
		self.objects[obj.name] = True
		
		print("- object '{obj.name}' of type '{obj.type}'".format(obj=obj))
		if not obj.type in object_writers:              
			print("WARNING: Did not recognize object type '{obj.type}'".format(obj=obj))
			return

		data = object_writers[obj.type](self, obj, obj.data)
		if not data:
			return

		obj_matrix = matrix_to_list(obj.matrix_world)
		if not is_identity_matrix(obj_matrix):
			self.script_file.write(r'''
# --- object OB:{obj.name} ---
objects['OB:{obj.name}'] = {data}
'''.format(**vars()))
		else:
			self.script_file.write(r'''
# --- object OB:{obj.name} ---
matrix = {obj_matrix}
objects['OB:{obj.name}'] = liar.scenery.Transformation({data}, matrix)
'''.format(**vars()))
	
	def write_footer(self, scene):
		active_camera = scene.camera.data.name
		render = scene.render
		threads = (render.threads_mode == 'FIXED') and render.threads or "liar.RenderEngine.AUTO_NUMBER_OF_THREADS"
		if render.use_border:
			x1, y1, x2, y2 = scene.render.border
			x1, x2 = render.border_min_x, render.border_max_x
			y1, y2 = render.border_min_y, render.border_max_y
			window = ((x1, 1-y2), (x2, 1-y1))
		else:
			window = ((0, 0), (1, 1))
		self.script_file.write('''
engine = liar.RenderEngine()
engine.tracer = liar.tracers.DirectLighting()
engine.sampler = sampler
engine.scene = liar.scenery.AabpTree(list(objects.values()))
engine.camera = cameras['CA:%(active_camera)s']
engine.numberOfThreads = %(threads)s

def render(display=True, remote=None):
	engine.target = make_target(display=display, remote=remote)
	engine.render(%(window)r)

if __name__ == "__main__":
	from optparse import OptionParser
	parser = OptionParser()
	parser.add_option('', '--remote')
	options, args = parser.parse_args()
	if options.remote:
	    ip, port = options.remote.split(':')
	    remote = ip, int(port)
	else:
	    remote = None
	render(display=True, remote=remote)
''' % vars())

	def draw_progress(self, progress):
		liar_log("Exporting scene to LiAR [%3d%%]" % int(100 * progress))

	def export(self, scene=None, script_path=None):
		if not scene:
			scene = bpy.data.scenes[0]
		if not script_path:
			script_path = (os.path.splitext(bpy.data.filepath)[0] or "untitled") + "_liar.py"
		script_path = ef.util.filesystem_path(script_path)
		if not os.path.abspath(script_path):
			script_path = ef.util.filesystem_path(os.path.join(bpy.app.tempdir, script_path))
		self.script_path = script_path
		self.archive_path = os.path.splitext(script_path)[0]

		self.objects = {}
		self.lamps = {}
		self.meshes = {}
		self.materials = {}
		self.textures = {}
		self.images = {}

		#if os.path.exists(self.script_path):
		#    if not Blender.Draw.PupMenu("Overwrite File?%t|Yes%x1|No%x0"):
		#        return
			
		print("Exporting to LiAR '%s' ..." % self.script_path)
		self.draw_progress(0)
		
		self.script_file = open(self.script_path, 'w')
		try:
		
			self.write_header()
			self.write_body(scene)
			self.write_footer(scene)
			
		finally:
			self.script_file.close()
			try:
				self.zip_file.close()
			except AttributeError:
				pass

		self.draw_progress(1)
		print("Done ...")
		return self.script_path
