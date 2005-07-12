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
#include "grid_board.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(GridBoard)
PY_CLASS_CONSTRUCTOR_2(GridBoard, kernel::TTexturePtr, kernel::TTexturePtr);
PY_CLASS_MEMBER_RW(GridBoard, "split", split, setSplit);

// --- public --------------------------------------------------------------------------------------

GridBoard::GridBoard(const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB):
	Mix2(&Type, iA, iB),
	split_(0.05f, 0.05f)
{
}



const TVector2D& GridBoard::split() const
{
	return split_;
}



void GridBoard::setSplit(const TVector2D& iSplit)
{
	split_ = iSplit;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

kernel::Spectrum GridBoard::doLookUp(const kernel::IntersectionContext& iContext) const
{
	const TScalar u = num::mod(iContext.uv().x, TNumTraits::one);
	const TScalar v = num::mod(iContext.uv().y, TNumTraits::one);
	const bool isA = u < split_.x || v < split_.x || 
		u > (TNumTraits::one - split_.x) || v > (TNumTraits::one - split_.y);
	return (isA ? textureA() : textureB())->lookUp(iContext);	
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

