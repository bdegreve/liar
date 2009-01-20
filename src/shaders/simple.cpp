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
PY_CLASS_MEMBER_RW_DOC(Simple, diffuse, setDiffuse, "texture for diffuse component")
PY_CLASS_MEMBER_RW_DOC(Simple, specular, setSpecular, "texture for specular component")
PY_CLASS_MEMBER_RW_DOC(Simple, specularPower, setSpecularPower, "texture for rollof power of specular component")
PY_CLASS_MEMBER_RW_DOC(Simple, reflectance, setReflectance, "texture for reflective componont")
PY_CLASS_MEMBER_RW_DOC(Simple, transmittance, setTransmittance, "texture for transmittance componont")
PY_CLASS_MEMBER_RW_DOC(Simple, refractionIndex, setRefractionIndex, "texture for refraction index")

// --- public --------------------------------------------------------------------------------------

Simple::Simple():
	diffuse_(Texture::black()),
	specular_(Texture::black()),
	specularPower_(Texture::white()),
	reflectance_(Texture::black()),
	transmittance_(Texture::black()),
	refractionIndex_(Texture::white())
{	
}



Simple::Simple(const TTexturePtr& iDiffuse):
	diffuse_(iDiffuse),
	specular_(Texture::black()),
	specularPower_(Texture::white()),
	reflectance_(Texture::black()),
	transmittance_(Texture::black()),
	refractionIndex_(Texture::white())
{	
}



Simple::Simple(const TTexturePtr& iDiffuse, const TTexturePtr& iSpecular):
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

const Spectrum Simple::doShade(	
	const Sample& iSample,
	const DifferentialRay& iPrimaryRay,
	const Intersection& iIntersection,
	const IntersectionContext& iContext) const
{
	const Spectrum diffuse = diffuse_->lookUp(iSample, iContext) / TNumTraits::pi;
	const Spectrum specular = specular_->lookUp(iSample, iContext);
	const Spectrum specularPower = specularPower_->lookUp(iSample, iContext);
	const Spectrum reflectance = reflectance_->lookUp(iSample, iContext);
	const Spectrum transmittance = transmittance_->lookUp(iSample, iContext);

	const TPoint3D& intersectionPoint = iContext.point();
	const TVector3D& normal = iContext.normal();
	const bool isLeaving = iIntersection.solidEvent() == seLeaving;

	Spectrum result;

	TLightSamplesRange lightSamples = iContext.sampleLights(iSample);
	for (TLightSamplesRange::iterator i = lightSamples.begin(); i != lightSamples.end(); ++i)
	{
		const TScalar cosTheta = prim::dot(normal, i->direction());
		
		// diffuse lighting
		//
		if (cosTheta > TNumTraits::zero)
		{
			result += i->radiance() * diffuse * cosTheta;
		}

		// specular lighting
		//
		const TVector3D h = (i->direction() - iPrimaryRay.direction()).normal();
		const TScalar cosAlpha = prim::dot(h, normal);
		if (cosAlpha > TNumTraits::zero)
		{
			result += i->radiance() * specular * pow(cosAlpha, specularPower);
		}
	}

	if (reflectance)
	{
		result += reflectance * iContext.gather(iSample, reflect(iContext, iPrimaryRay));
	}

	if (transmittance)
	{
		const TScalar refractionIndex = refractionIndex_->lookUp(iSample, iContext).average();
		const TScalar n1overn2 = isLeaving ? refractionIndex : num::inv(refractionIndex);
		const DifferentialRay refractedRay = refract(iContext, iPrimaryRay, n1overn2);
		if (refractedRay.isValid())
		{
			result += transmittance * iContext.gather(iSample, refractedRay);
		}
	}

	return result;
}



const TPyObjectPtr Simple::doGetState() const
{
	return python::makeTuple(diffuse_, specular_, specularPower_, reflectance_,
		transmittance_, refractionIndex_);
}



void Simple::doSetState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, diffuse_, specular_, specularPower_, reflectance_,
		transmittance_, refractionIndex_);
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
