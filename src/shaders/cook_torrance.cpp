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
#include "cook_torrance.h"
#include "../kernel/ray_tracer.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(CookTorrance, "Anisotropic Phong BRDF by Ashikhmin & Shirley (2001)")
PY_CLASS_CONSTRUCTOR_0(CookTorrance)
PY_CLASS_CONSTRUCTOR_2(CookTorrance, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(CookTorrance, refractionIndex, setRefractionIndex, "texture for refraction index")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, absorptionCoefficient, setAbsorptionCoefficient, "texture for absorption coefficient")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, reflectance, setReflectance, "texture for additional reflectance multiplier")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, roughnessU, setRoughnessU, "texture for the rms microfacet slope in U direction")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, roughnessV, setRoughnessV, "texture for the rms microfacet slope in V direction")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, numberOfSamples, setNumberOfSamples, "set number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

CookTorrance::CookTorrance():
	CookTorrance(Texture::white(), Texture::white())
{
}



CookTorrance::CookTorrance(const TTexturePtr& refractionIndex):
	CookTorrance(refractionIndex, Texture::white())
{
}



CookTorrance::CookTorrance(const TTexturePtr& refractionIndex, const TTexturePtr& absorptionCoefficient):
	Shader(BsdfCaps::reflection | BsdfCaps::glossy),
	refractionIndex_(refractionIndex),
	absorptionCoefficient_(absorptionCoefficient),
	reflectance_(Texture::white()),
	roughnessU_(Texture::black()),
	roughnessV_(Texture::black()),
	numberOfSamples_(9)
{
}



const TTexturePtr& CookTorrance::refractionIndex() const
{
	return refractionIndex_;
}



void CookTorrance::setRefractionIndex(const TTexturePtr& refractionIndex)
{
	refractionIndex_ = refractionIndex;
}



const TTexturePtr& CookTorrance::absorptionCoefficient() const
{
	return absorptionCoefficient_;
}



void CookTorrance::setAbsorptionCoefficient(const TTexturePtr& absorptionCoefficient)
{
	absorptionCoefficient_ = absorptionCoefficient;
}



const TTexturePtr& CookTorrance::reflectance() const
{
	return reflectance_;
}



void CookTorrance::setReflectance(const TTexturePtr& reflectance)
{
	reflectance_ = reflectance;
}



const TTexturePtr& CookTorrance::roughnessU() const
{
	return roughnessU_;
}



void CookTorrance::setRoughnessU(const TTexturePtr& roughness)
{
	roughnessU_ = roughness;
}



const TTexturePtr& CookTorrance::roughnessV() const
{
	return roughnessV_;
}



void CookTorrance::setRoughnessV(const TTexturePtr& roughness)
{
	roughnessV_ = roughness;
}



size_t CookTorrance::numberOfSamples() const
{
	return numberOfSamples_;
}



void CookTorrance::setNumberOfSamples(size_t number)
{
	numberOfSamples_ = std::max<size_t>(number, 1);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

size_t CookTorrance::doNumReflectionSamples() const
{
	return numberOfSamples_;
}



TBsdfPtr CookTorrance::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	typedef Spectral::TValue TValue;
	// in theory, refractive indices must be at least 1, but they must be more than zero for sure.
	// IOR as average of spectral works correctly for LIAR_SPECTRAL_MODE_SINGLE. Everything else uses ... well, and average.
	const Spectral eta = max(refractionIndex_->lookUp(sample, context, Illuminant), 1e-9f);
	const Spectral kappa = max(absorptionCoefficient_->lookUp(sample, context, Illuminant), 1e-9f);
	const Spectral reflectance = reflectance_->lookUp(sample, context, Reflectant);
    const TValue mu = std::max<TValue>(roughnessU_->scalarLookUp(sample, context), 1e-3f);
	const TValue mv = std::max<TValue>(roughnessV_->scalarLookUp(sample, context), 1e-3f);
	return TBsdfPtr(new Bsdf(sample, context, eta, kappa, reflectance, mu, mv));
}



