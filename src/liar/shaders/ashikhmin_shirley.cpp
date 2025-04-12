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
#include "ashikhmin_shirley.h"
#include "microfacet_blinn.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(AshikhminShirley, "Anisotropic Phong BRDF by Ashikhmin & Shirley (2001)")
PY_CLASS_CONSTRUCTOR_0(AshikhminShirley)
PY_CLASS_CONSTRUCTOR_2(AshikhminShirley, const TTextureRef&, const TTextureRef&)
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, diffuse, setDiffuse, "diffuse reflectance")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specular, setSpecular, "specular reflectance")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, roughnessU, setRoughnessU, "linear roughness [0-1] of specular component in U direction: alphaU = roughnessU ** 2")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, roughnessV, setRoughnessV, "linear roughness [0-1] of specular component in V direction: alphaV = roughnessV ** 2")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerU, setSpecularPowerU, "rollof power [0-inf] of specular component in U direction: powerU = max(2 / alphaU ** 2 - 2, 0)")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerV, setSpecularPowerV, "rollof power [0-inf] of specular component in V direction: powerV = max(2 / alphaV ** 2 - 2, 0)")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, mdf, setMdf, "microfacet distribution")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, numberOfSamples, setNumberOfSamples, "number of samples for Monte Carlo simulations")

using PowerFromRoughness = AshikhminShirley::PowerFromRoughness;
PY_DECLARE_CLASS_NAME(PowerFromRoughness, "PowerFromRoughness");
PY_CLASS_INNER_CLASS_NAME(AshikhminShirley, PowerFromRoughness, "PowerFromRoughness");
PY_CLASS_CONSTRUCTOR_1(PowerFromRoughness, const TTextureRef&)
PY_CLASS_MEMBER_RW(PowerFromRoughness, roughness, setRoughness)

using RoughnessFromPower = AshikhminShirley::RoughnessFromPower;
PY_DECLARE_CLASS_NAME(RoughnessFromPower, "RoughnessFromPower");
PY_CLASS_INNER_CLASS_NAME(AshikhminShirley, RoughnessFromPower, "RoughnessFromPower");
PY_CLASS_CONSTRUCTOR_1(RoughnessFromPower, const TTextureRef&)
PY_CLASS_MEMBER_RW(RoughnessFromPower, power, setPower)

// --- public --------------------------------------------------------------------------------------

AshikhminShirley::AshikhminShirley():
	AshikhminShirley(Texture::white(), Texture::white())
{
}



AshikhminShirley::AshikhminShirley(const TTextureRef& iDiffuse, const TTextureRef& iSpecular):
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse | BsdfCaps::glossy),
	diffuse_(iDiffuse),
	specular_(iSpecular),
	mdf_(MicrofacetBlinn::instance()),
	numberOfSamples_(1)
{
	setRoughnessU(Texture::black());
	setRoughnessV(Texture::black());
}



const TTextureRef& AshikhminShirley::diffuse() const
{
	return diffuse_;
}



void AshikhminShirley::setDiffuse(const TTextureRef& diffuse)
{
	diffuse_ = diffuse;
}



const TTextureRef& AshikhminShirley::specular() const
{
	return specular_;
}



void AshikhminShirley::setSpecular(const TTextureRef& specular)
{
	specular_ = specular;
}



const TTextureRef& AshikhminShirley::roughnessU() const
{
	return roughnessU_;
}



void AshikhminShirley::setRoughnessU(const TTextureRef& roughnessU)
{
	roughnessU_ = roughnessU;
	if (RoughnessFromPower* p = dynamic_cast<RoughnessFromPower*>(roughnessU.get()))
	{
		specularPowerU_ = p->power();
	}
	else
	{
		specularPowerU_ = TTextureRef(new PowerFromRoughness(roughnessU));
	}
}



const TTextureRef& AshikhminShirley::roughnessV() const
{
	return roughnessV_;
}



void AshikhminShirley::setRoughnessV(const TTextureRef& roughnessV)
{
	roughnessV_ = roughnessV;
	if (auto r = dynamic_cast<RoughnessFromPower*>(roughnessV.get()))
	{
		specularPowerV_ = r->power();
	}
	else
	{
		specularPowerV_ = TTextureRef(new PowerFromRoughness(roughnessV));
	}
}



const TTextureRef& AshikhminShirley::specularPowerU() const
{
	return specularPowerU_;
}



void AshikhminShirley::setSpecularPowerU(const TTextureRef& specularPower)
{
	specularPowerU_ = specularPower;
	if (auto p = dynamic_cast<PowerFromRoughness*>(specularPower.get()))
	{
		roughnessU_ = p->roughness();
	}
	else
	{
		roughnessU_ = TTextureRef(new RoughnessFromPower(specularPower));
	}
}



const TTextureRef& AshikhminShirley::specularPowerV() const
{
	return specularPowerV_;
}



