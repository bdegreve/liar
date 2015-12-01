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



# adjust dlopen flags so we can share symbols across extension modules
# http://liar.bramz.net/2007/01/28/shared-libraries-dlopen-and-rtti/
#
import sys as _sys
try:
	_oldflags = _sys.getdlopenflags()
except AttributeError:
	pass
else:
	try:
		import dl as _dl
	except ImportError:
		import DLFCN as _dl
	_sys.setdlopenflags(_dl.RTLD_NOW | _dl.RTLD_GLOBAL)


from liar.kernel import *
import liar.cameras
import liar.codecs
import liar.mediums
import liar.output
import liar.samplers
import liar.scenery
import liar.shaders
import liar.spectra
import liar.textures
import liar.tracers

def _load_observer(resource):
    import pkgutil
    import csv
    data = pkgutil.get_data('liar', resource)
    lines = (line for line in data.splitlines() if not line.startswith('#'))
    reader = csv.reader(lines, dialect='excel-tab')
    columns = { col[0]: map(float, col[1:]) for col in zip(*reader) }
    wavelengths = columns['w']
    if any(w > 1 for w in wavelengths):
        # if so, we assume wavelengths are expressed in nanometers. Convert to meters!
        wavelengths = [w * 1e-9 for w in columns['w']]
    return liar.Observer(wavelengths, zip(columns['xbar'], columns['ybar'], columns['zbar']))

liar.Observer.setStandard(_load_observer('data/observer.tsv'))


def _load_recovery_meng_simon(resource):
    import pkgutil
    import json
    data = json.loads(pkgutil.get_data('liar', resource))
    wavelengths = data['wavelengths']
    spectra = {tuple(s['xy']): s['spectrum'] for s in data['spectra']}
    return liar.spectra.RecoveryMengSimon(wavelengths, spectra)

liar.Recovery.setStandard(_load_recovery_meng_simon('data/recovery_meng_simon.json'))



# EOF
