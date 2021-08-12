from conans import ConanFile, CMake, tools, errors
import errno
import re
import os
import subprocess

def _get_version():
    try:
        content = tools.load("CMakeLists.txt")
    except EnvironmentError as err:
        if err.errno == errno.ENOENT:
            return None  # Assume conan metadata already knows
        raise
    match = re.search(r"project\(liar VERSION (\d+\.\d+\.\d+)\)", content)
    assert match
    version = match.group(1)

    return version


class LiarConan(ConanFile):
    name = "liar"
    version = _get_version()
    license = "GPL-2.0-or-later"
    author = "Bram de Greve <bram.degreve@bramz.net>"
    topics = "C++", "Python"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_paths"
    exports_sources = ("*", "!build*", "!env*", "!venv*")
    requires = [
        "Lass/1.11.0-32+g752b84d8@cocamware/testing",
    ]
    options = {
        "fPIC": [True, False],
        "have_openexr": [True, False],
        "have_libjpeg": [True, False],
        "have_lcms2": [True, False],
        "spectral_mode": ["RGB", "XYZ", "Banded", "Single"],
    }
    default_options = {
        "fPIC": True,
        "have_openexr": True,
        "have_libjpeg": True,
        "have_lcms2": False,
        "lass:shared": True,
        "openexr:shared": False,
        "libjpeg:shared": False,
        "lcms:shared": False,
        "spectral_mode": "RGB",
    }

    def requirements(self):
        if self.options.have_openexr:
            self.requires("openexr/2.3.0@conan/stable")
        if self.options.have_libjpeg:
            self.requires("libjpeg/9c@bincrafters/stable")
        if self.options.have_lcms2:
            self.requires("lcms/2.9@bincrafters/stable")

    def config_options(self):
        if self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def _cmake(self):
        python = Python(self.options["Lass"].python_executable, self.settings)
        if str(self.options["Lass"].python_version) != python.version:
            raise errors.ConanInvalidConfiguration(
                "python_version option not compatible with python_executable:"
                "{} != {}".format(
                    str(self.options["Lass"].python_version), python.version
                )
            )
        if self.options["Lass"].python_debug != python.debug:
            raise errors.ConanInvalidConfiguration(
                "python_debug option not compatible with python_executable."
            )
        if self.settings.os == "Windows":
            if python.debug and self.settings.build_type != "Debug":
                raise errors.ConanInvalidConfiguration(
                    "Can't build non-debug Lass with debug Python on Windows."
                )

        cmake = CMake(self)
        defs = {
            "CMAKE_CONFIGURATION_TYPES": self.settings.build_type,
            "BUILD_TESTING": self.develop and not self.in_local_cache,
            "SPECTRAL_MODE": self.options.spectral_mode,
            "Lass_DIR": os.path.join(self.deps_cpp_info["Lass"].rootpath, "cmake", "Lass"),
            "Python_EXECUTABLE": python.executable,
            "Python_LIBRARY": python.library,
        }
        if self.options.have_lcms2:
            defs["LCMS2_ENABLE"] = True
        cmake.configure(source_folder=".", defs=defs)
        return cmake

    def build(self):
        cmake = self._cmake()
        cmake.build()

    def package(self):
        cmake = self._cmake()
        cmake.install()

    def package_info(self):
        self.env_info.PYTHONPATH.append(self.package_folder)


class Python(object):
    """Build and config info of a Python"""

    def __init__(self, executable, settings):
        self._executable = tools.which(str(executable))
        self._os = settings.os

        if not self._executable:
            raise errors.ConanInvalidConfiguration(
                "Python executable cannot be found: {!s}".format(self._executable)
            )

    @property
    def executable(self):
        """Full path to Python executable"""
        return self._executable

    @property
    def version(self):
        """Short version like 3.7"""
        # pylint: disable=attribute-defined-outside-init
        try:
            return self._version
        except AttributeError:
            self._version = self._get_config_var("py_version_short")
            return self._version

    @property
    def debug(self):
        """True if python library is built with Py_DEBUG"""
        # pylint: disable=attribute-defined-outside-init
        try:
            return self._debug
        except AttributeError:
            output = self._query(
                "import sys; print(int(hasattr(sys, 'gettotalrefcount')))"
            )
            self._debug = bool(int(output))
            return self._debug

    @property
    def library(self):
        """Full path to python library you should link to"""
        # pylint: disable=attribute-defined-outside-init
        try:
            return self._library
        except AttributeError:
            if self._os == "Windows":
                stdlib = self._get_python_path("stdlib")
                version_nodot = self._get_config_var("py_version_nodot")
                postfix = "_d" if self.debug else ""
                self._library = os.path.join(
                    os.path.dirname(stdlib),
                    "libs",
                    "python{}{}.lib".format(version_nodot, postfix),
                )
            else:
                fname = self._get_config_var("LDLIBRARY")
                hints = []
                ld_library_path = os.getenv("LD_LIBRARY_PATH")
                if ld_library_path:
                    hints += ld_library_path.split(os.pathsep)
                hints += [
                    self._get_config_var("LIBDIR"),
                    self._get_config_var("LIBPL"),
                ]
                for dirname in hints:
                    candidate = os.path.join(dirname, fname)
                    if os.path.isfile(candidate):
                        self._library = candidate
                        break
                else:
                    raise RuntimeError("Unable to find {}".format(fname))
            return self._library

    def _get_python_path(self, key):
        return self._query(
            "import sysconfig; print(sysconfig.get_path({!r}))".format(key)
        )

    def _get_config_var(self, key):
        return self._query(
            "import sysconfig; print(sysconfig.get_config_var({!r}))".format(key)
        )

    def _query(self, script):
        return subprocess.check_output(
            [self.executable, "-c", script], universal_newlines=True
        ).strip()
