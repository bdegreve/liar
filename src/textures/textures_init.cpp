/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "textures_common.h"

// keep in alphabetical order please! [Bramz]
//
//#include "angular_mapping.h"
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
#include "uv.h"
#include "xyz.h"

PY_DECLARE_MODULE(textures)

extern "C"
{

LIAR_TEXTURES_DLL void inittextures(void)
{
    using namespace liar::textures;

	PY_INJECT_MODULE_EX(textures, "liar.textures", "textures for LiAR")

	// some base classes, keep in keep in alphabetical order please! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(Mix2, textures, "base class of textures mixing two input textures")
	
	// keep in alphabetical order please! [Bramz]
	//
	//PY_INJECT_CLASS_IN_MODULE(AngularMapping, textures, "converts point to angular coordinates as used in light probes")
	PY_INJECT_CLASS_IN_MODULE(CheckerBoard, textures, "mixes two textures in 2D checkerboard pattern")
	PY_INJECT_CLASS_IN_MODULE(CheckerVolume, textures, "mixes two textures in 3D checkerboard pattern")
	PY_INJECT_CLASS_IN_MODULE(Constant, textures, "texture with constant value")
	PY_INJECT_CLASS_IN_MODULE(Global, textures, "use global texture parameters instead of local")
	PY_INJECT_CLASS_IN_MODULE(GridBoard, textures, "mixes two textures in 2D grid pattern")
	PY_INJECT_CLASS_IN_MODULE(Image, textures, "image file")
	PY_INJECT_CLASS_IN_MODULE(LinearInterpolator, textures, "interpolates textures using gray value of control texture as parameter")
	PY_INJECT_CLASS_IN_MODULE(Product, textures, "makes product of child textures")
	PY_INJECT_CLASS_IN_MODULE(Time, textures, "evaluates to sampled time")
	PY_INJECT_CLASS_IN_MODULE(Uv, textures, "mixes two textures by the u and v context channels")
	PY_INJECT_CLASS_IN_MODULE(Xyz, textures, "mixes three textures by the x, y and z context channels")

	PyRun_SimpleString("print 'liar.textures imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");
}

}

// EOF