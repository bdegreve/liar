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
#include "unshaded.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Unshaded)
PY_CLASS_CONSTRUCTOR_0(Unshaded)
PY_CLASS_CONSTRUCTOR_1(Unshaded, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Unshaded, "colour", colour, setColour, "texture")

// --- public --------------------------------------------------------------------------------------

Unshaded::Unshaded():
	colour_(Texture::white())
{
}



Unshaded::Unshaded(const TTexturePtr& iColour):
	colour_(iColour)
{
}



const TTexturePtr& Unshaded::colour() const
{
	return colour_;
}



void Unshaded::setColour(const TTexturePtr& iColour)
{
	colour_ = iColour;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const Spectrum Unshaded::doShade(
	const Sample& iSample,
	const DifferentialRay& iPrimaryRay,
	const Intersection& iIntersection,
	const IntersectionContext& iContext) const
{
	return colour_->lookUp(iSample, iContext);
}



const TPyObjectPtr Unshaded::doGetState() const
{
	return python::makeTuple(colour_);
}



void Unshaded::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, colour_);
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF