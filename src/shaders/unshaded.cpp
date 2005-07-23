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
PY_CLASS_CONSTRUCTOR_1(Unshaded, const kernel::TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Unshaded, "value", value, setValue, "texture")

// --- public --------------------------------------------------------------------------------------

Unshaded::Unshaded():
	kernel::Shader(&Type),
	value_(kernel::Texture::white())
{
}



Unshaded::Unshaded(const kernel::TTexturePtr& iValue):
	kernel::Shader(&Type),
	value_(iValue)
{
}



const kernel::TTexturePtr& Unshaded::value() const
{
	return value_;
}



void Unshaded::setValue(const kernel::TTexturePtr& iValue)
{
	value_ = iValue;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

kernel::Spectrum Unshaded::doUnshaded(const kernel::Sample& iSample,
									 const kernel::IntersectionContext& iContext)
{
	return value_->lookUp(iSample, iContext);
}



kernel::Spectrum Unshaded::doDirectLight(const kernel::Sample& iSample,
										const kernel::DifferentialRay& iPrimaryRay,
										const kernel::Intersection& iIntersection,
										const kernel::IntersectionContext& iContext,
										const kernel::TSceneObjectPtr& iScene,
										const kernel::LightContext& iLight)
{
	return kernel::Spectrum();
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF