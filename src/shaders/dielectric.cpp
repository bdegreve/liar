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

#include "shaders_common.h"
#include "dielectric.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Dielectric, "dielectric Fresnel material (like glass)")
PY_CLASS_CONSTRUCTOR_0(Dielectric)
PY_CLASS_CONSTRUCTOR_1(Dielectric, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(Dielectric, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Dielectric, innerRefractionIndex, setInnerRefractionIndex, 
	"index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(Dielectric, outerRefractionIndex, setOuterRefractionIndex, 
	"index of refraction for material on outside")
PY_CLASS_MEMBER_RW_DOC(Dielectric, reflectance, setReflectance, 
	"multiplier for the Fresnel reflectance")
PY_CLASS_MEMBER_RW_DOC(Dielectric, transmittance, setTransmittance, 
	"multiplier for the Fresnel transmittance")

// --- public --------------------------------------------------------------------------------------

Dielectric::Dielectric():
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular)
{
	init();
}



Dielectric::Dielectric(const TTexturePtr& innerRefractionIndex):
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular)
{
	init(innerRefractionIndex);
}



Dielectric::Dielectric(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex):
	Shader(Bsdf::capsReflection | Bsdf::capsTransmission | Bsdf::capsSpecular)
{
	init(innerRefractionIndex, outerRefractionIndex);
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



const TTexturePtr& Dielectric::reflectance() const
{
	return reflectance_;
}



void Dielectric::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



const TTexturePtr& Dielectric::transmittance() const
{
	return transmittance_;
}



void Dielectric::setTransmittance(const TTexturePtr& transmittance)
{
	transmittance_ = transmittance;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Dielectric::doNumReflectionSamples() const
{
	return 1;
}



size_t Dielectric::doNumTransmissionSamples() const
{
	return 1;
}



TBsdfPtr Dielectric::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	// in theory, refractive indices must be at least 1, but they must be more than zero for sure.
	// IOR as average of spectral works correctly for LIAR_SPECTRAL_MODE_SINGLE. Everything else uses ... well, and average.
	const TValue ior1 = std::max(average(outerRefractionIndex_->lookUp(sample, context, Illuminant)), 1e-9f);
	const TValue ior2 = std::max(average(innerRefractionIndex_->lookUp(sample, context, Illuminant)), 1e-9f);
	const bool isLeaving = context.solidEvent() == seLeaving;
	const TValue ior = isLeaving ? ior2 / ior1 : ior1 / ior2;
	const Spectral reflectance = reflectance_->lookUp(sample, context, Reflectant);
	const Spectral transmittance = transmittance_->lookUp(sample, context, Reflectant);

	return TBsdfPtr(new DielectricBsdf(sample, context, caps(), ior, reflectance, transmittance));
}


const TPyObjectPtr Dielectric::doGetState() const
{
	return python::makeTuple(innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_);
}



void Dielectric::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_);
}



void Dielectric::init(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex, const TTexturePtr& reflectance, const TTexturePtr& transmittance)
{
	innerRefractionIndex_ = innerRefractionIndex;
	outerRefractionIndex_ = outerRefractionIndex;
	reflectance_ = reflectance;
	transmittance_ = transmittance;
}



// --- bsdf ----------------------------------------------------------------------------------------

Dielectric::DielectricBsdf::DielectricBsdf(const Sample& sample, const IntersectionContext& context, TBsdfCaps caps, TValue ior, const Spectral& reflectance, const Spectral& transmittance) :
	Bsdf(sample, context, caps),
	reflectance_(reflectance),
	transmittance_(transmittance),
	ior_(ior)
{
	LASS_ASSERT(ior_ > 0);
}



BsdfOut Dielectric::DielectricBsdf::doEvaluate(const TVector3D&, const TVector3D&, TBsdfCaps) const
{
	return BsdfOut();
}



SampleBsdfOut Dielectric::DielectricBsdf::doSample(const TVector3D& omegaIn, const TPoint2D&, TScalar componentSample, TBsdfCaps allowedCaps) const
{
	typedef Spectral::TValue TValue;

	enum
	{
		capsRefl = Bsdf::capsReflection | Bsdf::capsSpecular,
		capsTrans = Bsdf::capsTransmission | Bsdf::capsSpecular
	};

	const TValue cosI = static_cast<TValue>(omegaIn.z);
	LASS_ASSERT(cosI > 0);
	const TValue sinT2 = num::sqr(ior_) * (1 - num::sqr(cosI));
	const TValue cosT = num::sqrt(std::max<TValue>(1 - sinT2, 0));
	const TValue rOrth = (ior_ * cosI - cosT) / (ior_ * cosI + cosT);
	const TValue rPar = (cosI - ior_ * cosT) / (cosI + ior_ * cosT);
	const TValue rFresnel = (num::sqr(rOrth) + num::sqr(rPar)) / 2;

	const TValue powRefl = kernel::hasCaps(allowedCaps, capsRefl) ? reflectance_.absAverage() * rFresnel : 0;
	const TValue powTrans = kernel::hasCaps(allowedCaps, capsTrans) ? transmittance_.absAverage() * (1 - rFresnel) : 0;
	LASS_ASSERT(powRefl + powTrans > 0);
	const TValue probRefl = powRefl / (powRefl + powTrans);
	
	if (componentSample < probRefl)
	{
		const TVector3D omegaRefl(-omegaIn.x, -omegaIn.y, omegaIn.z);
		return SampleBsdfOut(omegaRefl, reflectance_ * rFresnel / cosI, probRefl, capsRefl);
	}
	else
	{
		TVector3D omegaTrans = -omegaIn;
		omegaTrans *= ior_;
		omegaTrans.z += (ior_ * cosI - cosT);
		const TValue cosO = static_cast<TValue>(omegaTrans.z);
		return SampleBsdfOut(omegaTrans, transmittance_ * (1 - rFresnel) / num::abs(cosO), 1 - probRefl, capsTrans);
	}
}



bool Dielectric::DielectricBsdf::doIsDispersive() const
{
	return isDispersive_;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
