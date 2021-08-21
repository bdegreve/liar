/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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

#include "textures_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "abs.h"
#include "angular_mapping.h"
#include "binary_operator.h"
#include "checker_board.h"
#include "checker_volume.h"
#include "constant.h"
#include "cube_mapping.h"
#include "division.h"
#include "fbm.h"
#include "frequency.h"
#include "global.h"
#include "grid_board.h"
#include "image.h"
#include "linear_interpolator.h"
#include "max.h"
#include "orco.h"
#include "perlin.h"
#include "product.h"
#include "projection_mapping.h"
#include "ray_generation.h"
#include "screen_space.h"
#include "sin.h"
#include "sky_blend.h"
#include "sqr.h"
#include "sqrt.h"
#include "sum.h"
#include "time.h"
#include "transformation_uv.h"
#include "transformation_local.h"
#include "unary_operator.h"
#include "uv.h"
#include "xyz.h"

using namespace liar::textures;

PY_DECLARE_MODULE_DOC(textures, "textures for LiAR")

// some base classes, keep in keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(textures, BinaryOperator)
PY_MODULE_CLASS(textures, Perlin)
PY_MODULE_CLASS(textures, UnaryOperator)
	PY_MODULE_CLASS(textures, ContextMapping)

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(textures, Abs)
PY_MODULE_CLASS(textures, AngularMapping)
PY_MODULE_CLASS(textures, CheckerBoard)
PY_MODULE_CLASS(textures, CheckerVolume)
PY_MODULE_CLASS(textures, Constant)
PY_MODULE_CLASS(textures, CubeMapping)
PY_MODULE_CLASS(textures, Division)
PY_MODULE_CLASS(textures, FBm)
PY_MODULE_CLASS(textures, Frequency)
PY_MODULE_CLASS(textures, Global)
PY_MODULE_CLASS(textures, GridBoard)
PY_MODULE_CLASS(textures, Image)
PY_MODULE_CLASS(textures, LinearInterpolator)
PY_MODULE_CLASS(textures, Max)
PY_MODULE_CLASS(textures, OrCo)
PY_MODULE_CLASS(textures, Product)
PY_MODULE_CLASS(textures, ProjectionMapping)
PY_MODULE_CLASS(textures, RayGeneration)
PY_MODULE_CLASS(textures, Sin)
PY_MODULE_CLASS(textures, SkyBlend)
PY_MODULE_CLASS(textures, Sqr)
PY_MODULE_CLASS(textures, Sqrt)
PY_MODULE_CLASS(textures, Sum)
PY_MODULE_CLASS(textures, ScreenSpace)
PY_MODULE_CLASS(textures, Time)
PY_MODULE_CLASS(textures, TransformationUv)
PY_MODULE_CLASS(textures, TransformationLocal)
PY_MODULE_CLASS(textures, Uv)
PY_MODULE_CLASS(textures, Xyz)

void texturesPostInject(PyObject*)
{
	LASS_COUT << "liar.textures imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n";
}

LASS_EXECUTE_BEFORE_MAIN(
	textures.setPostInject(texturesPostInject);
	)

PY_MODULE_ENTRYPOINT(textures)

// EOF
