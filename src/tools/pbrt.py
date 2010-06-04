import sys
import os
import math
import warnings
import liar


def parse(path, *args, **kwargs):
	scene = PbrtScene(*args, **kwargs)
	scene.parse(path)
	return scene


class PbrtScene(object):
	def __init__(self, verbose=False, render_immediately=True, display=True):
		self.engine = liar.RenderEngine()
		self.Identity()
		self.__state = _STATE_OPTIONS_BLOCK
		self.__named_coordinate_systems = {}
		self.__render_immediately = True
		self.__verbose, self.__render_immediately, self.__display = verbose, render_immediately, display
		self.Camera()
		self.PixelFilter()
		self.Film()
		self.SurfaceIntegrator()
	
	def Identity(self):
		self.__cur_transform = liar.Transformation3D.identity()
	
	def Translate(self, x, y, z):
		self.__cur_transform = liar.Transformation3D.translation((x, y, z)).concatenate(self.__cur_transform)
	
	def Scale(self, x, y, z):
		self.__cur_transform = liar.Transformation3D.scaler((x, y, z)).concatenate(self.__cur_transform)
	
	def Rotate(self, angle, x, y, z):
		self.__cur_transform = liar.Transformation3D.rotation((x, y, z), math.radians(angle)).concatenate(self.__cur_transform)
	
	def LookAt(self, ex, ey, ez, lx, ly, lz, ux, uy, uz):
		# defining a LHS using RHS operations?  wtf? pretend like nothing happened!
		from liar.tools.geometry import cross_product, normalize, subtract
		eye = ex, ey, ez
		lookat = lx, ly, lz
		up = ux, uy, uz
		dir = normalize(subtract(lookat, eye))
		right = normalize(cross_product(dir, up))
		up = cross_product(right, dir)
		self.__cur_transform = liar.Transformation3D(eye, right, up, dir).inverse().concatenate(self.__cur_transform)
	
	def CoordinateSystem(self, name):
		self.__named_coordinate_systems[name] = self.__cur_transform
	
	def CoordSysTransform(self, name):
		self.__cur_transform = self.__named_coordinate_systems[name]
	
	def Transform(self, *m):
		m = _unwrap(m)
		assert len(m) == 16
		self.__cur_transform = liar.Transformation3D(m)
	
	def ConcatTransform(self, *m):
		m = _unwrap(m)
		assert len(m) == 16
		self.__cur_transform = liar.Transformation3D(m).concatenate(self.__cur_transform)
	
	def ReverseOrientation(self):
		# quick hack
		self.Scale(-1, -1, -1)
	
	
	def WorldBegin(self):
		self.verify_options()
		self.Identity()
		self.CoordinateSystem("world")
		self.__state = _STATE_WORLD_BLOCK
		self.__area_light = None
		self.__pushed_transforms = []
		self.__pushed_states = []
		self.__textures = {}
		self.__auto_textures = {}
		self.__objects = []
		self.Material()
	
	def WorldEnd(self):
		engine, camera = self.engine, self.engine.camera
		width, height = engine.sampler.resolution = self.resolution
		camera.aspectRatio = self.__frameaspectratio or (float(width) / height)
		if camera.aspectRatio > 1:
			camera.fovAngle = 2 * math.atan(math.tan(.5 * math.radians(self.__fov)) * camera.aspectRatio)
		else:
			camera.fovAngle = math.radians(self.__fov)
		engine.target = self.__film
		if self.__display:
			try:
				Display = liar.output.Display
			except AttributeError:
				warnings.warn("no display output possible, unsupported by LiAR installation")
			else:
				w = min(width, 800)
				h = int(w / camera.aspectRatio)
				display = Display("PBRT powered by LiAR!", (w, h))
				engine.target = liar.output.Splitter([engine.target, display])
		if self.__pixelFilter:
			self.__pixelFilter.target, engine.target = engine.target, self.__pixelFilter
		engine.scene = liar.scenery.AabbTree(self.__objects)
		if self.__render_immediately:
			self.render()
	
	
	def TransformBegin(self):
		self.verify_world()
		self.__pushed_transforms.append(self.__cur_transform)
	
	def TransformEnd(self):
		self.verify_world()
		self.__cur_transform = self.__pushed_transforms.pop()
	
	def AttributeBegin(self):
		self.TransformBegin()
		self.__pushed_states.append((self.__textures, self.__material, self.__area_light))
		
	def AttributeEnd(self):
		self.TransformEnd()
		self.__textures, self.__material, self.__area_light = self.__pushed_states.pop()
	
	
	def Camera(self, name="perspective", frameaspectratio=None, screenwindow=None, fov=90, **kwargs):
		self.verify_options()
		self.__frameaspectratio, self.__screenwindow, self.__fov = frameaspectratio, screenwindow, fov
		camera_space = self.__cur_transform.inverse()
		self.__named_coordinate_systems["camera"] = camera_space
		mat = camera_space.matrix
		right, up, direction, position = [mat[k:12:4] for k in range(4)]
		self.engine.camera = getattr(self, "_camera_" + name)(position, direction, up, right, **kwargs)
	
	def _camera_perspective(self, position, direction, up, right, hither=10e-3, yon=10e30, shutteropen=0, shutterclose=1, lensradius=0, focaldistance=10e30):
		from liar.tools.geometry import negate
		camera = liar.cameras.PerspectiveCamera()
		camera.position, camera.sky, camera.direction = position, up, direction
		camera.right, camera.down = negate(right), negate(up) # -right for rhs to lhs, -up for down
		camera.nearLimit, camera.farLimit = hither, yon
		camera.shutterOpenDelta, camera.shutterCloseDelta = shutteropen, shutterclose
		camera.lensRadius, camera.focusDistance = lensradius, focaldistance
		return camera
	
	
	def Sampler(self, name="bestcandidate", **kwargs):
		self.verify_options()
		if name != "stratified":
			warnings.warn("at this point, we only support stratified samplers.")
			name = "stratified"
		self.engine.sampler = getattr(self, "_sampler_" + name)(**kwargs)
	
	def _sampler_stratified(self, jitter=True, xsamples=2, ysamples=2, pixelsamples=0):
		sampler = liar.samplers.Stratifier()
		sampler.jittered = jitter
		sampler.samplesPerPixel = pixelsamples or (xsamples * ysamples)
		return sampler
	
	
	def Film(self, name="image", xresolution=640, yresolution=480, cropwindow=(0,1,0,1), **kwargs):
		self.verify_options()
		self.resolution = (xresolution, yresolution)
		xstart, xend, ystart, yend = cropwindow
		self.cropwindow = (xstart, ystart), (xend, yend)
		self.__film = getattr(self, "_film_" + name)(**kwargs)
	
	def _film_image(self, filename="pbrt.exr", writefrequency=-1, premultiplyalpha=True):
		return liar.output.Image(filename, self.resolution)
	
	
	def PixelFilter(self, name="mitchell", **kwargs):
		self.verify_options()
		self.__pixelFilter = getattr(self, "_pixelfilter_" + name)(**kwargs)
	
	def _pixelfilter_box(self):
		return None
	
	def _pixelfilter_mitchell(self, xwidth=2, ywidth=2, B=1./3, C=None):
		return liar.output.FilterMitchell(None, B)
	
	
	def Shape(self, name, **kwargs):
		self.verify_world()
		shape = getattr(self, "_shape_" + name)(**kwargs)
		shape.shader = self.__material
		if self.__area_light:
			light_name, light_kwargs = self.__area_light
			shape = getattr(self, "_lightsource_" + light_name)(shape=shape, **light_kwargs)
			shape.surface.shader = liar.shaders.Unshaded(liar.textures.Constant(shape.radiance))
		self.__objects.append(self._with_cur_transform(shape))
	
	def _with_cur_transform(self, shape):
		if self.__cur_transform.isIdentity():
			return shape
		if self.__cur_transform.isTranslation():
			matrix = self.__cur_transform.matrix
			return liar.scenery.Translation(shape, matrix[3:12:4])
		return liar.scenery.Transformation(shape, self.__cur_transform)
	
	def _shape_disk(self, height=0, radius=1):
		return liar.scenery.Disk((0, 0, height), (0, 0, 1), radius)
	
	def _shape_sphere(self, radius):
		return liar.scenery.Sphere((0, 0, 0), radius)
	
	def _shape_trianglemesh(self, indices, P, N=None, uv=None):
		def split_as_tuples(xs, n):
			assert len(xs) % n == 0
			return [tuple(xs[k:k+n]) for k in range(0, len(xs), n)]

		if N and uv:
			as_vertex_index = lambda i: (i, i, i)
		elif N:
			as_vertex_index = lambda i: (i, i)
		elif uv:
			as_vertex_index = lambda i: (i, None, i)
		else:
			as_vertex_index = lambda i: (i, )

		verts = split_as_tuples(P, 3)
		normals = split_as_tuples(N or [], 3)
		uvs = split_as_tuples(uv or [], 2)
		triangles = split_as_tuples([as_vertex_index(i) for i in indices], 3)
		return liar.scenery.TriangleMesh(verts, normals, uvs, triangles)
	
	def _shape_loopsubdiv(self, nlevels=3, *args, **kwargs):
		mesh = self._shape_trianglemesh(*args, **kwargs)
		mesh.loopSubdivision(nlevels)
		mesh.smoothNormals()
		return mesh
	
	
	def Material(self, name="matte", bumpmap=None, **kwargs):
		self.verify_world()
		self.__material = getattr(self, "_material_" + name)(**kwargs)
		if bumpmap:
			self.__material = liar.shaders.BumpMapping(material, self._get_texture(bumpmap))
	
	def _material_matte(self, Kd=1, sigma=0):
		return liar.shaders.Lambert(self._get_texture(Kd))
	
	def _material_plastic(self, Kd=1, Ks=1, roughness=0.1):
		diffuse = liar.shaders.Lambert(self._get_texture(Kd))
		return liar.shaders.Sum([diffuse])
	
	
	def Texture(self, name, type, class_, mapping="uv", uscale=1, vscale=1, udelta=0, vdelta=0, v1=(1,0,0), v2=(0,1,0), **kwargs):
		self.verify_world()
		tex = getattr(self, "_texture_" + class_)(**kwargs)
		if mapping != "uv":
			warnings.warn("at this point, we don't support mappings other than uv")
		if (uscale, vscale, udelta, vdelta) != (1, 1, 0, 0):
			transform = liar.Transformation2D([uscale, 0, udelta, 0, vscale, vdelta, 0, 0, 1])
			if isinstance(tex, liar.textures.TransformationUv):
				tex.transformation = transform.concatenate(tex.transformation)
			else:
				tex = liar.textures.TransformationUv(tex, transform)
		self.__textures[name] = tex
	
	def _texture_constant(self, value):
		return liar.textures.Constant(liar.rgb(value))
	
	def _texture_scale(self, tex1=1, tex2=1):
		tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
		return liar.textures.Product([tex1, tex2])
	
	def _texture_mix(self, tex1=0, tex2=1, amount=0.5):
		tex1, tex2, amount = self._get_texture(tex1), self._get_texture(tex2), self._get_texture(amount)
		return liar.textures.LinearInterpolator([(0, tex1), (1, tex2)], amount)
	
	def _texture_checkerboard(self, tex1=1, tex2=0, dimension=2, aamode="closedform"):
		tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
		assert dimension in (2, 3)
		if dimension == 3:
			tex = liar.textures.CheckerVolume(tex1, tex2)
		else:
			print liar.Transformation2D.scaler(.5)
			tex = liar.textures.CheckerBoard(tex1, tex2)
			assert aamode in ("none", "closedform", "supersample")
			tex.antiAliasing = ("none", "bilinear")[aamode != "none"]
			tex = liar.textures.TransformationUv(tex, liar.Transformation2D.scaler(.5))
		return tex
	
	def _texture_imagemap(self, filename, wrap="repeat", maxanisotropy=8, trilinear=False):
		return liar.textures.Image(filename)
	
	def _get_texture(self, arg):
		if _is_string(arg):
			return self.__textures[arg]
		try:
			return self.__auto_textures[arg]
		except KeyError:
			try:
				x, y, z = arg
			except TypeError:
				tex = liar.textures.Constant(arg)
			else:
				tex = liar.textures.Constant(liar.rgb(x, y, z))
			self.__auto_textures[arg] = tex
			return tex
	
	
	def LightSource(self, name, **kwargs):
		self.verify_world()
		light = getattr(self, "_lightsource_" + name)(**kwargs)
		self.__objects.append(self._with_cur_transform(light))
	
	def AreaLightSource(self, name, **kwargs):
		self.verify_world()
		self.__area_light = name, kwargs
	
	def _lightsource_area(self, shape, L=(1, 1, 1), nsamples=1):
		self.verify_world()
		light = liar.scenery.LightArea(shape)
		light.radiance = L
		light.numberOfEmissionSamples = nsamples
		return light
	
	def _lightsource_distant(self, from_=(0, 0, 0), to=(0, 0, 1), L=(1, 1, 1)):
		from liar.tools import geometry
		direction = geometry.subtract(to, from_)
		return liar.scenery.LightDirectional(direction, L)
	
	def _lightsource_point(self, from_=(0, 0, 0), I=(1, 1, 1)):
		return liar.scenery.LightPoint(from_, I)
	
	def SurfaceIntegrator(self, name="directlighting", **kwargs):
		self.verify_options()
		self.engine.tracer = getattr(self, "_surfaceintegrator_" + name)(**kwargs)
	
	def _surfaceintegrator_directlighting(self, maxdepth=5, strategy="all"):
		tracer = liar.tracers.DirectLighting()
		tracer.maxRayGeneration = maxdepth
		return tracer
	
	def _surfaceintegrator_photonmapper(self, causticphotons=20000, indirectphotons=100000, directphotons=100000, nused=50, maxdepth=5, maxdist=.1, finalgather=True, finalgathersamples=32, directwithphotons=False):
		tracer = liar.tracers.PhotonMapper()
		tracer.globalMapSize = indirectphotons
		if finalgather or directwithphotons:
			tracer.globalMapSize += directphotons
		for k in ("global", "caustic", "volume"):
			tracer.setEstimationSize(k, nused)
			tracer.setEstimationRadius(k, maxdist)
		tracer.numFinalGatherRays = finalgather and finalgathersamples
		tracer.isRayTracingDirect = not directwithphotons
		return tracer
		
	def verify_options(self):
		assert self.__state == _STATE_OPTIONS_BLOCK

	def verify_world(self):
		assert self.__state == _STATE_WORLD_BLOCK
	
	
	def parse(self, path):
		if path == '-':
			return self.parse_stream('stdin', sys.stdin)
		dirname, fname = os.path.split(path)
		oldcwd = os.getcwd()
		if dirname:
			os.chdir(dirname)
		try:
			return self.parse_stream(path, open(fname))
		finally:
			os.chdir(oldcwd)
	
	def parse_stream(self, path, stream):
		import keyword
		include_next = False
		identifier = None
		args = []
		kwargs = {}
		key = None
		tokens = _scanner(path, stream)
		for token_type, token, line_number in tokens:
			if token_type == _IDENTIFIER:
				if identifier:
					if self.__verbose:
						print (identifier, args, kwargs)
					getattr(self, identifier)(*args, **kwargs)
					identifier = None
					args = []
					kwargs = {}
				if token == "Include":
					token_type, token, line_number = tokens.next()
					assert token_type == _STRING, "syntax error in file %(path)r, line %(line_number)d: Include must be followed by a string" % vars()
					self.parse(token)
				else:
					identifier = token
					if keyword.iskeyword(identifier):
						identifier += '_'
			elif token_type == _PARAMETER:
				#assert not token in kwargs
				key = token
				if keyword.iskeyword(key):
					key += '_'
			else:
				if token_type == _START_LIST:
					arg = []
					for token_type, token, line_number in tokens:
						if token_type == _END_LIST:
							break
						assert token_type in (_NUMBER, _STRING), "syntax error in file %(path)r, line %(line_number)d: parameter lists should only contain numbers and strings" % vars()
						arg.append(token)
					arg = _unwrap(tuple(arg))
				else:
					arg = token
				if key:
					kwargs[key] = arg
					key = None
				else:
					args.append(arg)
		if identifier:
			if self.__verbose:
				print (identifier, args, kwargs)
			getattr(self, identifier)(*args, **kwargs)
	
	def render(self):
		self.engine.render(self.cropwindow)


