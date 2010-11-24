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

#include "textures_common.h"
#include "grid_board.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(GridBoard, "mixes two textures in 2D grid pattern")
PY_CLASS_CONSTRUCTOR_2(GridBoard, TTexturePtr, TTexturePtr);
PY_CLASS_MEMBER_RW(GridBoard, thickness, setThickness);

// --- public --------------------------------------------------------------------------------------

GridBoard::GridBoard(const TTexturePtr& a, const TTexturePtr& b):
	BinaryOperator(a, b),
	halfThickness_(0.05f, 0.05f)
{
}



const TVector2D GridBoard::thickness() const
{
	return 2 * halfThickness_;
}



void GridBoard::setThickness(const TVector2D& thickness)
{
	halfThickness_ = thickness / 2;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr GridBoard::doGetState() const
{
	return python::makeTuple(BinaryOperator::doGetState(), halfThickness_);
}



void GridBoard::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, halfThickness_);
	BinaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const XYZ GridBoard::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TScalar u = num::fractional(context.uv().x);
	const TScalar v = num::fractional(context.uv().y);
	const bool isA = u < halfThickness_.x || v < halfThickness_.x || 
		u > (TNumTraits::one - halfThickness_.x) || v > (TNumTraits::one - halfThickness_.y);
	return (isA ? textureA() : textureB())->lookUp(sample, context);	
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

