# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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

import liar.kernel as _kernel

A = (0.44757, 0.40745)
B = (0.34842, 0.35161)
C = (0.31006, 0.31616)
D50 = (0.34567, 0.35850)
D55 = (0.33242, 0.34743)
D65 = (0.31271, 0.32902)
D75 = (0.29902, 0.31485)
E = (1.0 / 3, 1.0 / 3)

AdobeRGB = _kernel.RgbSpace((0.64, 0.33), (0.21, 0.71), (0.15, 0.06), D65, 2.2)
AppleRGB = _kernel.RgbSpace((0.625, 0.34), (0.28, 0.595), (0.155, 0.07), D65, 1.8)
BestRGB = _kernel.RgbSpace((0.7347, 0.2653), (0.215, 0.775), (0.13, 0.035), D50, 2.2)
BetaRGB = _kernel.RgbSpace(
    (0.6888, 0.3112), (0.1986, 0.7551), (0.1265, 0.0352), D50, 2.2
)
BruceRGB = _kernel.RgbSpace((0.64, 0.33), (0.2800, 0.6500), (0.15, 0.06), D65, 2.2)
CIERGB = _kernel.RgbSpace((0.735, 0.265), (0.274, 0.717), (0.167, 0.009), E, 2.2)
CIEXYZ = _kernel.CIEXYZ
ColorMatchRGB = _kernel.RgbSpace((0.63, 0.34), (0.295, 0.605), (0.15, 0.075), D50, 1.8)
HDTV_RGB = _kernel.sRGB
NTSC_RGB = _kernel.RgbSpace((0.67, 0.33), (0.21, 0.71), (0.14, 0.08), C, 2.2)
PAL_SECAM_RGB = _kernel.RgbSpace((0.64, 0.33), (0.29, 0.6), (0.15, 0.06), D65, 2.2)
ProPhotoRGB = _kernel.RgbSpace(
    (0.7347, 0.2653), (0.1596, 0.8404), (0.0366, 0.0001), D50, 1.8
)
SMPTE_C_RGB = _kernel.RgbSpace((0.63, 0.34), (0.31, 0.595), (0.1550, 0.07), D65, 2.2)
sRGB = _kernel.sRGB
WideGamutRGB = _kernel.RgbSpace(
    (0.735, 0.265), (0.115, 0.826), (0.157, 0.018), D50, 2.2
)
