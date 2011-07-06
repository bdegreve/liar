#! /usr/bin/env python

# Demonstrates the use of output.Display
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

import sys
from _scene import *

address, port, x_left, x_right = sys.argv[1:]

e = engine()
e.target = output.SocketClient(address, int(port))
e.render(((float(x_left), 0), (float(x_right), 1)))
