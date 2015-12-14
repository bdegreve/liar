import sys
import os
import math
import logging
import liar
import liar.tools.spd

VERBOSITIES = QUIET, NORMAL, VERBOSE = range(3)


def parse(path, *args, **kwargs):
	scene = PbrtScene(*args, **kwargs)
	scene.parse(path)
	return scene


class PbrtScene(object):
	def __init__(self, verbosity=NORMAL, render_immediately=True, display=True, threads=None):
		self.__logger = logging.getLogger("liar.tools.pbrt")
		self.__logger.setLevel({QUIET: logging.CRITICAL, NORMAL: logging.INFO, VERBOSE: logging.DEBUG}[verbosity])
		self.engine = liar.RenderEngine()
		self.engine.numberOfThreads = threads or liar.RenderEngine.AUTO_NUMBER_OF_THREADS
		self.Identity()
		self.__state = _STATE_OPTIONS_BLOCK
		self.__named_coordinate_systems = {}
		self.__verbosity = verbosity
		self.__render_immediately = render_immediately
		self.__display = display
		self.Camera()
		self.PixelFilter()
		self.Film()
		self.SurfaceIntegrator()
		self.Sampler()

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
		from liar.tools.geometry import cross_product, normalize, subtract, negate
		eye = ex, ey, ez
		lookat = lx, ly, lz
		up = ux, uy, uz
		dir = normalize(subtract(lookat, eye))
		right = normalize(cross_product(dir, up))
		up = cross_product(right, dir)
		left = negate(right)
		self.__cur_transform = liar.Transformation3D(eye, left, up, dir).inverse().concatenate(self.__cur_transform)

	def CoordinateSystem(self, name):
		self.__named_coordinate_systems[name] = self.__cur_transform

	def CoordSysTransform(self, name):
		self.__cur_transform = self.__named_coordinate_systems[name]

	def Transform(self, *m):
		m = _unwrap(m)
		assert len(m) == 16
		self.__cur_transform = liar.Transformation3D(_transpose(m))

	def ConcatTransform(self, *m):
		m = _unwrap(m)
		assert len(m) == 16
		self.__cur_transform = liar.Transformation3D(_transpose(m)).concatenate(self.__cur_transform)

	def ReverseOrientation(self):
		# quick hack
		self.Scale(-1, -1, -1)


	def WorldBegin(self):
		self.verify_options()
		self.Identity()
		self.CoordinateSystem("world")
		self.__state = _STATE_WORLD_BLOCK
		self.__area_light = None
		self.__volume = None
		self.__pushed_transforms = []
		self.__pushed_states = []
		self.__textures = {}
		self.__auto_textures = {}
		self.__objects = []
		self.__instances = {}
		self.__cur_instance = None
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
				self.__logger.warning("no display output possible, unsupported by LiAR installation")
			else:
				w = min(width, 800)
				h = int(w / camera.aspectRatio)
				display = Display("PBRT powered by LiAR!", (w, h))
				engine.target = liar.output.Splitter([engine.target, display])
		if self.__pixelFilter:
			self.__pixelFilter.target, engine.target = engine.target, self.__pixelFilter
		engine.scene = liar.scenery.AabbTree(self.__objects)
		engine.scene.interior = self.__volume
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
		self.__pushed_states.append((self.__textures.copy(), self.__material, self.__area_light))

	def AttributeEnd(self):
		self.TransformEnd()
		self.__textures, self.__material, self.__area_light = self.__pushed_states.pop()

	def ObjectBegin(self, name):
		self.AttributeBegin()
		assert self.__cur_instance is None, "ObjectBegin called inside of instance definition"
		self.__cur_instance = self.__instances[name] = []

	def ObjectEnd(self):
		self.AttributeEnd()
		self.__cur_instance = None

	def ObjectInstance(self, name):
		self.verify_world()
		for shape in self.__instances[name]:
			self.__add_shape(shape)

	def __add_shape(self, shape):
		self.verify_world()
		shape = self.__with_cur_transform(shape)
		if not self.__cur_instance is None:
			self.__cur_instance.append(shape)
		self.__objects.append(shape)

	def __with_cur_transform(self, shape):
		# todo: merge __cur_transform with shape's transform.
		if self.__cur_transform.isIdentity():
			return shape
		if self.__cur_transform.isTranslation():
			matrix = self.__cur_transform.matrix
			return liar.scenery.Translation(shape, matrix[3:12:4])
		return liar.scenery.Transformation(shape, self.__cur_transform)


	def Camera(self, name="perspective", frameaspectratio=None, screenwindow=None, fov=90, **kwargs):
		self.verify_options()
		self.__frameaspectratio, self.__screenwindow, self.__fov = frameaspectratio, screenwindow, fov
		camera_space = self.__cur_transform.inverse()
		self.__named_coordinate_systems["camera"] = camera_space
		mat = camera_space.matrix
		right, up, direction, position = [mat[k:12:4] for k in range(4)]
		self.engine.camera = getattr(self, "_camera_" + name)(position, direction, up, right, **kwargs)

	def _camera_perspective(self, position, direction, up, right, hither=10e-3, yon=10e30, shutteropen=0, shutterclose=1, lensradius=0, focaldistance=10e30, _falloff=0):
		from liar.tools.geometry import negate
		camera = liar.cameras.PerspectiveCamera()
		camera.position, camera.sky, camera.direction = position, up, direction
		camera.right, camera.down = right, negate(up) # -up for down
		camera.nearLimit, camera.farLimit = hither, yon
		camera.shutterOpenDelta, camera.shutterCloseDelta = shutteropen, shutterclose
		camera.lensRadius, camera.focusDistance = lensradius, focaldistance
		camera.falloffPower = _falloff
		return camera


	def Sampler(self, name="bestcandidate", **kwargs):
		self.verify_options()
		if name != "stratified":
			self.__logger.warning("at this point, we only support stratified samplers.")
			name = "stratified"
		self.engine.sampler = getattr(self, "_sampler_" + name)(**kwargs)

	def _sampler_stratified(self, jitter=True, xsamples=2, ysamples=2, pixelsamples=None):
		if pixelsamples:
			sampler = liar.samplers.LatinHypercube()
			sampler.samplesPerPixel = pixelsamples
		else:
			sampler = liar.samplers.Stratifier()
			sampler.samplesPerPixel = xsamples * ysamples
		sampler.jittered = jitter
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
		self.__add_shape(shape)

	def _shape_disk(self, height=0, radius=1):
		return liar.scenery.Disk((0, 0, height), (0, 0, 1), radius)

	def _shape_sphere(self, radius=1):
		return liar.scenery.Sphere((0, 0, 0), radius)

	def _shape_trianglemesh(self, indices, P, N=None, uv=None, st=None):
		def split_as_tuples(xs, n):
			assert len(xs) % n == 0
			return [tuple(xs[k:k+n]) for k in range(0, len(xs), n)]

		uv = uv or st
		if N and uv:
			as_vertex_index = lambda i: (i, i, i)
		elif N:
			as_vertex_index = lambda i: (i, i)
		elif uv:
			as_vertex_index = lambda i: (i, None, i)
		else:
			as_vertex_index = lambda i: (i, )

		uv = split_as_tuples(uv or [], 2)
		triangles = split_as_tuples([as_vertex_index(i) for i in indices], 3)
		return liar.scenery.TriangleMesh(P, N or [], uv or [], triangles)

	def _shape_loopsubdiv(self, nlevels=3, *args, **kwargs):
		mesh = self._shape_trianglemesh(*args, **kwargs)
		mesh.loopSubdivision(nlevels)
		mesh.smoothNormals()
		return mesh


	def Accelerator(self, *args, **kwargs):
		self.verify_options()
		# we ignore this


	def Material(self, name="matte", bumpmap=None, **kwargs):
		self.verify_world()
		try:
			self.__material = getattr(self, "_material_" + name)(**kwargs)
		except AttributeError:
			self.__logger.warning("unknown material %(name)r, defaulting to matte." % vars())
			kwargs = _filter_dict(kwargs, lambda key: key in ('Kd', 'sigma'))
			self.__material = self._material_matte(**kwargs)
		if bumpmap:
			self.__material = liar.shaders.BumpMapping(self.__material, self._get_texture(bumpmap))

	def _material_bluepaint(self):
		diffuse = self._get_texture((0.3094, 0.39667, 0.70837))
		mat = liar.shaders.Lafortune(diffuse)
		xy = self._get_texture((0.870567, 0.857255, 0.670982))
		z = self._get_texture((0.803624,  0.774290,  0.586674))
		e = self._get_texture((21.820103, 18.597755, 7.472717))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-0.451218, -0.406681, -0.477976))
		z = self._get_texture((0.023123, 0.017625, 0.227295))
		e = self._get_texture((2.774499, 2.581499, 3.677653))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-1.031545, -1.029426, -1.026588))
		z = self._get_texture((0.706734,  0.696530,  0.687715))
		e = self._get_texture((66.899060, 63.767912, 57.489181))
		mat.addLobe(xy, xy, z, e)
		return mat

	def _material_clay(self):
		diffuse = self._get_texture((0.383626, 0.260749, 0.274207))
		mat = liar.shaders.Lafortune(diffuse)
		xy = self._get_texture((-1.089701, -1.102701, -1.107603))
		z = self._get_texture((-1.354682, -2.714801, -1.569866))
		e = self._get_texture((17.968505, 11.024489, 21.270282))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-0.733381,  -0.793320, -0.848206))
		z = self._get_texture((0.676108, 0.679314, 0.726031))
		e = self._get_texture((8.219745, 9.055139, 11.261951))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-1.010548, -1.012378, -1.011263))
		z = self._get_texture((0.910783, 0.885239, 0.892451))
		e = self._get_texture((152.912795, 141.937171, 201.046802))
		mat.addLobe(xy, xy, z, e)
		return mat

	def _material_felt(self):
		diffuse = self._get_texture((0.025865,  0.025865,  0.025865))
		mat = liar.shaders.Lafortune(diffuse)
		xy = self._get_texture((-0.304075, -0.304075, -0.304075))
		z = self._get_texture((-0.065992, -0.065992, -0.065992))
		e = self._get_texture((3.047892, 3.047892, 3.047892))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-0.749561, -0.749561, -0.749561))
		z = self._get_texture((-1.167929, -1.167929, -1.167929))
		e = self._get_texture((6.931827, 6.931827, 6.931827))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((1.004921, 1.004921, 1.004921))
		z = self._get_texture((-0.205529, -0.205529, -0.205529))
		e = self._get_texture((94.117332, 94.117332, 94.117332))
		mat.addLobe(xy, xy, z, e)
		return mat

	def _material_glass(self, Kr=1, Kt=1, index=1.5):
		glass = liar.shaders.Dielectric(self._get_texture(index))
		glass.reflectance = self._get_texture(Kr)
		glass.transmittance = self._get_texture(Kt)
		return glass

	def _material_matte(self, Kd=1, sigma=0):
		return liar.shaders.Lambert(self._get_texture(Kd))

	def _material_metal(self, eta, k, roughness=.01):
		return liar.shaders.Conductor(self._get_texture(eta), self._get_texture(k))

	def _material_mirror(self, Kr=1):
		return liar.shaders.Mirror(self._get_texture(Kr))

	def _material_plastic(self, Kd=1, Ks=1, roughness=0.1):
		#diffuse = liar.shaders.Lambert(self._get_texture(Kd))
		#return liar.shaders.Sum([diffuse])
		return self._material_substrate(Kd=Kd, Ks=Ks, uroughness=roughness, vroughness=roughness)

	def _material_primer(self):
		diffuse = self._get_texture((0.118230, 0.121218, 0.133209))
		mat = liar.shaders.Lafortune(diffuse)
		xy = self._get_texture((-0.399286, -1.033473, -1.058104))
		z = self._get_texture((0.167504, 0.009545, -0.068002))
		e = self._get_texture((2.466633, 7.637253,  8.117645))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-1.041861, -1.100108, -1.087779))
		z = self._get_texture((0.014375, -0.198147, -0.053605))
		e = self._get_texture((7.993722, 29.446268, 41.988990))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-1.098605, -0.379883, -0.449038))
		z = self._get_texture((-0.145110, 0.159127, 0.173224))
		e = self._get_texture((31.899719, 2.372852, 2.636161))
		mat.addLobe(xy, xy, z, e)
		return mat

	def _material_skin(self):
		diffuse = self._get_texture((0.428425, 0.301341, 0.331054))
		mat = liar.shaders.Lafortune(diffuse)
		xy = self._get_texture((-1.131747, -1.016939, -0.966018))
		z = self._get_texture((-1.209182, -1.462488, -1.222419))
		e = self._get_texture((6.421658,  3.699932,  3.524889))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-0.546570, -0.643533, -0.638934))
		z = self._get_texture((0.380123,  0.410559,  0.437367))
		e = self._get_texture((3.685044,  4.266495,  4.539742))
		mat.addLobe(xy, xy, z, e)
		xy = self._get_texture((-0.998888, -1.020153, -1.027479))
		z = self._get_texture((0.857998,  0.703913,  0.573625))
		e = self._get_texture((64.208486, 63.919687, 43.809866))
		mat.addLobe(xy, xy, z, e)
		return mat

	def _material_substrate(self, Kd=.5, Ks=.5, uroughness=.1, vroughness=.1):
		from liar.textures import Division, Max
		shader = liar.shaders.AshikhminShirley(self._get_texture(Kd), self._get_texture(Ks))
		one = self._get_texture(1)
		eps = self._get_texture(1e-6)
		shader.specularPowerU = Division(one, Max(self._get_texture(uroughness), eps))
		shader.specularPowerV = Division(one, Max(self._get_texture(vroughness), eps))
		return shader

	def _material_uber(self, Kd=1, Ks=1, Kr=0, roughness=0.1, opacity=1):
		layers = []
		if Kd or Ks:
			if Ks:
				layers.append(self._material_substrate(Kd=Kd, Ks=Ks, uroughness=roughness, vroughness=roughness))
			else:
				layers.append(self._material_matte(Kd=Kd))
		if Kr:
			layers.append(self._material_mirror(Kr=Kr))
		if len(layers) == 1:
			mat = layers[0]
		else:
			mat = liar.shaders.Sum(layers)
		if opacity != 1:
			transparent = liar.shaders.Flip(liar.shaders.Mirror(self._get_texture(1)))
			mat = liar.shaders.LinearInterpolator([(0, transparent), (1, mat)], self._get_texture(opacity))
		return mat


	def Texture(self, name, type, class_, mapping="uv", uscale=1, vscale=1, udelta=0, vdelta=0, v1=(1,0,0), v2=(0,1,0), **kwargs):
		self.verify_world()
		tex = getattr(self, "_texture_" + class_)(**kwargs)
		if mapping != "uv":
			self.__logger.warning("at this point, we don't support mappings other than uv")
		if (uscale, vscale, udelta, vdelta) != (1, 1, 0, 0):
			transform = liar.Transformation2D([uscale, 0, udelta, 0, vscale, vdelta, 0, 0, 1])
			if isinstance(tex, liar.textures.TransformationUv):
				tex.transformation = transform.concatenate(tex.transformation)
			else:
				tex = liar.textures.TransformationUv(tex, transform)
		self.__textures[name] = tex

	def _texture_checkerboard(self, tex1=1, tex2=0, dimension=2, aamode="closedform"):
		tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
		assert dimension in (2, 3)
		if dimension == 3:
			tex = liar.textures.CheckerVolume(tex1, tex2)
		else:
			tex = liar.textures.CheckerBoard(tex1, tex2)
			assert aamode in ("none", "closedform", "supersample"), aamode
			tex.antiAliasing = ("none", "bilinear")[aamode != "none"]
			tex = liar.textures.TransformationUv(tex, liar.Transformation2D.scaler(.5))
		return tex

	def _texture_constant(self, value):
		return self._get_texture(value)

	def _texture_fbm(self, octaves=8, roughness=.5):
		return liar.textures.FBm(octaves, roughness)

	def _texture_imagemap(self, filename, wrap="repeat", maxanisotropy=8, trilinear=False, gamma=1):
		return liar.textures.Image(filename, _RGB_SPACE.withGamma(gamma))

	def _texture_mix(self, tex1=0, tex2=1, amount=0.5):
		tex1, tex2, amount = self._get_texture(tex1), self._get_texture(tex2), self._get_texture(amount)
		return liar.textures.LinearInterpolator([(0, tex1), (1, tex2)], amount)

	def _texture_scale(self, tex1=1, tex2=1):
		tex1, tex2 = self._get_texture(tex1), self._get_texture(tex2)
		return liar.textures.Product([tex1, tex2])

	def _texture_windy(self):
		from liar.textures import FBm, TransformationLocal, Abs, Product
		waveHeight = FBm(6)
		windStrength = Abs(TransformationLocal(FBm(3), liar.Transformation3D.scaler(.1)))
		return Product(waveHeight, windStrength)

	def _get_texture(self, arg):
		# arg is already a texture
		if isinstance(arg, liar.Texture):
			return arg
		# arg as a texture name
		try:
			return self.__textures[arg]
		except (KeyError, TypeError):
			pass
		# arg as a cached texture
		try:
			return self.__auto_textures[arg]
		except (KeyError, TypeError):
			pass
		tex = None
		# arg as a path to an image file
		try:
			if os.path.isfile(arg):
				tex = liar.textures.Image(arg, _RGB_SPACE)
		except TypeError:
			pass
		if not tex:
			# an RGB triple
			try:
				r, g, b = arg
			except TypeError:
				pass
			else:
				try:
					tex = liar.textures.Constant(_color(r, g, b))
				except TypeError:
					pass
		if not tex:
			# a gray value
			try:
				tex = liar.textures.Constant(arg)
			except TypeError:
				pass
		if not tex:
			raise ValueError("%(arg)r is nor a registered texture name, nor a imagemap path, nor an RGB triple, nor a single float" % vars())
		try:
			self.__auto_textures[arg] = tex
		except TypeError:
			pass
		return tex


	def Volume(self, name, p0=(0,0,0), p1=(1,1,1), **kwargs):
		self.verify_world()
		shader = getattr(self, "_volume_" + name)(**kwargs)
		try:
			shader.origin = p0
		except AttributeError:
			pass
		self.__volume = liar.mediums.Bounded(shader, (p0, p1))
		if not self.__cur_transform.isIdentity():
			self.__volume = liar.mediums.Transformation(self.__volume, self.__cur_transform)

	def _volume_homogeneous(self, sigma_a=0, sigma_s=0, g=0, Le=0):
		# let's start off with a simple one ...
		sigma_e = (_avg(sigma_a) + _avg(sigma_s))
		fog = liar.mediums.Fog(sigma_e, g)
		fog.emission = _color(Le)
		fog.color = _color(_mul(sigma_s, 0)) #_mul(sigma_s, 1 / sigma_e)
		return fog

	def _volume_exponential(self, sigma_a=0, sigma_s=0, g=0, Le=0, a=1, b=1, updir=(0,1,0)):
		# let's start off with a simple one ...
		sigma_e = (_avg(sigma_a) + _avg(sigma_s))
		fog = liar.mediums.ExponentialFog(a * sigma_e, g)
		fog.emission = _mul(Le, a)
		fog.color = _mul(sigma_s, 0) #_mul(sigma_s, 1 / sigma_e)
		fog.decay = b
		fog.up = updir
		return fog


	def LightSource(self, name, **kwargs):
		self.verify_world()
		light = getattr(self, "_lightsource_" + name)(**kwargs)
		self.__objects.append(self.__with_cur_transform(light))


	def AreaLightSource(self, name, **kwargs):
		self.verify_world()
		self.__area_light = name, kwargs

	def _lightsource_area(self, shape, L=(1, 1, 1), nsamples=1):
		self.verify_world()
		light = liar.scenery.LightArea(shape)
		light.radiance = _color(L)
		light.numberOfEmissionSamples = nsamples
		return light

	def _lightsource_distant(self, from_=(0, 0, 0), to=(0, 0, 1), L=(1, 1, 1)):
		from liar.tools import geometry
		direction = geometry.subtract(to, from_)
		return liar.scenery.LightDirectional(direction, _color(L))

	def _lightsource_infinite(self, L=(1, 1, 1), nsamples=1, mapname=None):
		tex, res = self._get_light_texture(L, mapname)
		flipAndShiftU = liar.Transformation2D([-1, 0, -.25, 0, 1, 0, 0, 0, 1])
		light = liar.scenery.LightSky(liar.textures.TransformationUv(tex, flipAndShiftU))
		if res:
			light.samplingResolution = res
		light.numberOfEmissionSamples = nsamples
		light.shader = liar.shaders.Unshaded(light.radiance)
		return light

	def _lightsource_point(self, from_=(0, 0, 0), I=(1, 1, 1)):
		return liar.scenery.LightPoint(from_, _color(I))

	def _lightsource_projection(self, I=(1, 1, 1), fov=45, mapname=None):
		intensity, resolution = self._get_light_texture(I, mapname)
		light = liar.scenery.LightProjection()
		light.intensity = intensity
		light.projection.sky, light.projection.direction = (0, 1, 0), (0, 0, 1)
		light.projection.right, light.projection.up = (1, 0, 0), (0, -1, 0)
		if resolution:
			light.samplingResolution = resolution
			light.projection.aspectRatio = resolution[0] / resolution[1]
		else:
			light.projection.aspectRatio = 1
		light.projection.fovAngle = math.radians(fov)
		if light.projection.aspectRatio > 1:
			# PBRT's fov is on shortest edge, but liar's fov is always on width
			# fix by scaling focalLength
			light.projection.focalLength /= light.projection.aspectRatio
		return light

	def _lightsource_spot(self, I=(1,1,1), from_=(0, 0, 0), to=(0, 0, 1), coneangle=30, conedeltaangle=5):
		light = liar.scenery.LightSpot()
		light.position = from_
		light.lookAt(to)
		light.intensity = _color(I)
		light.outerAngle = math.radians(coneangle)
		light.innerAngle = math.radians(conedeltaangle)
		return light

	def _get_light_texture(self, factor, mapname):
		if not mapname:
			return self._get_texture(multiplier)
		tex = self._get_texture(mapname)
		try:
			res = tex.resolution
		except AttributeError:
			res = None
		try:
			tex.mipMapping = 'none'
		except AttributeError:
			pass
		if not (factor == (1, 1, 1) or factor == 1):
			tex = liar.textures.Product(self._get_texture(factor), tex)
		return tex, res


	def SurfaceIntegrator(self, name="directlighting", **kwargs):
		self.verify_options()
		self.engine.tracer = getattr(self, "_surfaceintegrator_" + name)(**kwargs)

	def _surfaceintegrator_directlighting(self, maxdepth=5, strategy="all"):
		tracer = liar.tracers.DirectLighting()
		tracer.maxRayGeneration = maxdepth
		return tracer

	def _surfaceintegrator_photonmap(self, causticphotons=20000, indirectphotons=100000, directphotons=100000, nused=50, maxdepth=5, maxdist=.1, finalgather=True, finalgathersamples=32, directwithphotons=False):
		tracer = liar.tracers.PhotonMapper()
		tracer.globalMapSize = indirectphotons
		tracer.causticsQuality = (100. * causticphotons) / indirectphotons
		if self.__verbosity:
			print ("  causticsQuality=%r" % tracer.causticsQuality)
		if finalgather or directwithphotons:
			tracer.globalMapSize += directphotons
		for k in ("global", "caustic", "volume"):
			tracer.setEstimationSize(k, nused)
			#tracer.setEstimationRadius(k, maxdist)
		tracer.numFinalGatherRays = finalgather and finalgathersamples
		tracer.isRayTracingDirect = not directwithphotons
		return tracer


	def VolumeIntegrator(self, *args, **kwargs):
		self.verify_options()
		# we don't do special settings yet


	def Renderer(self, *args, **kwargs):
		self.verify_options()
		# we don't do special settings yet


	def verify_options(self):
		assert self.__state == _STATE_OPTIONS_BLOCK

	def verify_world(self):
		assert self.__state == _STATE_WORLD_BLOCK


	def parse(self, path):
		if path == '-':
			return self.parse_stream('stdin', sys.stdin)
		return self.parse_stream(path, open(path))

		dirname, fname = os.path.split(path)
		oldcwd = os.getcwd()
		if dirname:
			pass
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
		arg_type, arg_name = None, None
		tokens = _scanner(path, stream)
		for token_type, token, line_number in tokens:
			if token_type == _IDENTIFIER:
				if identifier:
					self.__print_statement(identifier, args, kwargs)
					getattr(self, identifier)(*args, **kwargs)
					identifier = None
					args = []
					kwargs = {}
				if token == "Include":
					token_type, token, line_number = next(tokens)
					assert token_type == _STRING, "syntax error in file %(path)r, line %(line_number)d: Include must be followed by a string" % vars()
					self.__logger.debug("Include %r" % token)
					self.parse(token)
				else:
					identifier = token
					if keyword.iskeyword(identifier):
						identifier += '_'
			elif token_type == _PARAMETER:
				#assert not token in kwargs
				arg_type, arg_name = token
				if keyword.iskeyword(arg_name):
					arg_name += '_'
			else:
				if token_type == _START_LIST:
					value = []
					for token_type, token, line_number in tokens:
						if token_type == _END_LIST:
							break
						assert token_type in (_NUMBER, _STRING), "syntax error in file %(path)r, line %(line_number)d: parameter lists should only contain numbers and strings" % vars()
						value.append(token)
				else:
					value = [token]
				if arg_type:
					value = getattr(self, '_arg_' + arg_type)(*value)
					arg_type = None
				value = _unwrap(tuple(value))
				if arg_name:
					kwargs[arg_name] = value
					arg_name = None
				else:
					args.append(value)
		if identifier:
			self.__print_statement(identifier, args, kwargs)
			getattr(self, identifier)(*args, **kwargs)


	def _arg_integer(self, *values):
		return map(int, values)

	def _arg_float(self, *values):
		return map(float, values)

	def _arg_point(self, *values):
		return _split_as_tuples(values, 3)

	def _arg_vector(self, *values):
		return _split_as_tuples(values, 3)

	def _arg_normal(self, *values):
		return _split_as_tuples(values, 3)

	def _arg_bool(self, *values):
		return map(bool, values)

	def _arg_string(self, *values):
		return map(str, values)

	def _arg_texture(self, *values):
		return map(self._get_texture, values)

	def _arg_xyz(self, *values):
		return map(liar.Spectrum.XYZ, _split_as_tuples(values, 3))

	def _arg_rgb(self, *values):
		return [liar.rgb(rgb, _RGB_SPACE) for rgb in _split_as_tuples(values, 3)]

	_arg_color = _arg_rgb

	def _arg_blackbody(self, *values):
		assert False, values

	def _arg_spectrum(self, *values):
		if len(values) == 1:
			filename, = values
			assert _is_string(filename), filename
			return liar.tools.spd.load(filename)
		wavelengths, values = zip(*_split_as_tuples(values, 2))
		return [liar.spectra.Sampled(wavelengths, values)]


	def __print_statement(self, identifier, args, kwargs):
		def truncated_repr(x, n=80):
			s = repr(x)
			if len(s) <= n:
				return s
			trunc = " ..."
			if s[-1] in "'\")]}>":
				trunc += s[-1]
			return s[:n-len(trunc)] + trunc
		import pprint
		pretty_args = [truncated_repr(value) for value in args]
		pretty_kwargs = ["%s=%s" % (key, truncated_repr(value)) for key, value in sorted(kwargs.items())]
		self.__logger.debug("%s (%s)" % (identifier, ", ".join(pretty_args + pretty_kwargs)))

	def render(self):
		self.engine.render(self.cropwindow)


