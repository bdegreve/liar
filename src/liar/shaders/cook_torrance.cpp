/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2024  Bram de Greve (bramz@users.sourceforge.net)
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
#include "microfacet_beckmann.h"
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
PY_CLASS_MEMBER_RW_DOC(CookTorrance, mdf, setMdf, "microfacet distribution")
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
	mdf_(MicrofacetBeckmann::instance()),
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



const TMicrofacetDistributionPtr& CookTorrance::mdf() const
{
	return mdf_;
}


void CookTorrance::setMdf(const TMicrofacetDistributionPtr& mdf)
{
	mdf_ = mdf;
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
	return TBsdfPtr(new Bsdf(sample, context, eta, kappa, reflectance, mdf_.get(), alphaU, alphaV));
}



const TPyObjectPtr CookTorrance::doGetState() const
{
	return python::makeTuple(refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



void CookTorrance::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, refractionIndex_, absorptionCoefficient_, reflectance_, roughnessU_, roughnessV_, numberOfSamples_);
}



CookTorrance::Bsdf::Bsdf(
		const Sample& sample, const IntersectionContext& context, const Spectral& eta, const Spectral& kappa, const Spectral& reflectance,
		const MicrofacetDistribution* mdf, TValue alphaU, TValue alphaV):
	kernel::Bsdf(sample, context, BsdfCaps::reflection | BsdfCaps::glossy),
	eta_(eta),
	kappa_(kappa),
	reflectance_(reflectance),
	mdf_(mdf),
	alphaU_(alphaU),
	alphaV_(alphaV)
{
}

namespace
{

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
	TValue pdfH;
	const TValue d = mdf_->D(h, alphaU_, alphaV_, pdfH);
	const TScalar g = G(omegaIn, omegaOut, h);
	const TScalar pdf = pdfH / (4 * dot(omegaIn, h));
	BsdfOut out(reflectance_, pdf);
	out.value *= static_cast<Spectral::TValue>(d * g / (4 * omegaIn.z * omegaOut.z));
	out.value *= fresnelConductor(eta_, kappa_, omegaIn, h);
	return out;
}



SampleBsdfOut CookTorrance::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar /*componentSample*/, BsdfCaps allowedCaps) const
{
	LASS_ASSERT(omegaIn.z > 0);
	const TVector3D h = mdf_->sampleH(sample, alphaU_, alphaV_);
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
