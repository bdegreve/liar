/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerU, setSpecularPowerU, "texture for rollof power of specular component in U direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, specularPowerV, setSpecularPowerV, "texture for rollof power of specular component in V direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, numberOfSamples, setNumberOfSamples, "set number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

AshikhminShirley::AshikhminShirley():
	Shader(Bsdf::capsReflection | Bsdf::capsDiffuse | Bsdf::capsGlossy),
	diffuse_(Texture::white()),
	specular_(Texture::white()),
	specularPowerU_(Texture::white()),
	specularPowerV_(Texture::white()),
	numberOfSamples_(9)
{
}



AshikhminShirley::AshikhminShirley(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular):
	Shader(Bsdf::capsReflection | Bsdf::capsDiffuse | Bsdf::capsGlossy),
	diffuse_(iDiffuse),
	specular_(iSpecular),
	specularPowerU_(Texture::white()),
	specularPowerV_(Texture::white()),
	numberOfSamples_(9)
{
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



const TTexturePtr& AshikhminShirley::specularPowerU() const
{
	return specularPowerU_;
}



void AshikhminShirley::setSpecularPowerU(const TTexturePtr& specularPower)
{
	specularPowerU_ = specularPower;
}



const TTexturePtr& AshikhminShirley::specularPowerV() const
{
	return specularPowerV_;
}



void AshikhminShirley::setSpecularPowerV(const TTexturePtr& specularPower)
{
	specularPowerV_ = specularPower;
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

void AshikhminShirley::doBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn, 
		const BsdfIn* first, const BsdfIn* last, BsdfOut* result) const
{
	const XYZ Rd = diffuse_->lookUp(sample, context);
	const XYZ Rs = specular_->lookUp(sample, context);
	const TScalar nu = positiveAverage(specularPowerU_->lookUp(sample, context));
	const TScalar nv = positiveAverage(specularPowerV_->lookUp(sample, context));
	const TScalar rd = positiveAverage(Rd);
	const TScalar rs = positiveAverage(Rs);

	const TVector3D& k1 = omegaIn;
	LASS_ASSERT(k1.z > 0);
	const TScalar a = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k1.z / 2));
	const XYZ rhoA = Rd * (1 - Rs) * (a * 28.f / (23.f * TNumTraits::pi));
	const TScalar c = num::sqrt((nu + 1) * (nv + 1)) / (8 * TNumTraits::pi);

	while (first != last)
	{
		const bool doDiffuse = kernel::hasCaps(first->allowedCaps, Bsdf::capsReflection | Bsdf::capsDiffuse);
		const bool doGlossy = kernel::hasCaps(first->allowedCaps, Bsdf::capsReflection | Bsdf::capsGlossy);
		const TScalar pd = doDiffuse ? rd : 0;
		const TScalar ps = doGlossy ? rs : 0;
		const TScalar ptot = pd + ps;

		const TVector3D& k2 = first->omegaOut;
		if (k2.z > 0 && ptot > 0)
		{
			if (doDiffuse)
			{
				const TScalar b = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k2.z / 2));
				result->value += rhoA * b;
				result->pdf += (pd * k2.z) / (TNumTraits::pi * ptot);
			}
			if (doGlossy)
			{
				const TVector3D h = (k1 + k2).normal();
				const TScalar n = nu * num::sqr(h.x) + nv * num::sqr(h.y);
				const TScalar nn = h.z == 1 ? 0 : n / (1 - num::sqr(h.z));
				const TScalar hk = dot(h, k1);
				const XYZ F = Rs + (1 - Rs) * temp::pow5(1 - hk);
				result->value += F * (c * num::pow(h.z, nn) / (hk * std::max(k1.z, k2.z)));
				const TScalar pdfH = 4 * c * num::pow(h.z, n);
				result->pdf += (ps * pdfH) / (4 * hk * ptot);
			}
		}
		++first;
		++result;
	}
}

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

