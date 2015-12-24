# LiAR isn't a raytracer
# Copyright (C) 2004-2015  Bram de Greve (bramz@users.sourceforge.net)
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

def load(path):
    with open(path) as f:
        return _parse(f)

def _parse(f):
    from liar.spectra import Sampled
    valid_line = lambda line: line and not line.startswith('#') and not line.startswith(';')

    lines = filter(valid_line, (line.strip() for line in f))
    records = (map(float, line.split()) for line in lines)
    columns = zip(*records)
    wavelengths = columns[0]
    if any(w > 1 for w in wavelengths):
        # assume wavelengths in nanometers, convert to meters
        wavelengths = [w * 1e-9 for w in wavelengths]
    return [Sampled(wavelengths, values) for values in columns[1:]]