const TPyObjectPtr CookTorrance::doGetState() const
{
	return python::makeTuple(refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



void CookTorrance::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



CookTorrance::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance, TValue mU, TValue mV):
	kernel::Bsdf(sample, context, BsdfCaps::reflection | BsdfCaps::glossy),
	eta_(eta),
	kappa_(kappa),
	reflectance_(reflectance),
	mU_(mU),
	mV_(mV)
{
}



namespace
{

// An Overview of BRDF Models, Rosana Montesand Carlos Ureña, Technical Report LSI-2012-001
// Dept. Lenguajes y Sistemas Informáticos University of Granada, Granada, Spain

// BECKMANN P., SPIZZICHINO A.: The Scattering of Electromagnetic Waves from Rough Surfaces.
// Pergamon Press, New York, 1963. Reprinted in 1987 by Artech House Publishers, Norwood, Massachusetts

inline TScalar D_beckmann(const TVector3D& h, TScalar mU, TScalar mV)
{
	// original isotropic Beckmann distribution
	// D = 1 / (m^2 cos^4 theta) * exp -{ (tan^2 theta) / m^2 }
	//
	const TScalar cosTheta2 = num::sqr(h.z);
	if (cosTheta2 == TNumTraits::zero)
		return TNumTraits::zero;
	const TScalar sinTheta2 = std::max(TNumTraits::one - cosTheta2, TNumTraits::zero);
	const TScalar sinTheta = num::sqrt(sinTheta2);
	const TScalar cosPhi2 = sinTheta > 0 ? num::sqr(h.x / sinTheta) : TScalar(0.5f);
	const TScalar sinPhi2 = sinTheta > 0 ? num::sqr(h.y / sinTheta) : TScalar(0.5f);
	const TScalar tanTheta2 = sinTheta2 / cosTheta2;
	return num::exp(-tanTheta2 * (cosPhi2 / num::sqr(mU) + sinPhi2 / num::sqr(mV))) / (TNumTraits::pi * mU * mV * cosTheta2 * cosTheta2);
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


inline TScalar G(const TVector3D& omegaIn, const TVector3D& omegaOut, const TVector3D& h)
{
	const TScalar dotOmegaH = num::abs(dot(omegaIn, h));
	return std::min(TNumTraits::one, std::min(2 * h.z * omegaIn.z / dotOmegaH, 2 * h.z * omegaOut.z / dotOmegaH));
}


inline Spectral fresnelConductor(const Spectral& eta, const Spectral& kappa, const TVector3D& omegaIn, const TVector3D& h)
{
	using TValue = Spectral::TValue;
	const TValue cosI = num::abs(static_cast<TValue>(dot(omegaIn, h)));
	LASS_ASSERT(cosI > 0);
	const TValue cosI2 = num::sqr(cosI);
	const Spectral a = sqr(eta) + sqr(kappa);
	const Spectral b = (2 * cosI) * eta;

	const Spectral aOrth = a + cosI2;
	const Spectral aPar = a * cosI2 + 1;

	const Spectral rOrth2 = (aOrth - b) / (aOrth + b);
	const Spectral rPar2 = (aPar - b) / (aPar + b);
	return (rOrth2 + rPar2) / 2;
}

}



BsdfOut CookTorrance::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps())); 

	LASS_ASSERT(omegaIn.z > 0);
	if (omegaOut.z <= 0)
	{
		return BsdfOut();
	}
	const TVector3D h = (omegaIn + omegaOut).normal();
	const TScalar d = D_beckmann(h, mU_, mV_); // == pdfD
	const TScalar g = G(omegaIn, omegaOut, h);
	const TScalar pdf = d / (4 * dot(omegaIn, h));
	BsdfOut out(reflectance_, pdf);
	out.value *= static_cast<Spectral::TValue>(d * g / (4 * omegaIn.z * omegaOut.z));
	out.value *= fresnelConductor(eta_, kappa_, omegaIn, h);
	return out;
}



SampleBsdfOut CookTorrance::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar /*componentSample*/, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const TVector3D h = sampleD_beckmann(sample, mU_, mV_);
	const TVector3D omegaOut = h.reflect(omegaIn);
	if (omegaOut.z <= 0)
	{
		return SampleBsdfOut();
	}	
	return SampleBsdfOut(omegaOut, doEvaluate(omegaIn, omegaOut, allowedCaps), BsdfCaps::reflection | BsdfCaps::glossy);
}


// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
