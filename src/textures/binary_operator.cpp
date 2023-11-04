/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2020  Bram de Greve (bramz@users.sourceforge.net)
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
#include "binary_operator.h"
#include "constant.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(BinaryOperator, "base class of textures mixing two input textures")
PY_CLASS_MEMBER_RW_DOC(BinaryOperator, textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(BinaryOperator, textureB, setTextureB, "second texture")

// --- public --------------------------------------------------------------------------------------

void BinaryOperator::setTextureA(const TTexturePtr& a)
{
	a_ = a;
}



void BinaryOperator::setTextureB(const TTexturePtr& b)
{
	b_ = b;
}



// --- protected -----------------------------------------------------------------------------------

BinaryOperator::BinaryOperator(const TTexturePtr& a, const TTexturePtr& b):
	a_(a),
	b_(b)
{
}



const TPyObjectPtr BinaryOperator::doGetState() const
{
	return python::makeTuple(a_, b_);
}



void BinaryOperator::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, a_, b_);
}



// --- private -------------------------------------------------------------------------------------

bool BinaryOperator::doIsChromatic() const
{
	return a_->isChromatic() || b_->isChromatic();
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