void AshikhminShirley::setSpecularPowerV(const TTextureRef& specularPower)
{
	specularPowerV_ = specularPower;
	if (auto p = dynamic_cast<PowerFromRoughness*>(specularPower.get()))
	{
		roughnessV_ = p->roughness();
	}
	else
	{
		roughnessV_ = TTextureRef(new RoughnessFromPower(specularPower));
	}
}



const TMicrofacetDistributionRef& AshikhminShirley::mdf() const
{
	return mdf_;
}



void AshikhminShirley::setMdf(const TMicrofacetDistributionRef& mdf)
{
	mdf_ = mdf;
}


size_t AshikhminShirley::numberOfSamples() const
{
	return numberOfSamples_;
}



void AshikhminShirley::setNumberOfSamples(size_t number)
{
	numberOfSamples_ = std::max<size_t>(number, 1);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

namespace temp
{
	template <typename T>
	inline T pow5(const T& iX)
	{
		T temp = iX * iX;
		temp *= temp;
		temp *= iX;
		return temp;
	}
}

size_t AshikhminShirley::doNumReflectionSamples() const
{
	return numberOfSamples_;
}



TBsdfPtr AshikhminShirley::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	typedef Spectral::TValue TValue;
	const Spectral Rd = diffuse_->lookUp(sample, context, SpectralType::Reflectant);
	const Spectral Rs = specular_->lookUp(sample, context, SpectralType::Reflectant);
	const TValue alphaU = num::sqr(std::max<TValue>(roughnessU_->scalarLookUp(sample, context), 0));
	const TValue alphaV = num::sqr(std::max<TValue>(roughnessV_->scalarLookUp(sample, context), 0));
	return TBsdfPtr(new Bsdf(sample, context, Rd * (1 - Rs), Rs, mdf_.get(), alphaU, alphaV));
}


/*
namespace temp
{

inline TScalar balanceHeuristic(TScalar p1, TScalar p2)
{
	return p1 / (p1 + p2);
}

inline TScalar squareHeuristic(TScalar p1, TScalar p2)
{
	const TScalar a = num::sqr(p1);
	const TScalar b = num::sqr(p2);
	return balanceHeuristic(a, b);
}

inline TScalar powerHeuristic(TScalar p1, TScalar p2, TScalar beta)
{
	const TScalar a = num::pow(p1, beta);
	const TScalar b = num::pow(p2, beta);
	return balanceHeuristic(a, b);
}

inline TScalar maximumHeuristic(TScalar p1, TScalar p2)
{
	return p1 >= p2 ? 1.f : 0.f;
}

inline TScalar cutoffHeuristic(TScalar p1, TScalar p2, TScalar alpha)
{
	const TScalar a = p1 >= alpha * p2 ? p1 : 0;
	const TScalar b = p2 >= alpha * p1 ? p2 : 0;
	return balanceHeuristic(a, b);
}

}
*/


const TPyObjectPtr AshikhminShirley::doGetState() const
{
	return python::makeTuple(diffuse_, specular_, specularPowerU_, specularPowerV_, numberOfSamples_);
}



void AshikhminShirley::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, diffuse_, specular_, specularPowerU_, specularPowerV_, numberOfSamples_);
}



AshikhminShirley::Bsdf::Bsdf(
		const Sample& sample, const IntersectionContext& context, const Spectral& diffuse, const Spectral& specular,
		const MicrofacetDistribution* mdf, TValue alphaU, TValue alphaV) :
	kernel::Bsdf(sample, context, BsdfCaps::reflection | BsdfCaps::diffuse | BsdfCaps::glossy),
	diffuse_(diffuse),
	specular_(specular),
	mdf_(mdf),
	alphaU_(alphaU),
	alphaV_(alphaV)
{
}



BsdfOut AshikhminShirley::Bsdf::doEvaluate(const TVector3D& k1, const TVector3D& k2, BsdfCaps allowedCaps) const
{
	if (k1.z <= 0 || k2.z <= 0)
	{
		return BsdfOut();
	}
	const TScalar pd = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::diffuse) ? diffuse_.absAverage() : 0;
	const TScalar ps = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? (specular_.absAverage() + 1) / 2 : 0;
	LASS_ASSERT(pd >= 0 && ps >= 0);
	const TScalar ptot = pd + ps;

	BsdfOut out;
	if (ps > 0)
	{
		const TVector3D h = (k1 + k2).normal();
		out.value = rhoS(k1, k2, h, out.pdf);
		out.pdf *= ps / ptot;
	}
	if (pd > 0)
	{
		out.value += rhoD(k1, k2);
		out.pdf += pd * k2.z / (ptot * TNumTraits::pi);
	}
	return out;
}



