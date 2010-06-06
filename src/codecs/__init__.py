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

import os.path

for filename in os.listdir(os.path.dirname(__file__)):
	module, ext = os.path.splitext(filename)
	if not ext in ('.py', '.so', '.pyd'):
		continue
	if module == '__init__' or module.startswith('lib'):
		continue
	if module.endswith('_d'):
		module = module[:-2]
	try:
		__import__(module, globals(), locals(), [])
	except ImportError:
		print('Failed to import "liar.codecs.%s"' % module)

# EOF
