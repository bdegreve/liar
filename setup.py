#! /usr/bin/env python

import os
import os.path
import copy

from distutils import log, sysconfig
from distutils.core import setup, Extension, Command
from distutils.command.build_clib import build_clib
from distutils.command.build_ext import build_ext
from types import *

lib_lass = "Lass", "lass-%s", ["1.0"]
lib_pixeltoaster = "PixelToaster", "PixelToaster-%s", ["1.%s" % i for i in range(4, 10)]

ignore_files = {
	"shaders": ["simple.cpp"],
	"tracers": ["direct_lighting.cpp"]
	}

try:
	from string import Template
except:
	class Template:
		def __init__(self, template):
			self.template = template
		def substitute(self, dictionary):
			result = self.template
			for key in dictionary:
				result = result.replace('${%s}' % key, str(dictionary[key]))
			return result

def list_files(module, extension):
	dirname = os.path.join("src", module)
	ignores = ignore_files.get(module, [])
	file_filter = lambda f: os.path.splitext(f)[1] == extension and not f in ignores
	files = filter(file_filter, os.listdir(dirname))
	return [os.path.join(dirname, f) for f in files]
			
'''
http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/464406
'''
class pkgconfig(dict):
	import os.path
	_paths = ['~/lib/pkgconfig', '/usr/lib/pkgconfig', '/usr/local/lib/pkgconfig']
	def __init__(self, package):
		self._load(package)
	def _load(self, package):
		for path in self._paths:
			fn = os.path.join(os.path.expanduser(path), '%s.pc' % package)
			if os.path.exists(fn):
				self._parse(fn)
				return
	def _parse(self, filename):
		lines = open(filename).readlines()
		lokals = {}
		for line in lines:
			line = line.strip()
			if not line:
				continue
			elif ':' in line: # exported variable
				name, val = line.split(':', 1)
				val = val.strip()
				if '$' in val:
					try:
						val = Template(val).substitute(lokals)
					except ValueError:
						raise ValueError("Error in variable substitution!")
				self[name] = val
			elif '=' in line: # local variable
				name, val = line.split('=', 1)
				if '$' in val:
					try:
						val = Template(val).substitute(lokals)
					except ValueError:
						raise ValueError("Error in variable substitution!")
				lokals[name] = val

def extract_fields(config, key, prefix):
	result = []
	for field in config[key].split():
		if field.startswith('-' + prefix):
			result.append(field[(1+len(prefix)):])
	return result
	
def check_for_library(library):
	name, mask, versions = library
	print "checking for %s..." % name,
	for version in versions[::-1]: # reversed order =)
		config = pkgconfig(mask % version)
		if config:
			print version
			return config
	print "No"
	return None
	


def get_config_nt():
	root = os.getenv('LASSROOT')
	while not root:
		root = raw_input(r"environment variable LASSROOT is not specified, provide it manually " +
			r"(if 'lass.h' is in 'C:\Lass\lass\lass.h', then specify 'C:\Lass'): ")
	root = root.replace('/', '\\')
	if not root.endswith('\\'):
		root = root + '\\'
	have_pixeltoaster_h = 1 # assume we have it
	windir = os.getenv('windir')
	windir = windir.replace('\\','\\\\')
	system = windir + '\\\\system32\\\\'
	config = {
		'include_dirs': [root],
		'library_dirs': [os.path.join(root, 'lib')],
		'libraries': [
			'd3d9', 
			'user32', 
			'gdi32'],
		#'lass_%s_win32_vc71' % p for p in ('io', 'num', 'prim', 'util')],
		'extra_compile_args': [
			'/GR', 
			'/DHAVE_PIXELTOASTER_H=%s' % have_pixeltoaster_h],
		'extra_link_args': [],
		'data_files': [('lib/site-packages',[system+'msvcr71.dll',system+'msvcp71.dll'])]
	}
	return config

def get_config_posix():
	pc_lass = check_for_library(lib_lass)
	if not pc_lass:
		raise DistutilsSetupError, "Failed to locate Lass library, please install."
		
	pc_pixeltoaster = check_for_library(lib_pixeltoaster)
	if pc_pixeltoaster:
		have_pixeltoaster_h = 1
	else:
		have_pixeltoaster_h = 0
	
	library_dirs = extract_fields(pc_lass, 'Libs', 'L') + extract_fields(pc_pixeltoaster, 'Libs', 'L')
	config = {
		'include_dirs': extract_fields(pc_lass, 'Cflags', 'I') + 
			extract_fields(pc_pixeltoaster, 'Cflags', 'I'),
		'library_dirs': library_dirs,
		'libraries': extract_fields(pc_lass, 'Libs', 'l') + 
			extract_fields(pc_pixeltoaster, 'Libs', 'l'),
		'extra_compile_args': [
			'-DHAVE_PIXELTOASTER_H=%s' % have_pixeltoaster_h, 
			'-Wno-comments', 
			'-Wno-unknown-pragmas'],
		'extra_link_args': [
			'-Wl,-no-undefined', 
			'-Wl,-warn-once', 
			'-Wl,-v',
			'-Wl,-rpath,.',
			'-Wl,-E'] +
			['-Wl,-rpath,%s' % libdir for libdir in library_dirs],
		'data_files': []
	}
	return config

