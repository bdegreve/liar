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
#include "simple.h"
#include "../kernel/ray_tracer.h"

namespace liar
{
namespace shaders
{

PY_DECLARE_CLASS(Simple)
PY_CLASS_CONSTRUCTOR_0(Simple)
PY_CLASS_CONSTRUCTOR_1(Simple, const TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(Simple, const TTexturePtr&, const TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Simple, "diffuse", diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(Simple, "specular", specular, setSpecular, "texture for specular component")
PY_CLASS_MEMBER_RW_DOC(Simple, "specularPower", specularPower, setSpecularPower, "texture for rollof power of specular component")
PY_CLASS_MEMBER_RW_DOC(Simple, "reflectance", reflectance, setReflectance, "texture for reflective componont")
PY_CLASS_MEMBER_RW_DOC(Simple, "transmittance", transmittance, setTransmittance, "texture for transmittance componont")
PY_CLASS_MEMBER_RW_DOC(Simple, "refractionIndex", refractionIndex, setRefractionIndex, "texture for refraction index")

// --- public --------------------------------------------------------------------------------------

Simple::Simple():
	Shader(&Type),
	diffuse_(Texture::black()),
	specular_(Texture::black()),
	specularPower_(Texture::white()),
	reflectance_(Texture::black()),
	transmittance_(Texture::black()),
	refractionIndex_(Texture::white())
{	
}



Simple::Simple(const TTexturePtr& iDiffuse):
	Shader(&Type),
	diffuse_(iDiffuse),
	specular_(Texture::black()),
	specularPower_(Texture::white()),
	reflectance_(Texture::black()),
	transmittance_(Texture::black()),
	refractionIndex_(Texture::white())
{	
}



Simple::Simple(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular):
	Shader(&Type),
	diffuse_(iDiffuse),
	specular_(iSpecular),
	specularPower_(Texture::white()),
	reflectance_(Texture::black()),
	transmittance_(Texture::black()),
	refractionIndex_(Texture::white())
{	
}



const TTexturePtr& Simple::diffuse() const
{
	return diffuse_;
}



const TTexturePtr& Simple::specular() const
{
	return specular_;
}



const TTexturePtr& Simple::specularPower() const
{
	return specularPower_;
}



const TTexturePtr& Simple::reflectance() const
{
	return reflectance_;
}



const TTexturePtr& Simple::transmittance() const
{
	return transmittance_;
}



const TTexturePtr& Simple::refractionIndex() const
{
	return refractionIndex_;
}



void Simple::setDiffuse(const TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



void Simple::setSpecular(const TTexturePtr& iSpecular)
{
	specular_ = iSpecular;
}



void Simple::setSpecularPower(const TTexturePtr& iSpecularPower)
{
	specularPower_ = iSpecularPower;
}



void Simple::setReflectance(const TTexturePtr& iReflectance)
{
	reflectance_ = iReflectance;
}



void Simple::setTransmittance(const TTexturePtr& iTransmittance)
{
	transmittance_ = iTransmittance;
}



void Simple::setRefractionIndex(const TTexturePtr& iRefractionIndex)
{
	refractionIndex_ = iRefractionIndex;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

Spectrum Simple::doShade(const Sample& iSample,
						 const DifferentialRay& iPrimaryRay,
						 const Intersection& iIntersection,
						 const IntersectionContext& iContext,
						 const RayTracer& iTracer)
{
	const Spectrum diffuse = diffuse_->lookUp(iSample, iContext);
	const Spectrum specular = specular_->lookUp(iSample, iContext);
	const Spectrum specularPower = specularPower_->lookUp(iSample, iContext);
	const Spectrum reflectance = reflectance_->lookUp(iSample, iContext);
	const Spectrum transmittance = transmittance_->lookUp(iSample, iContext);

	const TPoint3D& intersectionPoint = iContext.point();
	const TVector3D& geoNormal = iContext.normal();
	const bool isOutside = dot(geoNormal, iPrimaryRay.direction()) < 0;
	const TVector3D shadeNormal = isOutside ? geoNormal : -geoNormal;

	Spectrum result;

	RayTracer::TLightRange lightSamples = iTracer.sampleLights(iSample, iContext);
	for (RayTracer::TLightRange::iterator i = lightSamples.begin(); i != lightSamples.end(); ++i)
	{
		const TScalar cosTheta = prim::dot(shadeNormal, i->direction());
		
		// diffuse lighting
		//
		if (cosTheta > TNumTraits::zero)
		{
			result += i->radiance() * diffuse * cosTheta;
		}

		// specular lighting
		//
		TVector3D r = shadeNormal;
		r *= 2 * cosTheta;
		r -= i->direction();
		const TScalar cosAlpha = -prim::dot(iPrimaryRay.direction(), r);
		if (cosAlpha > TNumTraits::zero)
		{
			result += i->radiance() * specular * pow(cosAlpha, specularPower);
		}
	}

	if (reflectance)
	{
		const TVector3D reflectedDirection = -geoNormal.reflect(iPrimaryRay.direction());
		const DifferentialRay reflectedRay(
			BoundedRay(intersectionPoint, reflectedDirection, tolerance, TNumTraits::infinity),
			TRay3D(intersectionPoint, reflectedDirection),
			TRay3D(intersectionPoint, reflectedDirection));
		result += reflectance * iTracer.castRay(iSample, reflectedRay);
	}

	if (transmittance)
	{
		const TScalar refractionIndex = refractionIndex_->lookUp(iSample, iContext).averagePower();
		const TScalar n = !isOutside ? refractionIndex : num::inv(refractionIndex);
		const TScalar cosI = -dot(shadeNormal, iPrimaryRay.direction());
		const TScalar sinT2 = num::sqr(n) * (TNumTraits::one - num::sqr(cosI));
		if (sinT2 <= TNumTraits::one)
		{
			const TScalar cosT = num::sqrt(TNumTraits::one - sinT2);
			const TVector3D refractedDirection = n * iPrimaryRay.direction() +
				(n * cosI - cosT) * shadeNormal;
			const DifferentialRay refractedRay(
				BoundedRay(intersectionPoint, refractedDirection, tolerance, TNumTraits::infinity),
				TRay3D(intersectionPoint, refractedDirection),
				TRay3D(intersectionPoint, refractedDirection));
			result += transmittance * iTracer.castRay(iSample, refractedRay);
		}
		else
		{
			result += kernel::Spectrum(TVector3D(1, 0, 1));
		}
	}

	return result;
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF