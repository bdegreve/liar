# LiAR isn't a raytracer
# Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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
from urllib.request import urlopen

__all__ = ["download"]


def download(url, fname=None):
    fname = fname or os.path.basename(url)
    if not os.path.exists(fname):
        print("Downloading '{}' ...".format(url))
        with open(fname, "wb") as fp, urlopen(url) as res:
            fp.write(res.read())
    return fname


# EOF
