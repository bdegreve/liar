/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021-2025  Bram de Greve (bramz@users.sourceforge.net)
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
#include "walter.h"
#include "microfacet_trowbridge_reitz.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Walter,
	"Anistropic Microfacet Dielectric BSDF by Walter et al. (2007)\n"
	"\n"
	"With GGX distribution (=Trowbridge-Reitz) and Smith shadow-masking\n"
	"Roughness uses the linearized Disney definition, with alpha = sqr(roughness)")
PY_CLASS_CONSTRUCTOR_0(Walter)
PY_CLASS_CONSTRUCTOR_1(Walter, const TTextureRef&)
PY_CLASS_CONSTRUCTOR_2(Walter, const TTextureRef&, const TTextureRef&)
PY_CLASS_MEMBER_RW_DOC(Walter, innerRefractionIndex, setInnerRefractionIndex, "index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(Walter, outerRefractionIndex, setOuterRefractionIndex, "index of refraction for material on outside")
PY_CLASS_MEMBER_RW_DOC(Walter, reflectance, setReflectance, "additional reflectance multiplier")
PY_CLASS_MEMBER_RW_DOC(Walter, transmittance, setTransmittance, "additional transmittance multiplier")
PY_CLASS_MEMBER_RW_DOC(Walter, roughnessU, setRoughnessU, "linear roughness [0-1] in U direction: alphaU = roughnesU ** 2")
PY_CLASS_MEMBER_RW_DOC(Walter, roughnessV, setRoughnessV, "linear roughness [0-1] in V direction: alphaV = roughnesV ** 2")
PY_CLASS_MEMBER_RW_DOC(Walter, mdf, setMdf, "microfacet distribution")
PY_CLASS_MEMBER_RW_DOC(Walter, numberOfSamples, setNumberOfSamples, "number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

Walter::Walter():
	Walter(Texture::white(), Texture::white())
{
}



Walter::Walter(const TTextureRef& innerRefractionIndex):
	Walter(innerRefractionIndex, Texture::white())
{
}



Walter::Walter(const TTextureRef& innerRefractionIndex, const TTextureRef& outerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::glossy),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(outerRefractionIndex),
	reflectance_(Texture::white()),
	transmittance_(Texture::white()),
	roughnessU_(Texture::black()),
	roughnessV_(Texture::black()),
	mdf_(MicrofacetTrowbridgeReitz::instance()),
	numberOfSamples_(1)
{
}



const TTextureRef& Walter::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void Walter::setInnerRefractionIndex(const TTextureRef& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTextureRef& Walter::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void Walter::setOuterRefractionIndex(const TTextureRef& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



const TTextureRef& Walter::reflectance() const
{
	return reflectance_;
}



void Walter::setReflectance(const TTextureRef& reflectance)
{
	reflectance_ = reflectance;
}



const TTextureRef& Walter::transmittance() const
{
	return transmittance_;
}



void Walter::setTransmittance(const TTextureRef& transmittance)
{
	transmittance_ = transmittance;
}



const TTextureRef& Walter::roughnessU() const
{
	return roughnessU_;
}



void Walter::setRoughnessU(const TTextureRef& roughness)
{
	roughnessU_ = roughness;
}



const TTextureRef& Walter::roughnessV() const
{
	return roughnessV_;
}



void Walter::setRoughnessV(const TTextureRef& roughness)
{
	roughnessV_ = roughness;
}



const TMicrofacetDistributionRef& Walter::mdf() const
{
	return mdf_;
}



void Walter::setMdf(const TMicrofacetDistributionRef& mdf)
{
	mdf_ = mdf;
}



size_t Walter::numberOfSamples() const
{
	return numberOfSamples_;
}



void Walter::setNumberOfSamples(size_t number)
{
	numberOfSamples_ = std::max<size_t>(number, 1);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t Walter::doNumReflectionSamples() const
{
	return numberOfSamples_;
}



size_t Walter::doNumTransmissionSamples() const
{
	return numberOfSamples_;
}



TBsdfPtr Walter::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	typedef Spectral::TValue TValue;
	// in theory, refractive indices must be at least 1, but they must be more than zero for sure.
	// IOR as average of spectral works correctly for LIAR_SPECTRAL_MODE_SINGLE. Everything else uses ... well, and average.
	TValue etaI = std::max(average(outerRefractionIndex_->lookUp(sample, context, SpectralType::Illuminant)), 1e-9f);
	TValue etaT = std::max(average(innerRefractionIndex_->lookUp(sample, context, SpectralType::Illuminant)), 1e-9f);
	if (context.solidEvent() == seLeaving)
		std::swap(etaI, etaT);
	const Spectral reflectance = reflectance_->lookUp(sample, context, SpectralType::Reflectant);
	const Spectral transmittance = transmittance_->lookUp(sample, context, SpectralType::Reflectant);
	const TValue mu = num::sqr(std::max<TValue>(roughnessU_->scalarLookUp(sample, context), 1e-3f));
	const TValue mv = num::sqr(std::max<TValue>(roughnessV_->scalarLookUp(sample, context), 1e-3f));
	return TBsdfPtr(new Bsdf(sample, context, reflectance, transmittance, etaI, etaT, mdf_.get(), mu, mv));
}



const TPyObjectPtr Walter::doGetState() const
{
	return python::makeTuple(innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_, roughnessU_, roughnessV_, numberOfSamples_);
}



void Walter::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_, roughnessU_, roughnessV_, numberOfSamples_);
}



Walter::Bsdf::Bsdf(
		const Sample& sample, const IntersectionContext& context, const Spectral& reflectance, const Spectral& transmittance, TValue etaI, TValue etaT,
	const MicrofacetDistribution* mdf, TValue alphaU, TValue alphaV):
	kernel::Bsdf(sample, context, BsdfCaps::transmission | BsdfCaps::glossy),
	reflectance_(reflectance),
	transmittance_(transmittance),
	mdf_(mdf),
	etaI_(etaI),
	etaT_(etaT),
	alphaU_(alphaU),
	alphaV_(alphaV)
{
}



namespace
{

using TValue = Spectral::TValue;

inline TValue fresnelDielectric(TValue ior, const TVector3D& omegaIn, const TVector3D& h)
{

	const TValue cosI = num::abs(static_cast<TValue>(dot(omegaIn, h)));
	LASS_ASSERT(cosI > 0);
	const TValue sinT2 = num::sqr(ior) * (1 - num::sqr(cosI));
	const TValue cosT = num::sqrt(std::max<TValue>(1 - sinT2, 0));
	const TValue rOrth = (ior * cosI - cosT) / (ior * cosI + cosT);
	const TValue rPar = (cosI - ior * cosT) / (cosI + ior * cosT);
	const TValue rFresnel = (num::sqr(rOrth) + num::sqr(rPar)) / 2;
	return std::min<TValue>(rFresnel, 1);
}

}



BsdfOut Walter::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));

	LIAR_ASSERT(omegaIn.z > 0, "omegaIn=" << omegaIn);
	if (omegaOut.z > 0)
	{
		const TVector3D h = (omegaIn + omegaOut).normal();
		LIAR_ASSERT(h.z > 0, "h=" << h);

		const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
		const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);
		if (pdfRefl < 0)
		{
			return BsdfOut(); // neither reflection or transmission
		}

		TValue pdfH;
		const TValue d = mdf_->D(h, alphaU_, alphaV_, pdfH);
		const TValue g = mdf_->G1(omegaIn, h, alphaU_, alphaV_) * mdf_->G1(omegaOut, h, alphaU_, alphaV_);
		const TValue dh_dwo = static_cast<TValue>(num::inv(4 * dot(omegaIn, h)));

		BsdfOut out(reflectance_, pdfRefl * pdfH * dh_dwo);
		out.value *= rFresnel * d * g / static_cast<TValue>(4 * omegaIn.z * omegaOut.z);
		LIAR_ASSERT_POSITIVE_FINITE(out.value);
		LIAR_ASSERT_POSITIVE_FINITE(out.pdf);
		return out;
	}
	else if (omegaOut.z < 0)
	{
		const TVector3D h = (-etaI_ * omegaIn - etaT_ * omegaOut).normal();
		if (h.z <= 0)
		{
			// the microfacet normal is on the wrong side of the surface
			return BsdfOut();
		}

		const TValue cosThetaI = static_cast<TValue>(num::abs(dot(omegaIn, h)));
		const TValue cosThetaT = static_cast<TValue>(num::abs(dot(omegaOut, h)));

		const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
		const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);
		if (pdfRefl < 0)
		{
			return BsdfOut(); // neither reflection or transmission
		}

		TValue pdfH;
		const TValue d = mdf_->D(h, alphaU_, alphaV_, pdfH);
		const TValue g = mdf_->G1(omegaIn, h, alphaU_, alphaV_) * mdf_->G1(omegaOut, h, alphaU_, alphaV_);
		const TValue dh_dwo = num::sqr(etaT_) * cosThetaT / num::sqr(etaI_ * cosThetaI + etaT_ * cosThetaT);

		BsdfOut out(transmittance_, (1 - pdfRefl) * pdfH * dh_dwo);
		out.value *= ((1 - rFresnel) * d * g * (cosThetaI * dh_dwo)) / static_cast<TValue>(num::abs(omegaIn.z * omegaOut.z));

		// if (adjoint)
		//	out.value *= num::sqr(etaI_) / num::sqr(etaT_)
		LIAR_ASSERT_POSITIVE_FINITE(out.value);
		LIAR_ASSERT_POSITIVE_FINITE(out.pdf);
		return out;
	}
	return BsdfOut();
}



