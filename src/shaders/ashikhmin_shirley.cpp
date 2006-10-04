/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

#include "shaders_common.h"
#include "ashikhmin_shirley.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(AshikhminShirley)
PY_CLASS_CONSTRUCTOR_0(AshikhminShirley)
PY_CLASS_CONSTRUCTOR_2(AshikhminShirley, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, "diffuse", diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, "specular", specular, setSpecular, "texture for specular component")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, "specularPowerU", specularPowerU, setSpecularPowerU, "texture for rollof power of specular component in U direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, "specularPowerV", specularPowerV, setSpecularPowerV, "texture for rollof power of specular component in V direction")
PY_CLASS_MEMBER_RW_DOC(AshikhminShirley, "numberOfSamples", numberOfSamples, setNumberOfSamples, "set number of samples for Monte Carlo simulations")

// --- public --------------------------------------------------------------------------------------

AshikhminShirley::AshikhminShirley():
	diffuse_(Texture::white()),
	specular_(Texture::white()),
	specularPowerU_(Texture::white()),
	specularPowerV_(Texture::white()),
	numberOfSamples_(4),
	samplesId_(-1)
{
}



AshikhminShirley::AshikhminShirley(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular):
	diffuse_(iDiffuse),
	specular_(iSpecular),
	specularPowerU_(Texture::white()),
	specularPowerV_(Texture::white())
{
}



const TTexturePtr& AshikhminShirley::diffuse() const
{
	return diffuse_;
}



void AshikhminShirley::setDiffuse(const TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



const TTexturePtr& AshikhminShirley::specular() const
{
	return specular_;
}



void AshikhminShirley::setSpecular(const TTexturePtr& iSpecular)
{
	specular_ = iSpecular;
}



const TTexturePtr& AshikhminShirley::specularPowerU() const
{
	return specularPowerU_;
}



void AshikhminShirley::setSpecularPowerU(const TTexturePtr& iSpecularPower)
{
	specularPowerU_ = iSpecularPower;
}



const TTexturePtr& AshikhminShirley::specularPowerV() const
{
	return specularPowerV_;
}



void AshikhminShirley::setSpecularPowerV(const TTexturePtr& iSpecularPower)
{
	specularPowerV_ = iSpecularPower;
}



const unsigned AshikhminShirley::numberOfSamples() const
{
	return numberOfSamples_;
}



void AshikhminShirley::setNumberOfSamples(unsigned iNumber)
{
	numberOfSamples_ = std::max<unsigned>(iNumber, 1);
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

const Spectrum AshikhminShirley::doShade(
	const Sample& iSample,
	const DifferentialRay& iPrimaryRay,
	const Intersection& iIntersection,
	const IntersectionContext& iContext) const
{
	
	const TVector3D k1 = -iPrimaryRay.direction();
	const TVector3D n = iContext.normal();
	const TVector3D u = iContext.dPoint_dU().normal();
	const TVector3D v = cross(n, u).normal();
	
	const Spectrum Rd = diffuse_->lookUp(iSample, iContext);
	const Spectrum Rs = specular_->lookUp(iSample, iContext);
	const TScalar nu = specularPowerU_->lookUp(iSample, iContext).average();
	const TScalar nv = specularPowerV_->lookUp(iSample, iContext).average();

	Spectrum result;

	
	TLightSamplesRange lightSamples = iContext.sampleLights(iSample);
	for (TLightSamplesRange::iterator i = lightSamples.begin(); i != lightSamples.end(); ++i)
	{
		const TVector3D k2 = i->direction();
		const TScalar cosTheta = prim::dot(n, k2);
		if (cosTheta > TNumTraits::zero)
		{
			const TVector3D h = (k1 + k2).normal();
			result += brdf(k1, k2, h, n, u, v, Rs, Rd, nu, nv) * i->radiance() * cosTheta;
		}
	}
	

	// reflection

	const TScalar ani = num::sqrt((nu + 1) / (nv + 1));
	const TScalar pi_2 = 2 * TNumTraits::pi;;
	Sample::TSubSequence2D sample = iSample.subSequence2D(samplesId_);
	const TScalar scaler = 1.f / sample.size();
	while (sample)
	{
		TScalar pdf = 0.f;
		const TVector3D h = generateH(*sample, n, u, v, nu, nv, pdf);
		const TScalar k1_dot_h = dot(k1, h);
		if (k1_dot_h > 0 && pdf > 0)
		{
			const TVector3D k2 = -k1 + 2 * k1_dot_h * h;
			pdf /= 4 * k1_dot_h;

			TRay3D ray(iContext.point(), k2);
			BoundedRay boundedRay(ray, tolerance);
			DifferentialRay differentialRay(boundedRay, ray, ray);

			result += (scaler / pdf) * brdf(k1, k2, h, n, u, v, Rs, Rd, nu, nv) * 
				iContext.gather(iSample, differentialRay);
		}

		++sample;
	}

	return result;
}



void AshikhminShirley::doRequestSamples(const TSamplerPtr& iSampler)
{
	samplesId_ = iSampler->requestSubSequence2D(numberOfSamples_);
}



const TPyObjectPtr AshikhminShirley::doGetState() const
{
	return python::makeTuple(diffuse_, specular_, specularPowerU_, specularPowerV_,
		numberOfSamples_, samplesId_);
}



void AshikhminShirley::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, diffuse_, specular_, specularPowerU_, specularPowerV_,
		numberOfSamples_, samplesId_);
}



