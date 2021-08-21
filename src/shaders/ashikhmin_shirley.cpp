/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(AshikhminShirley, "Anisotropic Phong BRDF by Ashikhmin & Shirley (2001)")
PY_CLASS_CONSTRUCTOR_0(AshikhminShirley)
PY_CLASS_CONSTRUCTOR_2(AshikhminShirley, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specular, setSpecular, "texture for specular component")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, roughnessU, setRoughnessU, "texture for roughness of specular component in U direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, roughnessV, setRoughnessV, "texture for roughness of specular component in V direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerU, setSpecularPowerU, "texture for rollof power of specular component in U direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerV, setSpecularPowerV, "texture for rollof power of specular component in V direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, numberOfSamples, setNumberOfSamples, "set number of samples for Monte Carlo simulations")

using PowerFromRoughness = AshikhminShirley::PowerFromRoughness;
PY_DECLARE_CLASS_NAME(PowerFromRoughness, "PowerFromRoughness");
PY_CLASS_INNER_CLASS_NAME(AshikhminShirley, PowerFromRoughness, "PowerFromRoughness");
PY_CLASS_CONSTRUCTOR_1(PowerFromRoughness, const TTexturePtr&)
PY_CLASS_MEMBER_RW(PowerFromRoughness, roughness, setRoughness)

using RoughnessFromPower = AshikhminShirley::RoughnessFromPower;
PY_DECLARE_CLASS_NAME(RoughnessFromPower, "RoughnessFromPower");
PY_CLASS_INNER_CLASS_NAME(AshikhminShirley, RoughnessFromPower, "RoughnessFromPower");
PY_CLASS_CONSTRUCTOR_1(RoughnessFromPower, const TTexturePtr&)
PY_CLASS_MEMBER_RW(RoughnessFromPower, power, setPower)

// --- public --------------------------------------------------------------------------------------

AshikhminShirley::AshikhminShirley():
	AshikhminShirley(Texture::white(), Texture::white())
{
}



AshikhminShirley::AshikhminShirley(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular):
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse | BsdfCaps::glossy),
	diffuse_(iDiffuse),
	specular_(iSpecular),
	numberOfSamples_(9)
{
	setRoughnessU(Texture::black());
	setRoughnessV(Texture::black());
}



const TTexturePtr& AshikhminShirley::diffuse() const
{
	return diffuse_;
}



void AshikhminShirley::setDiffuse(const TTexturePtr& diffuse)
{
	diffuse_ = diffuse;
}



const TTexturePtr& AshikhminShirley::specular() const
{
	return specular_;
}



void AshikhminShirley::setSpecular(const TTexturePtr& specular)
{
	specular_ = specular;
}



const TTexturePtr& AshikhminShirley::roughnessU() const
{
	return roughnessU_;
}



void AshikhminShirley::setRoughnessU(const TTexturePtr& roughnessU)
{
	roughnessU_ = roughnessU;
	if (RoughnessFromPower* p = dynamic_cast<RoughnessFromPower*>(roughnessU.get()))
	{
		specularPowerU_ = p->power();
	}
	else
	{
		specularPowerU_.reset(new PowerFromRoughness(roughnessU));
	}
}



const TTexturePtr& AshikhminShirley::roughnessV() const
{
	return roughnessV_;
}



void AshikhminShirley::setRoughnessV(const TTexturePtr& roughnessV)
{
	roughnessV_ = roughnessV;
	if (auto r = dynamic_cast<RoughnessFromPower*>(roughnessV.get()))
	{
		specularPowerV_ = r->power();
	}
	else
	{
		specularPowerV_.reset(new PowerFromRoughness(roughnessV));
	}
}



const TTexturePtr& AshikhminShirley::specularPowerU() const
{
	return specularPowerU_;
}



void AshikhminShirley::setSpecularPowerU(const TTexturePtr& specularPower)
{
	specularPowerU_ = specularPower;
	if (auto p = dynamic_cast<PowerFromRoughness*>(specularPower.get()))
	{
		roughnessU_ = p->roughness();
	}
	else
	{
		roughnessU_.reset(new RoughnessFromPower(specularPower));
	}
}



const TTexturePtr& AshikhminShirley::specularPowerV() const
{
	return specularPowerV_;
}



