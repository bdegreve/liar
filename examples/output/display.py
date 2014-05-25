#! /usr/bin/env python

# Demonstrates the use of output.Display
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

from _scene import *

e = engine()
e.target = output.Display("display.py", (width, height))
#e.numberOfThreads=1
e.render()

