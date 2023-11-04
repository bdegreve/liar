/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.bramz.net/
 */

#include "mediums_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "beer.h"
#include "bounded.h"
#include "exponential_fog.h"
#include "fog.h"
#include "transformation.h"

using namespace liar::mediums;

PY_DECLARE_MODULE_DOC(mediums, "volume shaders for LiAR")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(mediums, Beer)
PY_MODULE_CLASS(mediums, Bounded)
PY_MODULE_CLASS(mediums, Fog)
	PY_MODULE_CLASS(mediums, ExponentialFog)
PY_MODULE_CLASS(mediums, Transformation)

void mediumsPostInject(PyObject*)
{
	LASS_COUT << "liar.mediums imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	mediums.setPostInject(mediumsPostInject);
	)

PY_MODULE_ENTRYPOINT(mediums)

// EOF