SampleBsdfOut Walter::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const TVector3D h = mdf_->sampleH(sample, alphaU_, alphaV_);
	LASS_ASSERT(h.z > 0);
	TValue pdfH;
	const TValue d = mdf_->D(h, alphaU_, alphaV_, pdfH);

	const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
	const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);
	if (pdfRefl < 0)
	{
		return SampleBsdfOut(); // neither reflection or transmission
	}

	if (componentSample < pdfRefl)
	{
		const TVector3D omegaOut = h.reflect(omegaIn);
		if (omegaOut.z <= 0)
		{
			return SampleBsdfOut();
		}
		const TValue g = mdf_->G1(omegaIn, h, alphaU_, alphaV_) * mdf_->G1(omegaOut, h, alphaU_, alphaV_);
		const TValue dh_dwo = static_cast<TValue>(num::inv(4 * dot(omegaOut, h)));

		SampleBsdfOut out(omegaOut, reflectance_, pdfRefl * pdfH * dh_dwo, BsdfCaps::reflection | BsdfCaps::glossy);
		out.value *= rFresnel * d * g / static_cast<TValue>(4 * omegaIn.z * omegaOut.z);
		return out;
	}
	else
	{
		using TValueTraits = num::NumTraits<TValue>;
		const TValue eta = etaI_ / etaT_;
		const TValue cosThetaI = static_cast<TValue>(num::abs(dot(omegaIn, h)));
		const TValue sinTheta2 = num::sqr(eta) * (TValueTraits::one - num::sqr(cosThetaI));
		const TValue cosThetaT = num::sqrt(std::max(TValueTraits::one - sinTheta2, TValueTraits::zero));
		// const TVector3D omegaOut = ((eta * cosThetaI - num::sqrt(std::max<TScalar>(0, 1 + eta * (num::sqr(cosThetaI) - 1))) * h) - eta * omegaIn).normal();
		const TVector3D omegaOut = (-eta * omegaIn + (eta * cosThetaI - cosThetaT) * h).normal();
		if (omegaOut.z >= 0)
		{
			return SampleBsdfOut();
		}
		const TValue g = mdf_->G1(omegaIn, h, alphaU_, alphaV_) * mdf_->G1(omegaOut, h, alphaU_, alphaV_);
		const TValue dh_dwo = num::sqr(etaT_) * cosThetaT / num::sqr(etaI_ * cosThetaI + etaT_ * cosThetaT);
		SampleBsdfOut out(omegaOut, transmittance_, (1 - pdfRefl) * pdfH * dh_dwo, BsdfCaps::transmission | BsdfCaps::glossy);
		out.value *= ((1 - rFresnel) * d * g * (cosThetaI * dh_dwo)) / static_cast<TValue>(num::abs(omegaIn.z * omegaOut.z));
		return out;
	}
}


TValue Walter::Bsdf::pdfReflection(TValue rFresnel, BsdfCaps allowedCaps) const
{
	const TValue powRefl = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? reflectance_.absAverage() * rFresnel : 0;
	const TValue powTrans = kernel::hasCaps(allowedCaps, BsdfCaps::transmission | BsdfCaps::glossy) ? transmittance_.absAverage() * (1 - rFresnel) : 0;
	const TValue powTotal = powRefl + powTrans;
	if (powTotal == 0)
	{
		return -1; // it's neither reflection nor transmission
	}
	return powRefl / (powRefl + powTrans);
}

// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
