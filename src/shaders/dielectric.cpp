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
#include "dielectric.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Dielectric)
PY_CLASS_CONSTRUCTOR_0(Dielectric)
PY_CLASS_CONSTRUCTOR_1(Dielectric, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(Dielectric, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Dielectric, "innerRefractionIndex", innerRefractionIndex, setInnerRefractionIndex, 
	"index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(Dielectric, "outerRefractionIndex", outerRefractionIndex, setOuterRefractionIndex, 
	"index of refraction for material on outside")

// --- public --------------------------------------------------------------------------------------

Dielectric::Dielectric():
	Shader(capsReflection | capsTransmission | capsSpecular),
	innerRefractionIndex_(Texture::white()),
	outerRefractionIndex_(Texture::white())
{
}



Dielectric::Dielectric(const TTexturePtr& innerRefractionIndex):
	Shader(capsReflection | capsTransmission | capsSpecular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(Texture::white())
{
}



Dielectric::Dielectric(
		const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex):
	Shader(capsReflection | capsTransmission | capsSpecular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(outerRefractionIndex)
{
}



const TTexturePtr& Dielectric::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void Dielectric::setInnerRefractionIndex(const TTexturePtr& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTexturePtr& Dielectric::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void Dielectric::setOuterRefractionIndex(const TTexturePtr& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

const unsigned Dielectric::doNumReflectionSamples() const
{
	return 1;
}



const unsigned Dielectric::doNumTransmissionSamples() const
{
	return 1;
}



void Dielectric::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const
{
}



void Dielectric::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{
	TScalar ior1 = outerRefractionIndex_->lookUp(sample, context).average();
	TScalar ior2 = innerRefractionIndex_->lookUp(sample, context).average();

	if (ior1 <= 0)
	{
		LASS_WARNING_ONCE("detected outerRefractionIndex evaluating to zero or less.  Defaulting to 1."
			"See documentation");
		ior1 = 1;
	}
	if (ior2 <= 0)
	{
		LASS_WARNING_ONCE("detected innerRefractionIndex evaluating to zero or less.  Defaulting to 1."
			"See documentation");
		ior2 = 1;
	}

	const TVector3D omegaRefl(-omegaIn.x, -omegaIn.y, omegaIn.z);

	// http://users.skynet.be/bdegreve/writings/reflection_transmission.pdf
	const bool isLeaving = context.solidEvent() == seLeaving;
	const TScalar ior = isLeaving ? ior2 / ior1 : ior1 / ior2;
	const TScalar cosI = omegaIn.z;
	LASS_ASSERT(cosI > 0);
	const TScalar sinT2 = num::sqr(ior) * (1 - num::sqr(cosI));
	TScalar r = 1;
	TVector3D omegaTrans = -omegaIn;
	if (sinT2 < 1)
	{
		const TScalar cosT = num::sqrt(1 - sinT2);
		const TScalar rOrth = (ior1 * cosI - ior2 * cosT) / (ior1 * cosI + ior2 * cosT);
		const TScalar rPar = (ior2 * cosI - ior1 * cosT) / (ior2 * cosI + ior1 * cosT);
		r = (num::sqr(rOrth) + num::sqr(rPar)) / 2;

		omegaTrans *= ior;
		omegaTrans.z += (ior * cosI - cosT);
	}

	while (first != last)
	{
		const bool doReflection = testCaps(first->allowedCaps, capsReflection | capsSpecular);
		const bool doTransmission = testCaps(first->allowedCaps, capsTransmission | capsSpecular);
		const TScalar pr = doReflection ? (doTransmission ? r : 1) : 0;

		if (first->sample.x < pr)
		{
			result->omegaOut = omegaRefl;
			result->value = r;
			result->pdf = pr;
			result->usedCaps = capsReflection | capsSpecular;
		}
		else
		{
			result->omegaOut = omegaTrans;
			result->value = (1 - r);
			result->pdf = 1 - pr;
			result->usedCaps = capsTransmission | capsSpecular;
		}
		++first;
		++result;
	}
}



const TPyObjectPtr Dielectric::doGetState() const
{
	return python::makeTuple(outerRefractionIndex_, innerRefractionIndex_);
}



void Dielectric::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, outerRefractionIndex_, innerRefractionIndex_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF