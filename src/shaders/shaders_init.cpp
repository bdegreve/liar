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

#include "shaders_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "ashikhmin_shirley.h"
#include "beer.h"
#include "bounded_medium.h"
#include "bump_mapping.h"
#include "dielectric.h"
#include "exponential_fog.h"
#include "fog.h"
#include "lambert.h"
#include "mirror.h"
//#include "null_shader.h"
//#include "simple.h"
#include "sum.h"
#include "thin_dielectric.h"
#include "unshaded.h"

using namespace liar::shaders;

PY_DECLARE_MODULE_DOC(shaders, "surface and volume shaders for LiAR")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(shaders, AshikhminShirley)
PY_MODULE_CLASS(shaders, Beer)
PY_MODULE_CLASS(shaders, BoundedMedium)
PY_MODULE_CLASS(shaders, BumpMapping)
PY_MODULE_CLASS(shaders, Dielectric)
PY_MODULE_CLASS(shaders, Fog)
PY_MODULE_CLASS(shaders, ExponentialFog)
PY_MODULE_CLASS(shaders, Lambert)
PY_MODULE_CLASS(shaders, Mirror)
//PY_MODULE_CLASS(shaders, NullShader)
//PY_MODULE_CLASS(shaders, Simple)
PY_MODULE_CLASS(shaders, Sum)
PY_MODULE_CLASS(shaders, ThinDielectric)
PY_MODULE_CLASS(shaders, Unshaded)

void shadersPostInject(PyObject*)
{
	LASS_COUT << "liar.shaders imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	shaders.setPostInject(shadersPostInject);
	)

PY_MODULE_ENTRYPOINT(shaders)

// EOF
