/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_CONSTRUCTOR_1(Dielectric, const TTextureRef&)
PY_CLASS_CONSTRUCTOR_2(Dielectric, const TTextureRef&, const TTextureRef&)
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
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular)
{
	init();
}



Dielectric::Dielectric(const TTextureRef& innerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular)
{
	init(innerRefractionIndex);
}



Dielectric::Dielectric(const TTextureRef& innerRefractionIndex, const TTextureRef& outerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::specular)
{
	init(innerRefractionIndex, outerRefractionIndex);
}



const TTextureRef& Dielectric::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void Dielectric::setInnerRefractionIndex(const TTextureRef& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTextureRef& Dielectric::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void Dielectric::setOuterRefractionIndex(const TTextureRef& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



const TTextureRef& Dielectric::reflectance() const
{
	return reflectance_;
}



void Dielectric::setReflectance(const TTextureRef& reflectance)
{
	reflectance_ = reflectance;
}



const TTextureRef& Dielectric::transmittance() const
{
	return transmittance_;
}



void Dielectric::setTransmittance(const TTextureRef& transmittance)
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
	const TValue ior1 = std::max(average(outerRefractionIndex_->lookUp(sample, context, SpectralType::Illuminant)), 1e-9f);
	const TValue ior2 = std::max(average(innerRefractionIndex_->lookUp(sample, context, SpectralType::Illuminant)), 1e-9f);
	const bool isLeaving = context.solidEvent() == seLeaving;
	const TValue ior = isLeaving ? ior2 / ior1 : ior1 / ior2;
	const Spectral reflectance = reflectance_->lookUp(sample, context, SpectralType::Reflectant);
	const Spectral transmittance = transmittance_->lookUp(sample, context, SpectralType::Reflectant);
	const bool isDispersive = outerRefractionIndex_->isChromatic() || innerRefractionIndex_->isChromatic();

	return TBsdfPtr(new DielectricBsdf(sample, context, caps(), ior, reflectance, transmittance, isDispersive));
}


const TPyObjectPtr Dielectric::doGetState() const
{
	return python::makeTuple(innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_);
}



void Dielectric::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_);
}



void Dielectric::init(const TTextureRef& innerRefractionIndex, const TTextureRef& outerRefractionIndex, const TTextureRef& reflectance, const TTextureRef& transmittance)
{
	innerRefractionIndex_ = innerRefractionIndex;
	outerRefractionIndex_ = outerRefractionIndex;
	reflectance_ = reflectance;
	transmittance_ = transmittance;
}



// --- bsdf ----------------------------------------------------------------------------------------

Dielectric::DielectricBsdf::DielectricBsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, TValue ior, const Spectral& reflectance, const Spectral& transmittance, bool isDispersive):
	Bsdf(sample, context, caps),
	reflectance_(reflectance),
	transmittance_(transmittance),
	ior_(ior),
	isDispersive_(isDispersive)
{
	LASS_ASSERT(ior_ > 0);
}



BsdfOut Dielectric::DielectricBsdf::doEvaluate(const TVector3D&, const TVector3D&, BsdfCaps) const
{
	return BsdfOut();
}



SampleBsdfOut Dielectric::DielectricBsdf::doSample(const TVector3D& omegaIn, const TPoint2D&, TScalar componentSample, BsdfCaps allowedCaps) const
{
	typedef Spectral::TValue TValue;

	constexpr BsdfCaps capsRefl = BsdfCaps::reflection | BsdfCaps::specular;
	constexpr BsdfCaps capsTrans = BsdfCaps::transmission | BsdfCaps::specular;

	const TValue cosI = static_cast<TValue>(omegaIn.z);
	if (cosI <= 0)
	{
		return SampleBsdfOut();
	}
	const TValue sinT2 = num::sqr(ior_) * (1 - num::sqr(cosI));
	const TValue cosT = num::sqrt(std::max<TValue>(1 - sinT2, 0));
	const TValue rOrth = (ior_ * cosI - cosT) / (ior_ * cosI + cosT);
	const TValue rPar = (cosI - ior_ * cosT) / (cosI + ior_ * cosT);
	const TValue rFresnel = (num::sqr(rOrth) + num::sqr(rPar)) / 2;
	LIAR_ASSERT_POSITIVE_FINITE(rFresnel);

	const TValue powRefl = kernel::hasCaps(allowedCaps, capsRefl) ? reflectance_.absAverage() * rFresnel : 0;
	const TValue powTrans = kernel::hasCaps(allowedCaps, capsTrans) ? transmittance_.absAverage() * (1 - rFresnel) : 0;
	if (powRefl + powTrans == 0)
	{
		return SampleBsdfOut();
	}
	const TValue probRefl = powRefl / (powRefl + powTrans);

	if (componentSample < probRefl)
	{
		const TVector3D omegaRefl(-omegaIn.x, -omegaIn.y, omegaIn.z);
		return SampleBsdfOut(omegaRefl, reflectance_ * rFresnel / cosI, probRefl, capsRefl);
	}
	else
	{
		LIAR_ASSERT(cosT > 0, "cosT=" << cosT << ", rFresnel=" << rFresnel << ", probRefl=" << probRefl << ", componentSample=" << componentSample);
		const TVector3D omegaTrans(-ior_ * omegaIn.x, -ior_ * omegaIn.y, -cosT);
		return SampleBsdfOut(omegaTrans, transmittance_ * (1 - rFresnel) / cosT, 1 - probRefl, capsTrans);
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