inline const Spectrum AshikhminShirley::brdf(
		const TVector3D& k1, const TVector3D& k2, const TVector3D& h,
		const TVector3D& n, const TVector3D& u, const TVector3D& v,
		const Spectrum& Rs, const Spectrum& Rd, TScalar nu, TScalar nv) const
{
	const TScalar n_dot_k1 = dot(n, k1);
	const TScalar n_dot_k2 = dot(n, k2);
	if (n_dot_k1 <= 0 || n_dot_k2 <= 0)
	{
		return Spectrum();
	}

	const TScalar n_dot_h = dot(n, h);
	const TScalar u_dot_h = dot(u, h);
	const TScalar v_dot_h = dot(v, h);
	const TScalar k_dot_h = dot(k1, h);

	const TScalar a = std::max<TScalar>(0, 1 - temp::pow5(1 - n_dot_k1 / 2));
	const TScalar b = std::max<TScalar>(0, 1 - temp::pow5(1 - n_dot_k2 / 2));
	const Spectrum rhoD = ((28.f / (23.f * TNumTraits::pi)) * a * b) * Rd * (1 - Rs);

	const TScalar c = num::sqrt((nu + 1) * (nv + 1)) / (8 * TNumTraits::pi);
	const TScalar d = num::pow(n_dot_h, 
		(nu * num::sqr(u_dot_h) + nv * num::sqr(v_dot_h)) / (1 - num::sqr(n_dot_h)));
	const TScalar e = 1 + k_dot_h * std::max(n_dot_k1, n_dot_k2);
	const Spectrum F = Rs + (1 - Rs) * temp::pow5(1 - k_dot_h);
	const Spectrum rhoS = (c * d / e) * F;

	return rhoD + rhoS;
}



inline const TVector3D AshikhminShirley::generateH(const TVector2D& iSample, 
		const TVector3D& n, const TVector3D& u, const TVector3D& v, 
		TScalar nu, TScalar nv, TScalar& oPdf) const
{
	const TScalar f = num::sqrt((nu + 1) / (nv + 1));
	TScalar phi;
	if (iSample.x < .5f)
	{
		if (iSample.x < .25f)
		{
			phi = num::atan(f * num::tan(2 * TNumTraits::pi * iSample.x));
		}
		else
		{
			phi = TNumTraits::pi - num::atan(
				f * num::tan(2 * TNumTraits::pi * (.5f - iSample.x)));
		}
	}
	else
	{
		if (iSample.x < .75f)
		{
			phi = TNumTraits::pi + num::atan(
				f * num::tan(2 * TNumTraits::pi * (iSample.x - .5f))); 
		}
		else
		{
			phi = 2 * TNumTraits::pi - num::atan(
				f * num::tan(2 * TNumTraits::pi * (1.f - iSample.x))); 
		}
	}
	const TScalar cosPhi = num::cos(phi);
	const TScalar sinPhi = num::sin(phi);

	const TScalar cosTheta = std::pow(1 - iSample.y, 
		num::inv(nu * cosPhi * cosPhi + nv * sinPhi * sinPhi + 1));
	const TScalar sinTheta = num::sqrt(std::max(TNumTraits::zero, 1 - cosTheta * cosTheta));

	oPdf = num::sqrt((nu + 1) * (nv + 1)) / (2 * TNumTraits::pi) * 
		num::pow(cosTheta, nu * cosPhi * cosPhi + nv * sinPhi * sinPhi);

	return (cosPhi * sinTheta) * u + (sinPhi * sinTheta) * v +	cosTheta * n;

}

// --- free ----------------------------------------------------------------------------------------



}

}

// EOF