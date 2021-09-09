/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(Walter, "Anisotropic Phong BRDF by Ashikhmin & Shirley (2001)")
PY_CLASS_CONSTRUCTOR_0(Walter)
PY_CLASS_CONSTRUCTOR_1(Walter, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(Walter, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Walter, innerRefractionIndex, setInnerRefractionIndex, "index of refraction for material on inside")
PY_CLASS_MEMBER_RW_DOC(Walter, outerRefractionIndex, setOuterRefractionIndex, "index of refraction for material on outside")
PY_CLASS_MEMBER_RW_DOC(Walter, reflectance, setReflectance, "texture for additional reflectance multiplier")
PY_CLASS_MEMBER_RW_DOC(Walter, transmittance, setTransmittance, "texture for additional transmittance multiplier")
PY_CLASS_MEMBER_RW_DOC(Walter, roughnessU, setRoughnessU, "texture for the rms microfacet slope in U direction")
PY_CLASS_MEMBER_RW_DOC(Walter, roughnessV, setRoughnessV, "texture for the rms microfacet slope in V direction")
PY_CLASS_MEMBER_RW_DOC(Walter, numberOfSamples, setNumberOfSamples, "set number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

Walter::Walter():
	Walter(Texture::white(), Texture::white())
{
}



Walter::Walter(const TTexturePtr& innerRefractionIndex):
	Walter(innerRefractionIndex, Texture::white())
{
}



Walter::Walter(const TTexturePtr& innerRefractionIndex, const TTexturePtr& outerRefractionIndex):
	Shader(BsdfCaps::reflection | BsdfCaps::transmission | BsdfCaps::glossy),
	innerRefractionIndex_(innerRefractionIndex),
	outerRefractionIndex_(outerRefractionIndex),
	reflectance_(Texture::white()),
	transmittance_(Texture::white()),
	roughnessU_(Texture::black()),
	roughnessV_(Texture::black()),
	numberOfSamples_(9)
{
}



const TTexturePtr& Walter::innerRefractionIndex() const
{
	return innerRefractionIndex_;
}



void Walter::setInnerRefractionIndex(const TTexturePtr& innerRefractionIndex)
{
	innerRefractionIndex_ = innerRefractionIndex;
}



const TTexturePtr& Walter::outerRefractionIndex() const
{
	return outerRefractionIndex_;
}



void Walter::setOuterRefractionIndex(const TTexturePtr& outerRefractionIndex)
{
	outerRefractionIndex_ = outerRefractionIndex;
}



const TTexturePtr& Walter::reflectance() const
{
	return reflectance_;
}



void Walter::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



const TTexturePtr& Walter::transmittance() const
{
	return transmittance_;
}



void Walter::setTransmittance(const TTexturePtr& transmittance)
{
	transmittance_ = transmittance;
}



const TTexturePtr& Walter::roughnessU() const
{
	return roughnessU_;
}



void Walter::setRoughnessU(const TTexturePtr& roughness)
{
	roughnessU_ = roughness;
}



const TTexturePtr& Walter::roughnessV() const
{
	return roughnessV_;
}



void Walter::setRoughnessV(const TTexturePtr& roughness)
{
	roughnessV_ = roughness;
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
	return TBsdfPtr(new Bsdf(sample, context, reflectance, transmittance, etaI, etaT, mu, mv));
}



const TPyObjectPtr Walter::doGetState() const
{
	return python::makeTuple(innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_, roughnessU_, roughnessV_, numberOfSamples_);
}



void Walter::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, innerRefractionIndex_, outerRefractionIndex_, reflectance_, transmittance_, roughnessU_, roughnessV_, numberOfSamples_);
}



Walter::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& reflectance, const Spectral& transmittance, TValue etaI, TValue etaT, TValue mU, TValue mV):
	kernel::Bsdf(sample, context, BsdfCaps::transmission | BsdfCaps::glossy),
	reflectance_(reflectance),
	transmittance_(transmittance),
	etaI_(etaI),
	etaT_(etaT),
	mU_(mU),
	mV_(mV)
{
}



