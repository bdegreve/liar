# LiAR isn't a raytracer
# Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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

import os
import sys
from importlib.abc import MetaPathFinder
from importlib.machinery import ModuleSpec, SourceFileLoader
from pathlib import Path


class LiarLoader(SourceFileLoader):
    def __init__(self, fullname, path, *, data_dir: str):
        super().__init__(fullname, path)
        self.self_dir = Path(path).parent
        self.data_dir = Path(data_dir)

    def get_data(self, path: str) -> bytes:
        try:
            rel_path = Path(path).relative_to(self.self_dir)
            if rel_path.parts[0] == "data":
                data_path = self.data_dir / rel_path.relative_to("data")
                return data_path.read_bytes()
        except (FileNotFoundError, ValueError):
            pass
        return super().get_data(path)


class LiarPathFinder(MetaPathFinder):
    def __init__(self, *, src_dir: str, bin_dir: str, data_dir: str):
        self.src_dir = src_dir
        self.bin_dir = bin_dir
        self.data_dir = data_dir

    def find_spec(self, fullname, path, target=None):
        if not fullname.startswith("liar"):
            return None

        del path  # unused
        del target  # unused

        if fullname == "liar":
            location = os.path.join(self.src_dir, "__init__.py")
            # spec_from_file_location doesn't work: it starts executing the module
            # code without importing the extension modules ...
            # return spec_from_file_location(fullname, loader=LiarLoader(fullname, location))
            spec = ModuleSpec(
                name=fullname,
                loader=LiarLoader(fullname, location, data_dir=self.data_dir),
                origin=location,
                is_package=True,
            )
            spec.submodule_search_locations = [self.bin_dir, self.src_dir]
            spec.has_location = True  # to set __file__
            return spec

        if fullname == "liar.codecs":
            location = os.path.join(self.src_dir, "codecs", "__init__.py")
            spec = ModuleSpec(
                name=fullname,
                loader=SourceFileLoader(fullname, location),
                # origin=location,
                is_package=True,
            )
            spec.submodule_search_locations = [os.path.join(self.bin_dir, "codecs")]
            spec.has_location = True  # to set __file__
            return spec

        if fullname == "liar.tools":
            location = os.path.join(self.src_dir, "tools", "__init__.py")
            spec = ModuleSpec(
                name=fullname,
                loader=SourceFileLoader(fullname, location),
                # origin=location,
                is_package=True,
            )
            spec.submodule_search_locations = [
                os.path.join(self.bin_dir, "tools"),
                os.path.join(self.src_dir, "tools"),
            ]
            spec.has_location = True  # to set __file__
            return spec

        return None


def setup_liar_hooks(root_dir: str, bin_dir: str):
    if any(finder.__class__.__name__ == "LiarPathFinder" for finder in sys.meta_path):
        return  # already set up

    if not os.path.isdir(bin_dir):
        print(f"Skipping liar hooks: {bin_dir=} does not exist", file=sys.stderr)
        return

    print(f"Setting up liar hooks {root_dir=}, {bin_dir=}", file=sys.stderr)

    src_dir = os.path.join(root_dir, "src")
    data_dir = os.path.join(root_dir, "data")

    if os.name == "nt":
        os.add_dll_directory(bin_dir)
    sys.meta_path.insert(
        0, LiarPathFinder(src_dir=src_dir, bin_dir=bin_dir, data_dir=data_dir)
    )
