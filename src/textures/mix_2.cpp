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
#include "mix_2.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(Mix2)
PY_CLASS_MEMBER_RW_DOC(Mix2, "textureA", textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(Mix2, "textureB", textureB, setTextureB, "second texture")

// --- public --------------------------------------------------------------------------------------

void Mix2::setTextureA(const kernel::TTexturePtr& iA)
{
	a_ = iA;
}



void Mix2::setTextureB(const kernel::TTexturePtr& iB)
{
	b_ = iB;
}



// --- protected -----------------------------------------------------------------------------------

Mix2::Mix2(PyTypeObject* iType, const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB):
	Texture(iType),
	a_(iA),
	b_(iB)
{
}



// --- private -------------------------------------------------------------------------------------




// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

