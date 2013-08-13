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
        image.toneMapping = toneMapping or "linear"
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
            display = output.Display(title or filename or "", resolution)
            display.rgbSpace = rgbSpace or sRGB
            if not fStops is None:
                display.autoExposure = False
                display.exposure = fStops
            display.toneMapping = toneMapping or "linear"
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
