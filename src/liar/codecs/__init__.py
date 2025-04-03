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

import importlib
import logging
import pkgutil

_logger = logging.getLogger(__name__)

for _, name, ispkg in pkgutil.iter_modules(__path__):
    full_name = "{}.{}".format(__package__, name)
    if ispkg:
        _logger.warn("%s not loaded.", full_name)
        continue
    try:
        importlib.import_module(full_name)
    except ImportError as err:
        _logger.error("Failed to load %s", full_name, exc_info=1)

# EOF