namespace
{

using TValue = Spectral::TValue;

// An Overview of BRDF Models, Rosana Montesand Carlos Ureña, Technical Report LSI-2012-001
// Dept. Lenguajes y Sistemas Informáticos University of Granada, Granada, Spain

// BECKMANN P., SPIZZICHINO A.: The Scattering of Electromagnetic Waves from Rough Surfaces.
// Pergamon Press, New York, 1963. Reprinted in 1987 by Artech House Publishers, Norwood, Massachusetts

inline TValue D_beckmann(const TVector3D& h, TValue mU, TValue mV)
{
	// original isotropic Beckmann distribution
	// D = 1 / (m^2 cos^4 theta) * exp -{ (tan^2 theta) / m^2 }
	//
	const TValue cosTheta2 = num::sqr(static_cast<TValue>(h.z));
	if (cosTheta2 == 0)
		return 0;
	const TValue sinTheta2 = std::max<TValue>(1 - cosTheta2, 0);
	const TValue sinTheta = num::sqrt(sinTheta2);
	const TValue cosPhi2 = sinTheta > 0 ? num::sqr(static_cast<TValue>(h.x) / sinTheta) : TValue(0.5f);
	const TValue sinPhi2 = sinTheta > 0 ? num::sqr(static_cast<TValue>(h.y) / sinTheta) : TValue(0.5f);
	const TValue tanTheta2 = sinTheta2 / cosTheta2;
	return num::exp(-tanTheta2 * (cosPhi2 / num::sqr(mU) + sinPhi2 / num::sqr(mV))) / (num::NumTraits<TValue>::pi * mU * mV * cosTheta2 * cosTheta2);
}


inline TVector3D sampleD_beckmann(const TPoint2D& sample, TScalar mU, TScalar mV)
{
	LASS_ASSERT(sample.x < TNumTraits::one);
	const TScalar s = num::log(TNumTraits::one - sample.x);
	LASS_ASSERT(!num::isInf(s));

	TScalar cosPhi;
	TScalar sinPhi;
	TScalar tanTheta2;
	if (mU == mV)
	{
		const TScalar phi = 2 * TNumTraits::pi * sample.y;
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s * num::sqr(mU);
	}
	else
	{
		TScalar phi = std::atan(mV / mU * num::tan(2 * TNumTraits::pi * sample.y + TNumTraits::pi / 2));
		if (sample.y > 0.5f) 
			phi += TNumTraits::pi;
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s / (num::sqr(cosPhi / mU) + num::sqr(sinPhi / mV));
	}

	const TScalar cosTheta2 = num::inv(1 + tanTheta2);
	const TScalar cosTheta = num::sqrt(cosTheta2);
	const TScalar sinTheta = num::sqrt(std::max(1 - cosTheta2, TNumTraits::zero));

	return TVector3D(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}


inline TValue G(const TVector3D& omegaIn, const TVector3D& omegaOut, const TVector3D& h)
{
	const TValue g1 = std::max<TValue>(0, 2 * h.z * omegaIn.z / dot(omegaIn, h));
	const TValue g2 = std::max<TValue>(0, 2 * h.z * omegaOut.z / dot(omegaOut, h));
	return std::min<TValue>(1, std::min(g1, g2));
}


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

	LASS_ASSERT(omegaIn.z > 0);
	if (omegaOut.z > 0)
	{
		const TVector3D h = (omegaIn + omegaOut).normal();

		const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
		const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);

		const TValue d = D_beckmann(h, mU_, mV_); // == pdfD
		const TValue g = G(omegaIn, omegaOut, h);
		const TScalar pdf = d / (4 * dot(omegaIn, h));

		BsdfOut out(reflectance_, pdf * pdfRefl);
		out.value *= rFresnel * d * g / static_cast<TValue>(4 * omegaIn.z * omegaOut.z);
		return out;
	}
	else if (omegaOut.z < 0)
	{
		const TVector3D h = (-etaI_ * omegaIn - etaT_ * omegaOut).normal();
		const TValue cosThetaI = static_cast<TValue>(num::abs(dot(omegaIn, h)));
		const TValue cosThetaT = static_cast<TValue>(num::abs(dot(omegaOut, h)));

		const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
		const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);