_STATE_UNINITIALIZED, _STATE_OPTIONS_BLOCK, _STATE_WORLD_BLOCK = range(3)
_IDENTIFIER, _NUMBER, _STRING, _PARAMETER, _START_LIST, _END_LIST = range(6)

def _scanner(path, stream):
	from re import Scanner
	scanner = Scanner([
		(r"[a-zA-Z_]\w*", lambda scanner, token: (_IDENTIFIER, token)),
		(r"[-+]?(\d+\.\d*|\.\d+)([eE][-+]?[0-9]+)?", lambda scanner, token: (_NUMBER, float(token))),
		(r"[-+]?\d+", lambda scanner, token: (_NUMBER, int(token))),
		(r'\[', lambda scanner, token: (_START_LIST, token)),
		(r'\]', lambda scanner, token: (_END_LIST, token)),
		(r'"(integer|float|point|vector|normal|color|bool|string|texture)\s+[a-zA-Z_][a-zA-Z0-9_]*"', lambda scanner, token: (_PARAMETER, token[1:-1].split()[1])),
		(r'"true"', lambda scanner, token: (_NUMBER, True)),
		(r'"false"', lambda scanner, token: (_NUMBER, False)),
		(r'".*?"', lambda scanner, token: (_STRING, token[1:-1])),
		(r'#.*$', None), # comments
		(r'\s+', None), # whitespace
		])
	for line_number, line in enumerate(stream, start=1):
		tokens, rest = scanner.scan(line.rstrip('\n'))
		assert not rest, "syntax error in file %(path)r, line %(line_number)d: %(rest)r" % vars()
		for token_type, token in tokens:
			yield token_type, token, line_number


def _unwrap(arg):
	'''
	_unwrap( (x,) ) -> x
	_unwrap( x ) -> x
	
	if arg is a sequence of one item, return that item, otherwise return arg.
	'''
	try:
		x, = arg
	except ValueError:
		x = arg
	return x


try:
	_string_type = basestring
except NameError:
	_string_type = str

_is_string = lambda arg: isinstance(arg, _string_type)


if __name__ == "__main__":
	# use the module as a commandline script
	from optparse import OptionParser
	parser = OptionParser()
	parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True)
	parser.add_option("-n", "--no-display", action="store_false", dest="display", default=True)
	options, args = parser.parse_args()
	for path in args:
		parse(path, verbose=options.verbose, render_immediately=True, display=options.display)
