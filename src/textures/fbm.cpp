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
#include "fbm.h"

namespace liar
{
namespace textures
{

PY_DECLARE_CLASS_DOC(FBm, "3D Perlin based Fractional Brownian motion texture using intersection point as input")
PY_CLASS_CONSTRUCTOR_1(FBm, size_t);
PY_CLASS_CONSTRUCTOR_2(FBm, size_t, TScalar);

// --- public --------------------------------------------------------------------------------------

FBm::FBm(size_t numOctaves)
{
	init(numOctaves);
}



FBm::FBm(size_t numOctaves, TScalar falloff)
{
	init(numOctaves, falloff);
}



// --- protected -----------------------------------------------------------------------------------

const TPyObjectPtr FBm::doGetState() const
{
	return python::makeTuple(Perlin::doGetState(), numOctaves_, falloff_);
}



void FBm::doSetState(const TPyObjectPtr& state)
{
	TPyObjectPtr parentState;
	python::decodeTuple(state, parentState, numOctaves_, falloff_);
	Perlin::doSetState(parentState);
}



// --- private -------------------------------------------------------------------------------------

const XYZ FBm::doLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TScalar squaredFootprint = std::max(context.dPoint_dI().squaredNorm(), context.dPoint_dJ().squaredNorm());
	const TScalar idealNumOctaves = 1 - num::log2(squaredFootprint) / 2;
	const TScalar realNumOctaves = num::clamp(idealNumOctaves, TNumTraits::one, static_cast<TScalar>(numOctaves_));
	const size_t numOctaves = static_cast<size_t>(num::floor(realNumOctaves));
	
	const TVector3D p = context.point().position();

	TScalar result = 0;
	TScalar frequency = 1;
	TScalar weight = 1;
	for (size_t k = 0; k < numOctaves; ++k)
	{
		result += weight * noise(TPoint3D(p * frequency));
		frequency *= 2;
		weight *= falloff_;
	}

	const TScalar fracOctaves = realNumOctaves - numOctaves;
	if (fracOctaves > 0)
	{
		result += fracOctaves * weight * noise(TPoint3D(p * frequency));
	}

	return XYZ(result);
}



void FBm::init(size_t numOctaves, TScalar falloff)
{
	numOctaves_ = numOctaves;
	falloff_ = falloff;
}



// --- free ----------------------------------------------------------------------------------------

}

}

// EOF

