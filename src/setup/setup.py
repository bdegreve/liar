#
#	LiAR isn't a raytracer
#	Copyright (C) 2004-2005  Bram de Greve (bramz@sourceforge.net)
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	http://liar.sourceforge.net
#

from distutils.core import setup,Extension
import os
import sys

modules = ['kernel', 'cameras', 'output', 'samplers', 'scenery', 'shaders', 'textures']

def getRoot(var):
	root = os.getenv(var)
	while not root:
		raise NameError, "environment variable '%s' is not specified" % var
	root = root.replace('/', '\\')
	if not root.endswith('\\'):
		root = root + '\\'
	return root
	
lassRoot = getRoot('LASSROOT')
systemRoot = getRoot('windir') + 'system32\\'

if '_DEBUG' in sys.argv:
	pyDebug = '_d'
	winDebug = 'd'
else:
	pyDebug = ''
	winDebug = ''
	
sys.argv = [sys.argv[0], 'bdist_wininst']

setup(name = 'liar' + pyDebug,
      version = "0.0",
      author = "Bram de Greve",
      author_email = "bramz@sourceforge.net",
      maintainer = "Bram de Greve",
      maintainer_email = "bramz@sourceforge.net",
      url = "http://liar.sourceforge.net",
      description = "LiAR isn't a raytracer",
      long_description = """     
LiAR isn't a raytracer
Copyright (C) 2004-2005  Bram de Greve (bramz@sourceforge.net)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
""",
      download_url = "http://liar.sourceforge.net",
      packages=['liar'],
      package_dir={'liar': '..'},      
      data_files = [('lib/site-packages/liar',[r'..\..\bin\%s%s.pyd' % (x, pyDebug) for x in modules]),
                    ('lib/site-packages/liar',[r'..\..\doc\gpl.txt']),
                    ('lib/site-packages/liar',[r'%sbin\lass_win32_vc71%s.dll' % (lassRoot, pyDebug)]),
                    ('lib/site-packages/liar',['%smsvcr71%s.dll' % (systemRoot, winDebug), '%smsvcp71%s.dll' % (systemRoot, winDebug)]),
                    ]
      )