/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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
#include "bump_mapping.h"
#include "cook_torrance.h"
#include "conductor.h"
#include "dielectric.h"
#include "flip.h"
#include "jakob.h"
#include "lafortune.h"
#include "lambert.h"
#include "linear_interpolator.h"
#include "microfacet_beckmann.h"
#include "microfacet_blinn.h"
#include "microfacet_trowbridge_reitz.h"
#include "mirror.h"
#include "oren_nayar.h"
//#include "null_shader.h"
//#include "simple.h"
#include "sum.h"
#include "thin_dielectric.h"
#include "unshaded.h"
#include "walter.h"

using namespace liar::shaders;

PY_DECLARE_MODULE_DOC(shaders, "surface shaders for LiAR")

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(shaders, AshikhminShirley)
PY_MODULE_CLASS(shaders, BumpMapping)
PY_MODULE_CLASS(shaders, CookTorrance)
PY_MODULE_CLASS(shaders, Conductor)
PY_MODULE_CLASS(shaders, Dielectric)
PY_MODULE_CLASS(shaders, Flip)
PY_MODULE_CLASS(shaders, Jakob)
PY_MODULE_CLASS(shaders, Lafortune)
PY_MODULE_CLASS(shaders, Lambert)
PY_MODULE_CLASS(shaders, LinearInterpolator)
PY_MODULE_CLASS(shaders, Mirror)
PY_MODULE_CLASS(shaders, OrenNayar)
PY_MODULE_CLASS(shaders, MicrofacetBeckmann)
PY_MODULE_CLASS(shaders, MicrofacetBlinn)
PY_MODULE_CLASS(shaders, MicrofacetTrowbridgeReitz)
//PY_MODULE_CLASS(shaders, NullShader)
//PY_MODULE_CLASS(shaders, Simple)
PY_MODULE_CLASS(shaders, Sum)
PY_MODULE_CLASS(shaders, ThinDielectric)
PY_MODULE_CLASS(shaders, Unshaded)
PY_MODULE_CLASS(shaders, Walter)

void shadersPostInject(PyObject*)
{
	LASS_COUT << "liar.shaders imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	shaders.setPostInject(shadersPostInject);
	)

PY_MODULE_ENTRYPOINT(shaders)

// EOF
