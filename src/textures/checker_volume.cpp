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
#include "checker_volume.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(CheckerVolume, "mixes two textures in 3D checkerboard pattern")
PY_CLASS_CONSTRUCTOR_2(CheckerVolume, const TTexturePtr&, const TTexturePtr&);
PY_CLASS_MEMBER_RW(CheckerVolume, split, setSplit);

// --- public --------------------------------------------------------------------------------------

CheckerVolume::CheckerVolume(const TTexturePtr& a, const TTexturePtr& b):
	BinaryOperator(a, b),
	split_(0.5f, 0.5f, 0.5f)
{
}



const TVector3D& CheckerVolume::split() const
{
	return split_;
}



void CheckerVolume::setSplit(const TVector3D& split)
{
	split_ = split;
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr CheckerVolume::doGetState() const
{
	return python::makeTuple(BinaryOperator::doGetState(), split_);
}



void CheckerVolume::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, split_);
	BinaryOperator::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const Spectral
CheckerVolume::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TScalar x = num::fractional(context.point().x);
	const TScalar y = num::fractional(context.point().y);
	const TScalar z = num::fractional(context.point().z);
	if (z < split_.z)
	{
		return ((x < split_.x) == (y < split_.y) ? textureA() : textureB())->lookUp(sample, context);	
	}
	else
	{
		return ((x < split_.x) != (y < split_.y) ? textureA() : textureB())->lookUp(sample, context);
	}
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

