from conans import ConanFile, CMake, tools, errors
import errno
import re


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
    requires = (
        "Lass/1.9.0-194+g0d79721f@cocamware/testing",
        "openexr/2.3.0@conan/stable",
        "libjpeg/9c@bincrafters/stable",
    )
    build_requires = (
        "cmake_installer/[>=3.12]@conan/stable",
    )

    def _configure(self):
        cmake = CMake(self)
        defs = {
            "BUILD_TESTING": self.develop and not self.in_local_cache,
        }
        cmake.configure(source_folder=".", defs=defs)
        return cmake

    def build(self):
        cmake = self._configure()
        cmake.build()

    def package(self):
        cmake = self._configure()
        cmake.install()
