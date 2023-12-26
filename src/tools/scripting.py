# LiAR isn't a raytracer
# Copyright (C) 2012-2023  Bram de Greve (bramz@users.sourceforge.net)
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

_MAX_DISPLAY_WIDTH, _MAX_DISPLAY_HEIGHT = 1500, 1000 # is there a may to figure this out dynamically?

def makeRenderTarget(width, height, filename=None, title=None, display=True, fStops=None, rgbSpace=None, toneMapping=None):
    '''
    Width and height are mandatory.
    If supported, a Display will be made with a maximum size of 800 pixels
    If filename is not none, an Image target will be make.
    '''
    from liar import output, sRGB

    targets = []

    if filename:
        image = output.Image(filename, (width, height))
        if not fStops is None:
            image.exposureStops = fStops
            image.autoExposure = False
        image.toneMapping = toneMapping or output.Image.ToneMapping.Linear
        image.rgbSpace = rgbSpace or sRGB
        targets.append(image)

    if display:
        try:
            Display = output.Display
        except AttributeError:
            pass
        else:
            scale = min(float(_MAX_DISPLAY_WIDTH) / width, float(_MAX_DISPLAY_HEIGHT) / height)
            if scale < 1:
                resolution = int(scale * width), int(scale * height)
            else:
                resolution = width, height
            display = Display(title or filename or "", resolution)
            display.rgbSpace = rgbSpace or sRGB
            if not fStops is None:
                display.autoExposure = False
                display.exposure = fStops
            display.toneMapping = toneMapping or Display.ToneMapping.Linear
            targets.append(display)

    assert targets, "Failed to create any kind of render target"
    if len(targets) == 1:
        return targets[0]
    return output.Splitter(targets)


def renderOptions(**kwargs):
    import sys
    from optparse import OptionParser
    parser = OptionParser()
    for (switch, default) in kwargs.items():
        if isinstance(default, int):
            t = 'int'
        elif isinstance(default, float):
            t = 'float'
        else:
            t = 'string'
        parser.add_option('', '--%s' % switch.replace('_', '-'), action='store', type=t, default=default, help="[default=%default]")
    options, args = parser.parse_args()
    return options
