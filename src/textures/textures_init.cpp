/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
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
#include "checker_board.h"
#include "checker_volume.h"
#include "constant.h"
#include "mix_2.h"
#include "uv.h"

PY_DECLARE_MODULE(textures)

extern "C"
{

LIAR_TEXTURES_DLL void inittextures(void)
{
#ifndef _DEBUG
	//lass::io::proxyMan()->clog()->remove(&std::clog);
#endif

    using namespace liar::textures;

	PY_INJECT_MODULE_EX(textures, "liar.textures", "textures for LiAR")

	// some base classes, keep in keep in alphabetical order please! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(Mix2, textures, "base class of textures mixing two input textures")
	
	// keep in alphabetical order please! [Bramz]
	//
	PY_INJECT_CLASS_IN_MODULE(CheckerBoard, textures, "mixes two textures in 2D checkerboard pattern")
	PY_INJECT_CLASS_IN_MODULE(CheckerVolume, textures, "mixes two textures in 3D checkerboard pattern")
	PY_INJECT_CLASS_IN_MODULE(Constant, textures, "texture with constant value")
	PY_INJECT_CLASS_IN_MODULE(Uv, textures, "mixes two textures by the u and v context channels")

	PyRun_SimpleString("print 'liar.textures imported'\n");
}

}

// EOF