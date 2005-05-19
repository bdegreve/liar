/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LIAR isn't a raytracer
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
#include "checker_board.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(CheckerBoard)
PY_CLASS_CONSTRUCTOR_2(CheckerBoard, kernel::TTexturePtr, kernel::TTexturePtr);
PY_CLASS_MEMBER_RW_DOC(CheckerBoard, "textureA", textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(CheckerBoard, "textureB", textureB, setTextureB, "second texture")

// --- public --------------------------------------------------------------------------------------

CheckerBoard::CheckerBoard(const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB):
	Texture(&Type),
	a_(iA),
	b_(iB)
{
}



const kernel::TTexturePtr& CheckerBoard::textureA() const
{
	return a_;
}



const kernel::TTexturePtr& CheckerBoard::textureB() const
{
	return b_;
}



void CheckerBoard::setTextureA(const kernel::TTexturePtr& iA)
{
	a_ = iA;
}



void CheckerBoard::setTextureB(const kernel::TTexturePtr& iB)
{
	b_ = iB;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

CheckerBoard::TValue CheckerBoard::doLookUp(const kernel::IntersectionContext& iContext) const
{
	const TScalar u = num::mod(iContext.u(), TNumTraits::one);
	const TScalar v = num::mod(iContext.v(), TNumTraits::one);
	return (u < .5f) == (v < .5f) ? (*a_)(iContext) : (*b_)(iContext);	
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