def get_config():
	config_getters = {
		'nt': get_config_nt,
		'posix': get_config_posix
	}
	
	try:
		config_getter = config_getters[os.name]
	except:
		raise DistutilsSetupError, \
			("Don't know how to get configuration for the " +
			"LASS library '%s' on OS '%s'" % (liblass, os.name))
	
	config = config_getter()
	return config
	
def make_extension(name, config):
	sources = list_files(os.path.join(name), '.cpp')
	if name == 'kernel':
		sources = filter(lambda x: 'kernel_init.cpp' in x, sources)
	libs = copy.copy(config['libraries'])
	#libs.append('kernel') (get this one through build_clib)
	return  Extension(
		name,
		sources,
		include_dirs = config['include_dirs'], 
		library_dirs = config['library_dirs'], 
		libraries = libs,
		extra_compile_args = config['extra_compile_args'],
		extra_link_args = config['extra_link_args']
		)

def force_no_optimisation(compiler):
	for i in range(4):
		try: compiler.compiler_so.remove("-O%s" % i)
		except:	pass
	compiler.compiler_so.append("-O0")
	
# http://liar.sourceforge.net/blog/2007/01/27/the-bumpy-road-to-the-linux-build/
#		
class liar_build_shared_lib(build_clib):
	def initialize_options(self):
		self.build_lib = None
		self.package = None
		build_clib.initialize_options(self)
		
	def finalize_options(self):
		self.set_undefined_options('build', ('build_lib', 'build_lib'))
		if self.package is None:
			self.package = self.distribution.ext_package or ""
		build_clib.finalize_options(self)
		
	def build_libraries(self, libraries):
		for lib_name, build_info in libraries:
			self.build_library(lib_name, build_info)
	
	def build_library(self, lib_name, build_info):
		#force_no_optimisation(self.compiler)
		print lib_name, build_info
		sources = build_info.get('sources')
		if sources is None or type(sources) not in (ListType, TupleType):
			raise DistutilsSetupError, \
				("in 'libraries' options (library '%s', " +
				 "'sources' must be present and must be " +
				 "a list of source filenames") % lib_name
		sources = list(sources)
		
		log.info("building '%s' library", lib_name)
		
		macros = build_info.get('macros')
		include_dirs = build_info.get('include_dirs')
		extra_args = build_info.get('extra_compile_args')
		
		py_include = sysconfig.get_python_inc()
		plat_py_include = sysconfig.get_python_inc(plat_specific=1)
		for include in [py_include, plat_py_include]:
			if not include in include_dirs:
				include_dirs.append(include)
		
		objects = self.compiler.compile(
			sources,
			output_dir=self.build_temp,
			macros=macros,
			include_dirs=include_dirs,
			extra_postargs=extra_args,
			debug=self.debug)
				
		output_dir = self.build_lib
		if self.package:
			output_dir = os.path.join(output_dir, self.package)
		extra_args = build_info.get('extra_link_args')
		language = build_info.get('language') or self.compiler.detect_language(sources)
		self.compiler.link_shared_lib(
			objects, lib_name,
			output_dir=output_dir,
			libraries=build_info.get('libraries'),
			library_dirs=build_info.get('library_dirs'),
			runtime_library_dirs=None,
			extra_postargs=extra_args,
			export_symbols=0,
			debug=self.debug,
			build_temp=self.build_temp,
			target_lang=language)
			
			
class liar_build_ext(build_ext):
	def initialize_options(self):
		self.package = None
		build_ext.initialize_options(self)
		
	def finalize_options(self):
		if self.package is None:
			self.package = self.distribution.ext_package
		build_ext.finalize_options(self)
		
	def build_extension(self, ext):
		#force_no_optimisation(self.compiler)
		extra_dir = self.build_lib
		if self.package:
			extra_dir = os.path.join(extra_dir, self.package)
		ext.library_dirs.append(extra_dir)
		build_ext.build_extension(self, ext)
# ------------------------------------------------------------------------------------------

config = get_config()

component_names = ('kernel', 'cameras', 'output', 'samplers', 'scenery', 'shaders', 'textures', 'tracers')
component_extensions = [make_extension(name, config) for name in component_names]

libkernel_sources = list_files(os.path.join('kernel'), '.cpp')
libkernel_sources = filter(lambda x: not 'kernel_init.cpp' in x, libkernel_sources)
libkernel = ('kernel', {
	'sources': libkernel_sources,
	'include_dirs': config['include_dirs'], 
	'library_dirs': config['library_dirs'], 
	'libraries': config['libraries'],
	'macros': [('LIAR_KERNEL_BUILD_DLL', 1)],
	'extra_compile_args': config['extra_compile_args'],
	'extra_link_args': config['extra_link_args']})
	
setup(
	name="liar",
	version='0.2',
	description = "LiAR isn't a raytracer",
	author = "Bram de Greve",
	author_email = "bramz@users.sourceforge.net",
	maintainer = "Bram de Greve",
	maintainer_email = "bramz@users.sourceforge.net",
	package_dir = {'liar': 'src'},
	packages = ['liar', 'liar.tools'],
	libraries = [libkernel],
	ext_package = 'liar',
	ext_modules = component_extensions,
	cmdclass = {
		"build_clib": liar_build_shared_lib,
		"build_ext": liar_build_ext
		}
	)