void AshikhminShirley::doSampleBsdf(
		const Sample& sample, const IntersectionContext& context, const TVector3D& omegaIn, 
		const SampleBsdfIn* first, const SampleBsdfIn* last, SampleBsdfOut* result) const
{	
	const XYZ Rd = diffuse_->lookUp(sample, context);
	const XYZ Rs = specular_->lookUp(sample, context);
	const TScalar nu = positiveAverage(specularPowerU_->lookUp(sample, context));
	const TScalar nv = positiveAverage(specularPowerV_->lookUp(sample, context));
	const TScalar rd = positiveAverage(Rd);
	const TScalar rs = positiveAverage(Rs);

	const TVector3D& k1 = omegaIn;
	LASS_ASSERT(k1.z > 0);
	const TScalar a = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k1.z / 2));
	const XYZ rhoA = Rd * (1 - Rs) * (a * 28.f / (23.f * TNumTraits::pi));
	const TScalar c = num::sqrt((nu + 1) * (nv + 1)) / (8 * TNumTraits::pi);

	while (first != last)
	{
		const bool doDiffuse = kernel::hasCaps(first->allowedCaps, Bsdf::capsReflection | Bsdf::capsDiffuse);
		const bool doGlossy = kernel::hasCaps(first->allowedCaps, Bsdf::capsReflection | Bsdf::capsGlossy);
		TScalar pd = doDiffuse ? rd : 0;
		TScalar ps = doGlossy ? rs : 0;
		const TScalar ptot = pd + ps;
		if (ptot > 0)
		{
			pd /= ptot;
			ps /= ptot;

			TVector3D h, k2;
			bool pdfIsDiffuse = false;
			TPoint2D bsdfSample = first->sample;
			if (bsdfSample.x < pd)
			{
				bsdfSample.x /= pd;
				TScalar pdf;
				k2 = num::cosineHemisphere(bsdfSample, pdf).position();
				h = (k1 + k2).normal();
				pdfIsDiffuse = true;
			}
			else
			{
				LASS_ASSERT(ps > 0);
				bsdfSample.x = (bsdfSample.x - pd) / ps;
				h = sampleH(bsdfSample, nu, nv);
				k2 = h.reflect(k1);
			}
			result->omegaOut = k2;

			if (k2.z > 0)
			{
				TScalar pdfD = 0;
				if (doDiffuse)
				{
					const TScalar b = std::max(TNumTraits::zero, 1 - temp::pow5(1 - k2.z / 2));
					result->value += rhoA * b;
					pdfD = k2.z / TNumTraits::pi;
				}
				TScalar pdfS = 0;
				if (doGlossy)
				{
					const TScalar n = nu * num::sqr(h.x) + nv * num::sqr(h.y);
					const TScalar nn = h.z == 1 ? 0 : n / (1 - num::sqr(h.z));
					const TScalar hk = dot(h, k1);
					const XYZ F = Rs + (1 - Rs) * temp::pow5(1 - hk);
					result->value += F * (c * num::pow(h.z, nn) / (hk * std::max(k1.z, k2.z)));
					const TScalar pdfH = 4 * c * num::pow(h.z, n);
					pdfS = pdfH / (4 * hk);
				}
				if (pdfIsDiffuse)
				{
					result->value *= temp::maximumHeuristic(pdfD, pdfS);
					result->pdf = pd * pdfD;
					result->usedCaps = Bsdf::capsReflection | Bsdf::capsDiffuse;
				}
				else
				{
					result->value *= temp::maximumHeuristic(pdfS, pdfD);
					result->pdf = ps * pdfS;
					result->usedCaps = Bsdf::capsReflection | Bsdf::capsGlossy;
				}
			}
		}
		++first;
		++result;
	}
}



const TPyObjectPtr AshikhminShirley::doGetState() const
{
	return python::makeTuple(diffuse_, specular_, specularPowerU_, specularPowerV_,
		numberOfSamples_);
}



void AshikhminShirley::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, diffuse_, specular_, specularPowerU_, specularPowerV_,
		numberOfSamples_);
}



const TVector3D AshikhminShirley::sampleH(const TPoint2D& sample, 
		TScalar nu, TScalar nv/*, TScalar& pdf*/) const
{
	const TScalar f = num::sqrt((nu + 1) / (nv + 1));
	TScalar phi;
	if (sample.x < .5f)
	{
		if (sample.x < .25f)
		{
			phi = num::atan(f * num::tan(2 * TNumTraits::pi * sample.x));
		}
		else
		{
			phi = TNumTraits::pi - num::atan(
				f * num::tan(2 * TNumTraits::pi * (.5f - sample.x)));
		}
	}
	else
	{
		if (sample.x < .75f)
		{
			phi = TNumTraits::pi + num::atan(
				f * num::tan(2 * TNumTraits::pi * (sample.x - .5f))); 
		}
		else
		{
			phi = 2 * TNumTraits::pi - num::atan(
				f * num::tan(2 * TNumTraits::pi * (1.f - sample.x))); 
		}
	}
	const TScalar cosPhi = num::cos(phi);
	const TScalar sinPhi = num::sin(phi);
	
	const TScalar n = nu * num::sqr(cosPhi) + nv * num::sqr(sinPhi);

	const TScalar cosTheta = std::pow(1 - sample.y, num::inv(n + 1));
	const TScalar sinTheta = num::sqrt(std::max(TNumTraits::zero, 1 - num::sqr(cosTheta)));

	//pdf = num::sqrt((nu + 1) * (nv + 1)) / (2 * TNumTraits::pi) * num::pow(cosTheta, n);
	return TVector3D(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}

// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
