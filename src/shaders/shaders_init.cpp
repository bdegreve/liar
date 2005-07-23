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

#include "shaders_common.h"

// keep in alphabetical order please! [Bramz]
//
#include "lambert.h"
#include "simple.h"
#include "unshaded.h"

PY_DECLARE_MODULE(shaders)

extern "C"
{

LIAR_SHADERS_DLL void initshaders(void)
{
	using namespace liar::shaders;

	PY_INJECT_MODULE_EX(shaders, "liar.shaders", "surface and volume shaders for LiAR")

	// keep in alphabetical order please! [Bramz]
	//
    PY_INJECT_CLASS_IN_MODULE(Lambert, shaders, "perfect lambert shader")
    PY_INJECT_CLASS_IN_MODULE(Simple, shaders, "a simple shader")
    PY_INJECT_CLASS_IN_MODULE(Unshaded, shaders, "a shader that doesn't shade")

	PyRun_SimpleString("print 'liar.shaders imported (v" 
		LIAR_VERSION_FULL " - " __DATE__ ", " __TIME__ ")'\n");
}

}

// EOF