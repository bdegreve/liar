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
PY_CLASS_CONSTRUCTOR_1(Simple, const kernel::TTexturePtr&)
PY_CLASS_CONSTRUCTOR_2(Simple, const kernel::TTexturePtr&, const kernel::TTexturePtr&)
PY_CLASS_MEMBER_RW_DOC(Simple, "diffuse", diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(Simple, "specular", specular, setSpecular, "texture for specular component")
PY_CLASS_MEMBER_RW_DOC(Simple, "specularPower", specularPower, setSpecularPower, "texture for rollof power of specular component")

// --- public --------------------------------------------------------------------------------------

Simple::Simple():
	kernel::Shader(&Type),
	diffuse_(kernel::Texture::white()),
	specular_(kernel::Texture::black()),
	specularPower_(kernel::Texture::white()),
	reflective_(kernel::Texture::black())
{	
}



Simple::Simple(const kernel::TTexturePtr& iDiffuse):
	kernel::Shader(&Type),
	diffuse_(iDiffuse),
	specular_(kernel::Texture::black()),
	specularPower_(kernel::Texture::white()),
	reflective_(kernel::Texture::black())
{	
}



Simple::Simple(const kernel::TTexturePtr& iDiffuse, const kernel::TTexturePtr& iSpecular):
	kernel::Shader(&Type),
	diffuse_(iDiffuse),
	specular_(iSpecular),
	specularPower_(kernel::Texture::white()),
	reflective_(kernel::Texture::black())
{	
}



const kernel::TTexturePtr& Simple::diffuse() const
{
	return diffuse_;
}



const kernel::TTexturePtr& Simple::specular() const
{
	return specular_;
}



const kernel::TTexturePtr& Simple::specularPower() const
{
	return specularPower_;
}



const kernel::TTexturePtr& Simple::reflective() const
{
	return reflective_;
}



void Simple::setDiffuse(const kernel::TTexturePtr& iDiffuse)
{
	diffuse_ = iDiffuse;
}



void Simple::setSpecular(const kernel::TTexturePtr& iSpecular)
{
	specular_ = iSpecular;
}



void Simple::setSpecularPower(const kernel::TTexturePtr& iSpecularPower)
{
	specularPower_ = iSpecularPower;
}



void Simple::setReflective(const kernel::TTexturePtr& iReflective)
{
	reflective_ = iReflective;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

kernel::Spectrum Simple::doDirectLight(const kernel::Sample& iSample,
									   const kernel::DifferentialRay& iPrimaryRay,
									   const kernel::Intersection& iIntersection,
									   const kernel::IntersectionContext& iContext,
									   const kernel::TSceneObjectPtr& iScene,
									   const kernel::LightContext& iLight)
{
	// can we move these outside?
	const kernel::Spectrum diffuse = diffuse_->lookUp(iContext);
	const kernel::Spectrum specular = specular_->lookUp(iContext);
	const kernel::Spectrum specularPower = specularPower_->lookUp(iContext);
	if (!diffuse && !specular)
	{
		return kernel::Spectrum();
	}

	const TPoint3D& intersectionPoint = iContext.point();
	const TPoint3D shadowStartPoint = intersectionPoint + 
		(liar::tolerance * intersectionPoint.position().norm()) * iContext.normal(); 

	kernel::Spectrum result;
	for (kernel::Sample::TSubSequence2D i = iSample.subSequence2D(iLight.idLightSamples()); i; ++i)
	{
		TRay3D shadowRay;
		TScalar maxT;
		kernel::Spectrum radiance = iLight.sampleRadiance(
			*i, shadowStartPoint, iSample.time(), shadowRay, maxT);
		
		if (iLight.light()->isShadowless() || !iScene->isIntersecting(iSample, shadowRay, maxT))
		{
			const TScalar cosTheta = prim::dot(iContext.normal(), shadowRay.direction());
			
			// diffuse lighting
			if (cosTheta > TNumTraits::zero)
			{
				result += radiance * diffuse * cosTheta;
			}

			// specular lighting
			TVector3D r = iContext.normal();
			r *= 2 * cosTheta;
			r -= shadowRay.direction();
			const TScalar cosAlpha = -prim::dot(iPrimaryRay.ray().direction(), r);
			if (cosAlpha > TNumTraits::zero)
			{
				result += radiance * specular * pow(cosAlpha, specularPower);
			}
		}
	}

	return result;
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF