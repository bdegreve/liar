/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
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
#include "oren_nayar.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS_DOC(OrenNayar,
	"Diffuse Microfacet BRDF by Oren & Nayar (1994)\n"
	"\n"
	"OrenNayar(diffuse, sigma)"
)
PY_CLASS_CONSTRUCTOR_0(OrenNayar)
PY_CLASS_CONSTRUCTOR_2(OrenNayar, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(OrenNayar, diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(OrenNayar, sigma, setSigma, "texture for standard deviation of slope angle (in radians)")

// --- public --------------------------------------------------------------------------------------

OrenNayar::OrenNayar():
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(Texture::white()),
	sigma_(Texture::black())
{
}



OrenNayar::OrenNayar(const TTexturePtr& diffuse, const TTexturePtr& sigma):
	Shader(BsdfCaps::reflection | BsdfCaps::diffuse),
	diffuse_(diffuse),
	sigma_(sigma)
{
}



const TTexturePtr& OrenNayar::diffuse() const
{
	return diffuse_;
}



void OrenNayar::setDiffuse(const TTexturePtr& diffuse)
{
	diffuse_ = diffuse;
}



const TTexturePtr& OrenNayar::sigma() const
{
	return sigma_;
}



void OrenNayar::setSigma(const TTexturePtr& sigma)
{
	sigma_ = sigma;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

TBsdfPtr OrenNayar::doBsdf(const Sample& sample, const IntersectionContext& context) const
{
	using TValue = Spectral::TValue;
	const Spectral diffuse = diffuse_->lookUp(sample, context, SpectralType::Reflectant);
	const TValue sigma2 = num::sqr(std::max(sigma_->scalarLookUp(sample, context), 0.f));
	const TValue a = 1 - 0.5f * sigma2 / (sigma2 + 0.33f);
	const TValue b = 0.45f * sigma2 / (sigma2 + 0.09f);
	return TBsdfPtr(new OrenNayar::Bsdf(sample, context, caps(), diffuse, a, b));
}



const TPyObjectPtr OrenNayar::doGetState() const
{
	return python::makeTuple(diffuse_, sigma_);
}



void OrenNayar::doSetState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, diffuse_, sigma_);
}


// --- bsdf ----------------------------------------------------------------------------------------

OrenNayar::Bsdf::Bsdf(const Sample& sample, const IntersectionContext& context, BsdfCaps caps, const Spectral& diffuse, TValue a, TValue b):
	kernel::Bsdf(sample, context, caps),
	diffuseOverPi_(diffuse / num::NumTraits<Spectral::TValue>::pi),
	a_(a),
	b_(b)
{
}



BsdfOut OrenNayar::Bsdf::doEvaluate(const TVector3D& omegaIn, const TVector3D& omegaOut, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	const TScalar cosTheta = num::abs(omegaOut.z);
	if (cosTheta <= 0)
	{
		return BsdfOut();
	}
	const Spectral value = eval(omegaIn, omegaOut);
	const TScalar pdf = cosTheta / TNumTraits::pi;
	return BsdfOut(value, pdf);
}



SampleBsdfOut OrenNayar::Bsdf::doSample(const TVector3D& omegaIn, const TPoint2D& sample, TScalar, BsdfCaps LASS_UNUSED(allowedCaps)) const
{
	LASS_ASSERT(shaders::hasCaps(allowedCaps, caps()));
	TScalar pdf;
	const TVector3D omegaOut = num::cosineHemisphere(sample, pdf).position();
	const Spectral value = eval(omegaIn, omegaOut);
	return SampleBsdfOut(omegaOut, value, pdf, caps());
}



Spectral OrenNayar::Bsdf::eval(const TVector3D& omegaIn, const TVector3D& omegaOut) const
{
	const TValue cosThetaIn = static_cast<TValue>(omegaIn.z);
	const TValue cosThetaOut = static_cast<TValue>(std::abs(omegaOut.z));
	const TValue sinThetaIn2 = std::max(TValue(0), 1 - num::sqr(cosThetaIn));
	const TValue sinThetaOut2 = std::max(TValue(0), 1 - num::sqr(cosThetaOut));

	const TValue squaredNorm = sinThetaIn2 * sinThetaOut2;
	const TValue cosPhi = squaredNorm > 0
		? std::max(TValue(0), static_cast<TValue>(omegaIn.x * omegaOut.x + omegaIn.y * omegaOut.y) / num::sqrt(squaredNorm))
		: TValue(0);

	// using the observation that sinAlpha * sinBeta == sinThetaIn * sinThetaOut:
	const TValue sinAlphaTanBeta = num::sqrt(sinThetaIn2 * sinThetaOut2) / std::max(cosThetaIn, cosThetaOut);

	return diffuseOverPi_ * (a_ + b_ * cosPhi * sinAlphaTanBeta);
}



// --- free ----------------------------------------------------------------------------------------




}

}

// EOF
