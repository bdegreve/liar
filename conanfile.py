import os
import re
import shutil
import subprocess
from pathlib import Path
from typing import List, Optional

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.scm import Git
from conan.tools.system.package_manager import Apt

required_conan_version = ">=2.0.0"

SELF_DIR = Path(__file__).absolute().parent


def _get_version(conanfile: ConanFile) -> Optional[str]:
    try:
        content = (SELF_DIR / "CMakeLists.txt").read_text()
    except FileNotFoundError:
        return None  # Assume conan metadata already knows
    match = re.search(r"project\(liar VERSION (\d+\.\d+\.\d+)\)", content)
    assert match
    version = match.group(1)

    return version


class LiarConan(ConanFile):
    name = "liar"
    license = "GPL-2.0-or-later"
    author = "Bram de Greve <bram.degreve@bramz.net>"
    topics = "C++", "Python"
    settings = "os", "compiler", "build_type", "arch"
    requires = [
        "lass/1.11.0-140+g9ad7c83d",
    ]
    options = {
        "fPIC": [True, False],
        "have_openexr": [True, False],
        "have_libjpeg": [True, False],
        "have_png": [True, False],
        "have_lcms2": [True, False],
        "have_x11": [True, False],
        "spectral_mode": ["RGB", "XYZ", "Banded", "Single"],
    }
    default_options = {
        "fPIC": True,
        "have_openexr": True,
        "have_libjpeg": True,
        "have_png": True,
        "have_lcms2": False,
        "have_x11": True,
        "lass/*:shared": True,
        "openexr/*:shared": False,
        "libjpeg/*:shared": False,
        "lodepng/*:shared": False,
        "lcms/*:shared": False,
        "spectral_mode": "RGB",
    }

    def set_version(self):
        self.version = self.version or _get_version(self)

    def export_sources(self):
        git = Git(self)
        for src in git.included_files():
            dest = os.path.join(self.export_sources_folder, src)
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            shutil.copy2(src, dest)

    def requirements(self):
        if self.options.have_openexr:
            self.requires("openexr/2.5.7")
        if self.options.have_libjpeg:
            self.requires("libjpeg/9c")
        if self.options.have_png:
            self.requires("lodepng/cci.20200615")
        if self.options.have_lcms2:
            self.requires("lcms/2.9")

    def system_requirements(self):
        # depending on the platform or the tools.system.package_manager:tool configuration
        # only one of these will be executed
        if self.options.get_safe("have_x11"):
            Apt(self).install(["libx11-dev"])

    def config_options(self):
        if self.settings.get_safe("os") == "Windows":
            self.options.rm_safe("fPIC")
            self.options.rm_safe("have_x11")

    def validate(self):
        if self.settings.get_safe("compiler.cppstd"):
            check_min_cppstd(self, 17)
        if self.settings.compiler.value in ["gcc", "clang"]:
            if self.settings.compiler.libcxx in ["libstdc++"]:
                raise ConanInvalidConfiguration(
                    "gcc and clang require C++11 compatible libcxx"
                )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        lass = self.dependencies["lass"]
        python = Python(lass.options.python_executable, self.settings)
        if str(lass.options.python_version) != python.version:
            raise ConanInvalidConfiguration(
                "python_version option not compatible with python_executable:"
                "{} != {}".format(str(self.options.python_version), python.version)
            )
        if lass.options.python_debug != python.debug:
            raise ConanInvalidConfiguration(
                "python_debug option not compatible with python_executable."
            )
        if self.settings.os == "Windows":
            if python.debug and self.settings.build_type != "Debug":
                raise ConanInvalidConfiguration(
                    "Can't build non-debug Lass with debug Python on Windows."
                )

        tc = CMakeToolchain(self)
        tc.cache_variables["CMAKE_CONFIGURATION_TYPES"] = str(self.settings.build_type)
        tc.cache_variables["SPECTRAL_MODE"] = str(self.options.spectral_mode)
        tc.cache_variables["Lass_DIR"] = os.path.join(
            lass.package_folder, "share", "Lass"
        )
        tc.variables["Python_EXECUTABLE"] = python.executable.as_posix()
        tc.variables["Python_LIBRARY_RELEASE"] = python.library.as_posix()
        tc.variables["Python_LIBRARY_DEBUG"] = python.library.as_posix()
        if self.options.have_lcms2:
            tc.cache_variables["LCMS2_ENABLE"] = True
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.env_info.PYTHONPATH.append(self.package_folder)


class Python(object):
    """Build and config info of a Python"""

    def __init__(self, executable, settings):
        found_executable = shutil.which(str(executable))
        if not found_executable:
            raise ConanInvalidConfiguration(
                f"Python executable cannot be found: {executable!s}"
            )
        self._executable = Path(found_executable)
        self._os = settings.os

    @property
    def executable(self) -> Path:
        """Full path to Python executable"""
        return self._executable

    @property
    def version(self) -> str:
        """Short version like 3.7"""
        # pylint: disable=attribute-defined-outside-init
        try:
            return self._version
        except AttributeError:
            self._version = self._get_config_var("py_version_short")
            return self._version

    @property
    def debug(self) -> bool:
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
    def library(self) -> Path:
        """Full path to python library you should link to"""
        # pylint: disable=attribute-defined-outside-init
        try:
            return self._library
        except AttributeError:
            if self._os == "Windows":
                stdlib = self._get_python_path("stdlib")
                version = self._get_config_var("py_version_nodot")
                postfix = "_d" if self.debug else ""
                self._library = stdlib.parent / f"libs/python{version}{postfix}.lib"
            else:
                fname = self._get_config_var("LDLIBRARY")
                hints: List[Path] = []
                ld_library_path = os.getenv("LD_LIBRARY_PATH")
                if ld_library_path:
                    hints.extend(
                        Path(dirname) for dirname in ld_library_path.split(os.pathsep)
                    )
                hints += [
                    Path(self._get_config_var("LIBDIR")),
                    Path(self._get_config_var("LIBPL")),
                ]
                for dirname in hints:
                    candidate = dirname / fname
                    if candidate.is_file():
                        self._library = candidate
                        break
                else:
                    raise RuntimeError(f"Unable to find {fname}")
            return self._library

    def _get_python_path(self, key: str) -> Path:
        return Path(
            self._query(f"import sysconfig; print(sysconfig.get_path({key!r}))")
        )

    def _get_config_var(self, key: str) -> str:
        return self._query(
            f"import sysconfig; print(sysconfig.get_config_var({key!r}))"
        )

    def _query(self, script: str) -> str:
        return subprocess.check_output(
            [str(self.executable), "-c", script], text=True
        ).strip()
