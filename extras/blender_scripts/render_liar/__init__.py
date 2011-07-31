# LiAR isn't a raytracer
# Copyright (C) 2004-2011  Bram de Greve (bramz@users.sourceforge.net)
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


# http://wiki.blender.org/index.php/Dev:2.5/Py/Scripts/Guidelines/Addons
bl_info = {
    "name": "LiAR",
    "description": "LiAR integration",
    "author": "Bram de Greve",
    "version": (0, 0, 1),
    "blender": (2, 5, 7),
    "api": 35622,
    "location": "Render > Engine > LiAR",
    "warning": "alpha alert!",
    "category": "Render"
    }

import bpy
import extensions_framework

# import sys
# syspath = r'C:\Program Files (x86)\JetBrains\PyCharm 1.2.1\pycharm-debug.egg'
# if not syspath in sys.path:
    # sys.path.insert(0, syspath)
# from pydev import pydevd

def liar_log(str, popup=False):
    extensions_framework.log(str, popup=popup, module_name="render_liar") 

LiarAddon = extensions_framework.Addon(bl_info)
register, unregister = LiarAddon.init_functions()

from render_liar import render