SampleBsdfOut AshikhminShirley::Bsdf::doSample(const TVector3D& k1, const TPoint2D& sample, TScalar componentSample, BsdfCaps allowedCaps) const
{
	if (k1.z < 0)
	{
		return SampleBsdfOut();
	}
	TScalar pd = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::diffuse) ? diffuse_.absAverage() : 0;
	TScalar ps = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? (specular_.absAverage() + 1) / 2 : 0;
	const TScalar ptot = pd + ps;
	if (ptot <= 0)
	{
		return SampleBsdfOut();
	}
	pd /= ptot;
	ps /= ptot;

	SampleBsdfOut out;
	if (componentSample < pd)
	{
		out.omegaOut = num::cosineHemisphere(sample, out.pdf).position();
		out.pdf *= pd;
		out.value = rhoD(k1, out.omegaOut);
		out.usedCaps = BsdfCaps::reflection | BsdfCaps::diffuse;
	}
	else
	{
		LIAR_ASSERT(ps > 0, "ps=" << ps);
		const TVector3D h = mdf_->sampleH(sample, alphaU_, alphaV_);
		out.omegaOut = h.reflect(k1);
		if (out.omegaOut.z > 0)
		{
			out.value = rhoS(k1, out.omegaOut, h, out.pdf);
			out.pdf *= ps;
		}
		out.usedCaps = BsdfCaps::reflection | BsdfCaps::glossy;
	}
	return out;
}



const Spectral AshikhminShirley::Bsdf::rhoD(const TVector3D& k1, const TVector3D& k2) const
{
	const TScalar a = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k1.z / 2));
	const TScalar b = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k2.z / 2));
	return diffuse_ * static_cast<Spectral::TValue>(a * b * 28.f / (23.f * TNumTraits::pi));
}



const Spectral AshikhminShirley::Bsdf::rhoS(const TVector3D& k1, const TVector3D& k2, const TVector3D& h, TScalar& pdf) const
{
	using TValue = MicrofacetDistribution::TValue;
	const TScalar hk = dot(h, k1);
	LIAR_ASSERT(hk > 0 && hk <= TScalar(1.000001f), "hk=" << hk << " h=" << h << " k1=" << k1);
	const Spectral F = specular_ + (1 - specular_) * temp::pow5(std::max(static_cast<Spectral::TValue>(1 - hk), 0.f));
	TValue pdfH;
	const TValue D = mdf_->D(h, alphaU_, alphaV_, pdfH);
	pdf = pdfH / (4 * hk);
	LIAR_ASSERT_POSITIVE_FINITE(pdf);
	return F * static_cast<Spectral::TValue>(D / (4 * hk * std::max(k1.z, k2.z)));
}



// --- PowerFromRoughness ----------------------------------------------------------------------------

AshikhminShirley::PowerFromRoughness::PowerFromRoughness(const TTextureRef& roughness):
	roughness_(roughness)
{
}



const TTextureRef& AshikhminShirley::PowerFromRoughness::roughness() const
{
	return roughness_;
}



void AshikhminShirley::PowerFromRoughness::setRoughness(const TTextureRef& roughness)
{
	roughness_ = roughness;
}



const TPyObjectPtr AshikhminShirley::PowerFromRoughness::doGetState() const
{
	return python::makeTuple(roughness_);
}



void AshikhminShirley::PowerFromRoughness::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, roughness_);
}



const Spectral AshikhminShirley::PowerFromRoughness::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	const Spectral r = max(roughness_->lookUp(sample, context, SpectralType::Illuminant), 1e-3f);
	const Spectral a = sqr(r);
	return Spectral(max(2 / sqr(a) - 2, 0.f), type);
}



Spectral::TValue AshikhminShirley::PowerFromRoughness::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TValue r = std::max<TValue>(roughness_->scalarLookUp(sample, context), 1e-3f);
	const TValue a = num::sqr(r);
	return std::max<TValue>(2 / num::sqr(a) - 2, 0);
}



bool AshikhminShirley::PowerFromRoughness::doIsChromatic() const
{
	return roughness_->isChromatic();
}



// --- RoughnessFromPower ----------------------------------------------------------------------------

AshikhminShirley::RoughnessFromPower::RoughnessFromPower(const TTextureRef& power):
	power_(power)
{
}



const TTextureRef& AshikhminShirley::RoughnessFromPower::power() const
{
	return power_;
}



void AshikhminShirley::RoughnessFromPower::setPower(const TTextureRef& power)
{
	power_ = power;
}



const TPyObjectPtr AshikhminShirley::RoughnessFromPower::doGetState() const
{
	return python::makeTuple(power_);
}



void AshikhminShirley::RoughnessFromPower::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, power_);
}



const Spectral AshikhminShirley::RoughnessFromPower::doLookUp(const Sample& sample, const IntersectionContext& context, SpectralType type) const
{
	const Spectral e = max(power_->lookUp(sample, context, SpectralType::Illuminant), 0.f);
	const Spectral a = sqrt(2 / (e + 2));
	return Spectral(sqrt(a), type);
}



Spectral::TValue AshikhminShirley::RoughnessFromPower::doScalarLookUp(const Sample& sample, const IntersectionContext& context) const
{
	const TValue e = std::max<TValue>(power_->scalarLookUp(sample, context), 0.f);
	const TValue a = num::sqrt(2 / (e + 2));
	return num::sqrt(a);
}



bool AshikhminShirley::RoughnessFromPower::doIsChromatic() const
{
	return power_->isChromatic();
}


// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
