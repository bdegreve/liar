#
#	LiAR isn't a raytracer
#	Copyright (C) 2004-2006  Bram de Greve (bramz@sourceforge.net)
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

modules = ['kernel', 'cameras', 'output', 'samplers', 'scenery', 'shaders', 'textures', 'tracers']

import os
import os.path
import sys

script, conf = sys.argv
dirname = os.path.dirname(script)

buildIsComplete = True
for m in modules:
	try:
		state = file(os.path.join(dirname, "%s.%s.state" % (m, conf))).read()
		print m, conf, state
		if state != "end":
			buildIsComplete = False
	except:
		buildIsComplete = False

if not buildIsComplete:
	raise "Build is not complete"

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
print "lassRoot:", lassRoot
print "systemRoot:", systemRoot

if conf == 'Debug':
	title = 'liar_d'
	pyDebug = '_d'
	winDebug = 'd'
	pdb = ['pdb']
else:
	title = 'liar'
	pyDebug = ''
	winDebug = ''
	pdb = ['pdb']
	

from distutils.core import setup,Extension
sys.argv = [sys.argv[0], 'bdist_wininst']

setup(name = title,
	version = "0.2.0",
	author = "Bram de Greve",
	author_email = "bramz@sourceforge.net",
	maintainer = "Bram de Greve",
	maintainer_email = "bramz@sourceforge.net",
	url = "http://liar.sourceforge.net",
	description = "LiAR isn't a raytracer",
	long_description = """     
LiAR isn't a raytracer
Copyright (C) 2004-2006  Bram de Greve (bramz@sourceforge.net)

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
	packages=['liar', 'liar.tools'],
	package_dir={'liar': '..'},      
	data_files = [
		('lib/site-packages/liar',[r'..\..\bin\%s%s.pyd' % (x, pyDebug) for x in modules]),
		('lib/site-packages/liar',[r'..\..\bin\%s%s.%s' % (x, pyDebug, y) for x in modules for y in pdb]),
		('lib/site-packages/liar',[r'..\..\doc\gpl.txt']),
		('lib/site-packages/liar',[r'%sbin\lass_win32_vc8%s.dll' % (lassRoot, pyDebug)]),
		('lib/site-packages/liar',[r'%sbin\lass_win32_vc8%s.%s' % (lassRoot, pyDebug, y) for y in pdb]),
		('lib/site-packages/liar',['%smsvcr71%s.dll' % (systemRoot, winDebug), '%smsvcp71%s.dll' % (systemRoot, winDebug)]),
		]
	)
