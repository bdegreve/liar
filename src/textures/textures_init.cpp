/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
#include "angular_mapping.h"
#include "checker_board.h"
#include "checker_volume.h"
#include "constant.h"
#include "global.h"
#include "grid_board.h"
#include "image.h"
#include "linear_interpolator.h"
#include "mix_2.h"
#include "product.h"
#include "time.h"
#include "transformation_uv.h"
#include "uv.h"
#include "xyz.h"

using namespace liar::textures;

PY_DECLARE_MODULE_DOC(textures, "textures for LiAR")

// some base classes, keep in keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(textures, Mix2, textures)

// keep in alphabetical order please! [Bramz]
//
PY_MODULE_CLASS(textures, AngularMapping, textures)
PY_MODULE_CLASS(textures, CheckerBoard, textures)
PY_MODULE_CLASS(textures, CheckerVolume, textures)
PY_MODULE_CLASS(textures, Constant, textures)
PY_MODULE_CLASS(textures, Global, textures)
PY_MODULE_CLASS(textures, GridBoard, textures)
PY_MODULE_CLASS(textures, Image, textures)
PY_MODULE_CLASS(textures, LinearInterpolator, textures)
PY_MODULE_CLASS(textures, Product, textures)
PY_MODULE_CLASS(textures, Time, textures)
PY_MODULE_CLASS(textures, TransformationUv, textures)
PY_MODULE_CLASS(textures, Uv, textures)
PY_MODULE_CLASS(textures, Xyz, textures)

void texturesPostInject(PyObject*)
{
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.stdout.write('''liar.textures imported (v" LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")\n''')\n");
}

LASS_EXECUTE_BEFORE_MAIN(
	textures.setPostInject(texturesPostInject);
	)

PY_MODULE_ENTRYPOINT(textures)

// EOF