		const TValue d = D_beckmann(h, mU_, mV_); // == pdfD
		const TValue g = G(omegaIn, omegaOut, h);
		const TValue dh_dwo = num::sqr(etaT_) * cosThetaT / num::sqr(etaI_ * cosThetaI + etaT_ * cosThetaT);
		const TScalar pdf = d * dh_dwo;

		BsdfOut out(transmittance_, pdf * (1 - pdfRefl));
		out.value *= (cosThetaI * (1 - rFresnel) * g * d * dh_dwo) / static_cast<TValue>(num::abs(omegaIn.z * omegaOut.z));

		// if (adjoint)
		//	out.value *= num::sqr(etaI_) / num::sqr(etaT_)
		return out;
	}
	return BsdfOut();
}



SampleBsdfOut Walter::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const TVector3D h = sampleD_beckmann(sample, mU_, mV_);
	const TValue d = D_beckmann(h, mU_, mV_); // == pdfD
	LASS_ASSERT(h.z > 0);

	const TValue rFresnel = fresnelDielectric(etaI_ / etaT_, omegaIn, h);
	const TValue pdfRefl = pdfReflection(rFresnel, allowedCaps);

	if (componentSample < pdfRefl)
	{
		const TVector3D omegaOut = h.reflect(omegaIn);
		if (omegaOut.z <= 0)
		{
			return SampleBsdfOut();
		}
		const TValue g = G(omegaIn, omegaOut, h);
		const TScalar pdf = d / (4 * dot(omegaIn, h));
		SampleBsdfOut out(omegaOut, reflectance_, pdf * pdfRefl, BsdfCaps::reflection | BsdfCaps::glossy);
		out.value *= rFresnel * d * g / static_cast<TValue>(4 * omegaIn.z * omegaOut.z);
		return out;
	}
	else
	{
		const TValue eta = etaI_ / etaT_;
		const TScalar cosThetaI = num::abs(dot(omegaIn, h));
		const TScalar sinTheta2 = num::sqr(eta) * (TNumTraits::one - num::sqr(cosThetaI));
		const TScalar cosThetaT = num::sqrt(std::max(TNumTraits::one - sinTheta2, TNumTraits::zero));
		// const TVector3D omegaOut = ((eta * cosThetaI - num::sqrt(std::max<TScalar>(0, 1 + eta * (num::sqr(cosThetaI) - 1))) * h) - eta * omegaIn).normal();
		const TVector3D omegaOut = (-eta * omegaIn + (eta * cosThetaI - cosThetaT) * h).normal();
		if (omegaOut.z >= 0)
		{
			return SampleBsdfOut();
		}
		const TValue g = G(omegaIn, omegaOut, h);
		const TValue dh_dwo = num::sqr(etaT_) * cosThetaT / num::sqr(etaI_ * cosThetaI + etaT_ * cosThetaT);
		const TScalar pdf = d * dh_dwo;
		SampleBsdfOut out(omegaOut, transmittance_, pdf * (1 - pdfRefl), BsdfCaps::transmission | BsdfCaps::glossy);
		out.value *= (cosThetaI * (1 - rFresnel) * g * d * dh_dwo) / static_cast<TValue>(num::abs(omegaIn.z * omegaOut.z));
		return out;
	}
}


TValue Walter::Bsdf::pdfReflection(TValue rFresnel, BsdfCaps allowedCaps) const
{
	const TValue powRefl = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? reflectance_.absAverage() * rFresnel : 0;
	const TValue powTrans = kernel::hasCaps(allowedCaps, BsdfCaps::transmission | BsdfCaps::glossy) ? transmittance_.absAverage() * (1 - rFresnel) : 0;
	LASS_ASSERT(powRefl + powTrans > 0);
	return powRefl / (powRefl + powTrans);
}

// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
