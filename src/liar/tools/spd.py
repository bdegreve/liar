# LiAR isn't a raytracer
# Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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


import pkgutil
from io import StringIO

from liar.spectra import Sampled


def load(fp):
    valid_line = (
        lambda line: line and not line.startswith("#") and not line.startswith(";")
    )

    lines = filter(valid_line, (line.strip() for line in fp))
    records = (map(float, line.split()) for line in lines)
    columns = tuple(zip(*records))
    wavelengths = columns[0]
    if any(w > 1 for w in wavelengths):
        # assume wavelengths in nanometers, convert to meters
        wavelengths = [w * 1e-9 for w in wavelengths]
    return [Sampled(wavelengths, values) for values in columns[1:]]


def loads(s):
    fp = StringIO(s)
    return load(fp)


def load_builtin_refractive_index(material) -> tuple[Sampled, Sampled]:
    data = pkgutil.get_data("liar", f"data/spd/{material}.spd")
    if data is None:
        raise ValueError(f"Material '{material}' not found in built-in data.")
    n, k = loads(data.decode("utf-8"))
    return n, k


def dump(obj, fp):
    if isinstance(obj, Sampled):
        obj = [obj]
    if len(obj) == 0:
        raise ValueError("Must have a list of at least one spectrum")
    if not all(hasattr(o, "wavelengths") and hasattr(o, "values") for o in obj):
        raise ValueError(
            "All spectra must have 'wavelengths' and 'values' attributes (like Sampled)"
        )
    wavelengths = tuple(obj[0].wavelengths)
    for o in obj[1:]:
        if wavelengths != tuple(o.wavelengths):
            raise ValueError(
                "All spectra must be sampled on the same wavelengths. Resample before dumping."
            )
    if all(w < 1 for w in wavelengths):
        # assume wavelengths in meters, convert to nanometers
        wavelengths = [w * 1e9 for w in wavelengths]
    allValues = [tuple(o.values) for o in obj]
    for row in zip(wavelengths, *allValues):
        fp.write(" ".join(map(str, row)) + "\n")
