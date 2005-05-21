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
#include "checker_volume.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS(CheckerVolume)
PY_CLASS_CONSTRUCTOR_2(CheckerVolume, kernel::TTexturePtr, kernel::TTexturePtr);
PY_CLASS_MEMBER_RW_DOC(CheckerVolume, "textureA", textureA, setTextureA, "first texture")
PY_CLASS_MEMBER_RW_DOC(CheckerVolume, "textureB", textureB, setTextureB, "second texture")

// --- public --------------------------------------------------------------------------------------

CheckerVolume::CheckerVolume(const kernel::TTexturePtr& iA, const kernel::TTexturePtr& iB):
	Texture(&Type),
	a_(iA),
	b_(iB)
{
}



const kernel::TTexturePtr& CheckerVolume::textureA() const
{
	return a_;
}



const kernel::TTexturePtr& CheckerVolume::textureB() const
{
	return b_;
}



void CheckerVolume::setTextureA(const kernel::TTexturePtr& iA)
{
	a_ = iA;
}



void CheckerVolume::setTextureB(const kernel::TTexturePtr& iB)
{
	b_ = iB;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

CheckerVolume::TValue CheckerVolume::doLookUp(const kernel::IntersectionContext& iContext) const
{
	const TPoint3D p = iContext.point();
	const TScalar x = num::mod(p.x, TNumTraits::one);
	const TScalar y = num::mod(p.y, TNumTraits::one);
	const TScalar z = num::mod(p.z, TNumTraits::one);
	if (z < .5f)
	{
        return (x < .5f) == (y < .5f) ? (*a_)(iContext) : (*b_)(iContext);
	}
	else
	{
        return (x < .5f) != (y < .5f) ? (*a_)(iContext) : (*b_)(iContext);
	}
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