_STATE_UNINITIALIZED, _STATE_OPTIONS_BLOCK, _STATE_WORLD_BLOCK = range(3)
_IDENTIFIER, _NUMBER, _STRING, _PARAMETER, _START_LIST, _END_LIST = range(6)

def _scanner(path, stream):
	from re import Scanner
	scanner = Scanner([
		(r"[a-zA-Z_]\w*", lambda _, token: (_IDENTIFIER, token)),
		(r"[\-+]?(\d+\.\d*|\.\d+)([eE][\-+]?[0-9]+)?", lambda _, token: (_NUMBER, float(token))),
		(r"[\-+]?\d+", lambda _, token: (_NUMBER, int(token))),
		(r'\[', lambda _, token: (_START_LIST, token)),
		(r'\]', lambda _, token: (_END_LIST, token)),
		(r'"(integer|float|point|vector|normal|bool|string|texture|color|rgb|xyz|blackbody|spectrum)\s+[a-zA-Z_][a-zA-Z0-9_]*"', lambda _, token: (_PARAMETER, token[1:-1].split())),
		(r'"true"', lambda _, token: (_NUMBER, True)),
		(r'"false"', lambda _, token: (_NUMBER, False)),
		(r'".*?"', lambda _, token: (_STRING, token[1:-1])),
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
	except (TypeError, ValueError):
		x = arg
	return x


def _split_as_tuples(xs, n):
	assert len(xs) % n == 0
	return [tuple(xs[k:k+n]) for k in range(0, len(xs), n)]



def _filter_dict(mapping, predicate):
	return dict((key, value) for key, value in mapping.items() if predicate(key))


def _transpose(m):
	'''
	transpose a 4x4 matrix
	'''
	assert len(m) == 16
	return [m[0], m[4], m[8], m[12],
		m[1], m[5], m[9], m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]]


def _avg(arg):
	try:
		return float(arg)
	except TypeError:
		return float(sum(arg)) / len(arg)


def _mul(arg, factor):
	try:
		return [x * factor for x in arg]
	except TypeError:
		return factor * arg


def _color(*args):
	if len(args) == 1:
		arg = args[0]
		if isinstance(arg, liar.Spectrum):
			return arg
		try:
			return liar.Spectrum.Flat(float(arg))
		except TypeError:
			r, g, b = arg
	else:
		r, g, b = args
	return liar.rgb(r, g, b, _RGB_SPACE)

_RGB_SPACE = liar.sRGB.linearSpace()


try:
	_string_type = basestring
except NameError:
	_string_type = str

_is_string = lambda arg: isinstance(arg, _string_type)


if __name__ == "__main__":
	# use the module as a commandline script
	logging.basicConfig(level=logging.DEBUG, format="%(message)s")
	from optparse import OptionParser
	parser = OptionParser()
	parser.add_option("-q", "--quiet", action="store_const", const=QUIET, dest="verbosity", default=NORMAL)
	parser.add_option("-v", "--verbose", action="store_const", const=VERBOSE, dest="verbosity")
	parser.add_option("-d", "--display", action="store_true", dest="display", default=False, help="show progress in preview display [default=%default]")
	parser.add_option("-t", "--threads", action="store", type="int", help="number of threads for rendering")
	options, args = parser.parse_args()
	for path in args:
		parse(path, render_immediately=True, **options.__dict__)