void AshikhminShirley::setSpecularPowerV(const TTexturePtr& specularPower)
{
	specularPowerV_ = specularPower;
	if (auto p = dynamic_cast<PowerFromRoughness*>(specularPower.get()))
	{
		roughnessV_ = p->roughness();
	}
	else
	{
		roughnessV_.reset(new RoughnessFromPower(specularPower));
	}
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
	const Spectral Rd = diffuse_->lookUp(sample, context, Reflectant);
	const Spectral Rs = specular_->lookUp(sample, context, Reflectant);
	const TValue nu = std::max<TValue>(specularPowerU_->scalarLookUp(sample, context), 0);
	const TValue nv = std::max<TValue>(specularPowerV_->scalarLookUp(sample, context), 0);
	return TBsdfPtr(new Bsdf(sample, context, Rd, Rs, nu, nv));
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



AshikhminShirley::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& diffuse, const Spectral& specular, TScalar powerU, TScalar powerV) :
	kernel::Bsdf(sample, context, BsdfCaps::reflection | BsdfCaps::diffuse | BsdfCaps::glossy),
	diffuse_(diffuse),
	specular_(specular),
	powerU_(powerU),
	powerV_(powerV)
{
}



BsdfOut AshikhminShirley::Bsdf::doEvaluate(const TVector3D& k1, const TVector3D& k2, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(k1.z > 0);
	if (k2.z <= 0)
	{
		return BsdfOut();
	}
	const TScalar pd = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::diffuse) ? diffuse_.absAverage() : 0;
	const TScalar ps = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? specular_.absAverage() : 0;
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
	LASS_ASSERT(k1.z > 0);
	TScalar pd = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::diffuse) ? diffuse_.absAverage() : 0;
	TScalar ps = kernel::hasCaps(allowedCaps, BsdfCaps::reflection | BsdfCaps::glossy) ? specular_.absAverage() : 0;
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
		LASS_ASSERT(ps > 0);
		const TVector3D h = sampleH(sample);
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
	return diffuse_ * (1 - specular_) * static_cast<Spectral::TValue>(a * b * 28.f / (23.f * TNumTraits::pi));
}



const Spectral AshikhminShirley::Bsdf::rhoS(const TVector3D& k1, const TVector3D& k2, const TVector3D& h, TScalar& pdf) const
{
	const TScalar c = num::sqrt((powerU_ + 1) * (powerV_ + 1)) / (8 * TNumTraits::pi);
	const TScalar n = powerU_ * num::sqr(h.x) + powerV_ * num::sqr(h.y);
	const TScalar nn = h.z == 1 ? 0 : n / (1 - num::sqr(h.z));
	const TScalar hk = dot(h, k1);
	const Spectral F = specular_ + (1 - specular_) * static_cast<Spectral::TValue>(temp::pow5(1 - hk));
	const TScalar pdfH = 4 * c * num::pow(h.z, n);
	pdf = pdfH / (4 * hk);
	return F * static_cast<Spectral::TValue>((c * num::pow(h.z, nn) / (hk * std::max(k1.z, k2.z))));
}



const TVector3D AshikhminShirley::Bsdf::sampleH(const TPoint2D& sample) const
{
	const TScalar f = num::sqrt((powerU_ + 1) / (powerV_ + 1));
	TScalar phi;
	if (sample.x < .5f)
	{
		if (sample.x < .25f)
		{
			phi = num::atan(f * num::tan(2 * TNumTraits::pi * sample.x));
		}
		else
		{
			phi = TNumTraits::pi - num::atan(f * num::tan(2 * TNumTraits::pi * (.5f - sample.x)));
		}
	}
	else
	{
		if (sample.x < .75f)
		{
			phi = TNumTraits::pi + num::atan(f * num::tan(2 * TNumTraits::pi * (sample.x - .5f)));
		}
		else
		{
			phi = 2 * TNumTraits::pi - num::atan(f * num::tan(2 * TNumTraits::pi * (1.f - sample.x)));
		}
	}

	const TScalar cosPhi = num::cos(phi);
	const TScalar sinPhi = num::sin(phi);
	const TScalar n = powerU_ * num::sqr(cosPhi) + powerV_ * num::sqr(sinPhi);

	const TScalar cosTheta = std::pow(1 - sample.y, num::inv(n + 1));
	const TScalar sinTheta = num::sqrt(std::max(TNumTraits::zero, 1 - num::sqr(cosTheta)));
	return TVector3D(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}



// --- PowerFromRoughness ----------------------------------------------------------------------------

AshikhminShirley::PowerFromRoughness::PowerFromRoughness(const TTexturePtr& roughness):
	roughness_(roughness)
{
}



const TTexturePtr& AshikhminShirley::PowerFromRoughness::roughness() const
{
	return roughness_;
}



void AshikhminShirley::PowerFromRoughness::setRoughness(const TTexturePtr& roughness)
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
	const Spectral r = max(roughness_->lookUp(sample, context, Illuminant), 1e-3f);
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

AshikhminShirley::RoughnessFromPower::RoughnessFromPower(const TTexturePtr& power):
	power_(power)
{
}



const TTexturePtr& AshikhminShirley::RoughnessFromPower::power() const
{
	return power_;
}



void AshikhminShirley::RoughnessFromPower::setPower(const TTexturePtr& power)
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
	const Spectral e = max(power_->lookUp(sample, context, Illuminant), 0.f);
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
