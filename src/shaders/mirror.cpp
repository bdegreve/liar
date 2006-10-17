/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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
#include "mirror.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Mirror)
PY_CLASS_CONSTRUCTOR_0(Mirror)
PY_CLASS_CONSTRUCTOR_1(Mirror, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Mirror, "reflectance", reflectance, setReflectance, "texture for reflectance component")

// --- public --------------------------------------------------------------------------------------

Mirror::Mirror():
	Shader(capsReflection, capsAreStrict),
	reflectance_(Texture::white())
{
}



Mirror::Mirror(const TTexturePtr& reflectance):
	Shader(capsReflection, capsAreStrict),
	reflectance_(reflectance)
{
}



const TTexturePtr& Mirror::reflectance() const
{
	return reflectance_;
}



void Mirror::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Mirror::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaOut,
		const TVector3D* firstOmegaIn, const TVector3D* lastOmegaIn,
		Spectrum* firstValue, TScalar* firstPdf) const
{
	while (firstOmegaIn++ != lastOmegaIn)
	{
		*firstValue++ = Spectrum();
		*firstPdf++ = 0;
	}
}



void Mirror::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const TPoint2D* firstBsdfSample, const TPoint2D* lastBsdfSample,
		TVector3D* firstOmegaOut, Spectrum* firstValue, TScalar* firstPdf,
		unsigned allowedCaps) const
{
	LASS_ASSERT(testCaps(allowedCaps, caps()));
	const Spectrum r = reflectance_->lookUp(sample, context);
	const TVector3D omegaOut(-omegaIn.x, -omegaIn.y, omegaIn.z);
	while (firstBsdfSample++ != lastBsdfSample)
	{
		*firstOmegaOut++ = omegaOut;
		*firstValue++ = r;
		*firstPdf++ = 1;
	}
}



const TPyObjectPtr Mirror::doGetState() const
{
	return python::makeTuple(reflectance_);
}



void Mirror::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, reflectance_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF