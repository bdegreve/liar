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
#include "unary_operator.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(UnaryOperator, "Base class for textures operating on one input texture.")
PY_CLASS_MEMBER_RW(UnaryOperator, texture, setTexture);

// --- public --------------------------------------------------------------------------------------

void UnaryOperator::setTexture(const TTexturePtr& texture)
{
	texture_ = texture;
}



// --- protected -----------------------------------------------------------------------------------

UnaryOperator::UnaryOperator(const TTexturePtr& texture):
	texture_(texture)
{
}



const TPyObjectPtr UnaryOperator::doGetState() const
{
	return python::makeTuple(texture_);
}



void UnaryOperator::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, texture_);
}



// --- private -------------------------------------------------------------------------------------

bool UnaryOperator::doIsChromatic() const
{
	return texture_->isChromatic();
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF
