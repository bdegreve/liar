/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2023  Bram de Greve (bramz@users.sourceforge.net)
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

PY_DECLARE_CLASS_DOC(CookTorrance,
	"Anisotropic Microfacet Conductor BRDF by Cook & Torrance (1981)\n"
	"\n"
	"with Beckmann distribution and V-cavity shadow-masking\n"
	"Roughness uses the linearized Disney definition, with alpha = sqr(roughness)")
PY_CLASS_CONSTRUCTOR_0(CookTorrance)
PY_CLASS_CONSTRUCTOR_2(CookTorrance, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(CookTorrance, refractionIndex, setRefractionIndex, "refraction index")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, absorptionCoefficient, setAbsorptionCoefficient, "absorption coefficient")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, reflectance, setReflectance, "additional reflectance multiplier")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, roughnessU, setRoughnessU, "linear roughness [0-1] in U direction: alphaU = roughnesU ** 2")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, roughnessV, setRoughnessV, "linear roughness [0-1] in V direction: alphaV = roughnesV ** 2")
PY_CLASS_MEMBER_RW_DOC(CookTorrance, numberOfSamples, setNumberOfSamples, "number of samples for Monte Carlo simulations")

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
	numberOfSamples_(1)
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
	const Spectral eta = max(refractionIndex_->lookUp(sample, context, SpectralType::Illuminant), 1e-9f);
	const Spectral kappa = max(absorptionCoefficient_->lookUp(sample, context, SpectralType::Illuminant), 1e-9f);
	const Spectral reflectance = reflectance_->lookUp(sample, context, SpectralType::Reflectant);
	const TValue alphaU = num::sqr(std::max<TValue>(roughnessU_->scalarLookUp(sample, context), 1e-3f));
	const TValue alphaV = num::sqr(std::max<TValue>(roughnessV_->scalarLookUp(sample, context), 1e-3f));
	return TBsdfPtr(new Bsdf(sample, context, eta, kappa, reflectance, alphaU, alphaV));
}



const TPyObjectPtr CookTorrance::doGetState() const
{
	return python::makeTuple(refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



void CookTorrance::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



CookTorrance::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance, TValue alphaU, TValue alphaV):
	kernel::Bsdf(sample, context, BsdfCaps::reflection | BsdfCaps::glossy),
	eta_(eta),
	kappa_(kappa),
	reflectance_(reflectance),
	alphaU_(alphaU),
	alphaV_(alphaV)
{
}



namespace
{

// An Overview of BRDF Models, Rosana Montesand Carlos Ureña, Technical Report LSI-2012-001
// Dept. Lenguajes y Sistemas Informáticos University of Granada, Granada, Spain

// BECKMANN P., SPIZZICHINO A.: The Scattering of Electromagnetic Waves from Rough Surfaces.
// Pergamon Press, New York, 1963. Reprinted in 1987 by Artech House Publishers, Norwood, Massachusetts

inline TScalar D_beckmann(const TVector3D& h, TScalar alphaU, TScalar alphaV)
{
	// original isotropic Beckmann distribution
	// D = 1 / (m^2 cos^4 theta) * exp -{ (tan^2 theta) / m^2 }
	//
	// anisotropic roughness: 1/m^2 -> cos^2 phi / m_x^2 + sin^2 phi / m_y^2
	// and cos^2 phi = h_x^2 / sin^2 theta

	const TScalar cosTheta2 = num::sqr(h.z);
	if (cosTheta2 == TNumTraits::zero)
		return TNumTraits::zero;
	return num::exp(-(num::sqr(h.x / alphaU) + num::sqr(h.y / alphaV)) / cosTheta2)
		/ (TNumTraits::pi * alphaU * alphaV * num::sqr(cosTheta2));
}


inline TVector3D sampleD_beckmann(const TPoint2D& sample, TScalar alphaU, TScalar alphaV)
{
	LASS_ASSERT(sample.x < TNumTraits::one);
	const TScalar s = num::log(TNumTraits::one - sample.x);
	LASS_ASSERT(!num::isInf(s));

	TScalar cosPhi;
	TScalar sinPhi;
	TScalar tanTheta2;
	if (alphaU == alphaV)
	{
		const TScalar phi = 2 * TNumTraits::pi * sample.y;
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s * num::sqr(alphaU);
	}
	else
	{
		TScalar phi = std::atan(alphaV / alphaU * num::tan(2 * TNumTraits::pi * sample.y + TNumTraits::pi / 2));
		if (sample.y > 0.5f)
			phi += TNumTraits::pi;
		cosPhi = num::cos(phi);
		sinPhi = num::sin(phi);
		tanTheta2 = -s / (num::sqr(cosPhi / alphaU) + num::sqr(sinPhi / alphaV));
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
	const TScalar d = D_beckmann(h, alphaU_, alphaV_); // == pdfD
	const TScalar g = G(omegaIn, omegaOut, h);
	const TScalar pdf = d * h.z / (4 * dot(omegaIn, h));
	BsdfOut out(reflectance_, pdf);
	out.value *= static_cast<Spectral::TValue>(d * g / (4 * omegaIn.z * omegaOut.z));
	out.value *= fresnelConductor(eta_, kappa_, omegaIn, h);
	return out;
}



SampleBsdfOut CookTorrance::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar /*componentSample*/, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const TVector3D h = sampleD_beckmann(sample, alphaU_, alphaV_);
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
