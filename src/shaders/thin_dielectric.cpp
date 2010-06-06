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

#include "shaders_common.h"
#include "thin_dielectric.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(ThinDielectric, "thin dielectric material")
PY_CLASS_CONSTRUCTOR_0(ThinDielectric)
PY_CLASS_CONSTRUCTOR_1(ThinDielectric, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(ThinDielectric, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, innerRefractionIndex, setInnerRefractionIndex, 
	"index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, outerRefractionIndex, setOuterRefractionIndex, 
	"index of refraction for material on outside")
PY_CLASS_MEMBER_RW_DOC(ThinDielectric, transparency, setTransparency, 
	"colour of transparency")

// --- public --------------------------------------------------------------------------------------

ThinDielectric::ThinDielectric():
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular),
	innerRefractionIndex_(Texture::white()),
	outerRefractionIndex_(Texture::white()),
	transparency_(Texture::white())
{
}



ThinDielectric::ThinDielectric(const TTexturePtr& innerRefractionIndex):
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(Texture::white()),
	transparency_(Texture::white())
{
}



ThinDielectric::ThinDielectric(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex):
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(outerRefractionIndex),
	transparency_(Texture::white())
{
}



const TTexturePtr& ThinDielectric::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void ThinDielectric::setInnerRefractionIndex(const TTexturePtr& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTexturePtr& ThinDielectric::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void ThinDielectric::setOuterRefractionIndex(const TTexturePtr& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



const TTexturePtr& ThinDielectric::transparency() const
{
	return transparency_;
}



void ThinDielectric::setTransparency(const TTexturePtr& transparency)
{
	transparency_ = transparency;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t ThinDielectric::doNumReflectionSamples() const
{
	return 1;
}



size_t ThinDielectric::doNumTransmissionSamples() const
{
	return 1;
}



void ThinDielectric::doBsdf(const Sample&, const IntersectionContext&, const TVector3D&, const BsdfIn*, const BsdfIn*, BsdfOut*) const
{
}

#define LIAR_WARN_ONCE_EX(x, uniqueName)\
	do\
	{\
		static bool uniqueName = false;\
		if (!uniqueName)\
		{\
			LASS_WARNING(x);\
			uniqueName = true;\
		}\
	}\
	while (false)

#define LIAR_WARN_ONCE(x)\
	LIAR_WARN_ONCE_EX(x, LASS_UNIQUENAME(liarWarnOnce))

void ThinDielectric::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn,
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{
	TScalar ior1 = average(outerRefractionIndex_->lookUp(sample, context));
	TScalar ior2 = average(innerRefractionIndex_->lookUp(sample, context));
	const XYZ transparency = clamp(transparency_->lookUp(sample, context), TNumTraits::zero, TNumTraits::one);

	if (ior1 <= 0)
	{
		LIAR_WARN_ONCE("detected outerRefractionIndex evaluating to zero or less.  Defaulting to 1."
			"See documentation");
		ior1 = 1;
	}
	if (ior2 <= 0)
	{
		LIAR_WARN_ONCE("detected innerRefractionIndex evaluating to zero or less.  Defaulting to 1."
			"See documentation");
		ior2 = 1;
	}

	// http://users.skynet.be/bdegreve/writings/reflection_transmission.pdf
	const TScalar ior = ior1 / ior2; 
	const TScalar cosI = omegaIn.z;
	LASS_ASSERT(cosI > 0);
	const TScalar sinT2 = num::sqr(ior) * (1 - num::sqr(cosI));
	XYZ R(TNumTraits::one);
	XYZ T(TNumTraits::zero);
	if (sinT2 < 1)
	{
		const TScalar cosT = num::sqrt(1 - sinT2);
		LASS_ASSERT(cosT > 0);
		const TScalar rOrth = (ior1 * cosI - ior2 * cosT) / (ior1 * cosI + ior2 * cosT);
		const TScalar rPar = (ior2 * cosI - ior1 * cosT) / (ior2 * cosI + ior1 * cosT);
		const TScalar r = (num::sqr(rOrth) + num::sqr(rPar)) / 2;
		LASS_ASSERT(r < 1);
		const XYZ t = pow(transparency, num::inv(cosT));
		
		R = r * (1 + sqr((1 - r) * t) / (1 - sqr(r * t)));
		R = clamp(R, TNumTraits::zero, TNumTraits::one);
		T = num::sqr(1 - r) * t / (1 - sqr(r * t));
		T = clamp(T, TNumTraits::zero, TNumTraits::one);
		//LASS_COUT << R << "\n";
	}
	else
	{
		LASS_COUT << "uhoh: cosI=" << cosI << ", sinT2=" << sinT2 << ", ior=" << ior << "\n";
	}
	const TScalar r = average(R);
	const TScalar t = average(T);

	while (first != last)
	{
		const bool doReflection = kernel::hasCaps(first->allowedCaps, Bsdf::capsReflection | Bsdf::capsSpecular);
		const bool doTransmission = kernel::hasCaps(first->allowedCaps, Bsdf::capsTransmission | Bsdf::capsSpecular);
		const TScalar sr = doReflection ? (doTransmission ? r : 1) : 0;
		const TScalar st = doTransmission ? (doReflection ? t : 1) : 0;
		if (sr > 0 || st > 0)
		{
			const TScalar pr = sr / (sr + st);

			if (first->sample.x < pr)
			{
				result->omegaOut = TVector3D(-omegaIn.x, -omegaIn.y, omegaIn.z);
				result->value = R;
				result->pdf = pr;
				result->usedCaps = Bsdf::capsReflection | Bsdf::capsSpecular;
			}
			else
			{
				result->omegaOut = -omegaIn;
				result->value = T;
				result->pdf = 1 - pr;
				result->usedCaps = Bsdf::capsTransmission | Bsdf::capsSpecular;
			}
		}
		++first;
		++result;
	}
}



const TPyObjectPtr ThinDielectric::doGetState() const
{
	return python::makeTuple(outerRefractionIndex_, innerRefractionIndex_, transparency_);
}



void ThinDielectric::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, outerRefractionIndex_, innerRefractionIndex_, transparency_);
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
