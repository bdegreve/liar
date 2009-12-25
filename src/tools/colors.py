# LiAR isn't a raytracer
# Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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

import liar.kernel
import liar.tools.colors

def _setDefaultSpaceWrapper(cls, rgbSpace):
	_oldDefaultSpace(cls, rgbSpace)
	_makeColors()
	
def _makeColors():
	colors = {
		'RED': rgb(1, 0, 0)
		}
	liar.tools.colors.__dict__.update(colors)

_oldDefaultSpace = liar.kernel.RgbSpace.setDefaultSpace
liar.kernel.RgbSpace.setDefaultSpace = _setDefaultSpaceWrapper
_makeColors()
