/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2025  Bram de Greve (bramz@users.sourceforge.net)
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

#include "scenery_common.h"
#include "light_area.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightArea, "area light")
PY_CLASS_CONSTRUCTOR_1(LightArea, const TSceneObjectRef&)
PY_CLASS_MEMBER_R(LightArea, surface)
PY_CLASS_MEMBER_RW(LightArea, radiance, setRadiance)
PY_CLASS_MEMBER_RW(LightArea, attenuation, setAttenuation)
PY_CLASS_MEMBER_RW(LightArea, numberOfEmissionSamples, setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightArea, isDoubleSided, setDoubleSided)



// --- public --------------------------------------------------------------------------------------

LightArea::LightArea(const TSceneObjectRef& iSurface):
	surface_(iSurface),
	radiance_(Spectrum::white()),
	attenuation_(Attenuation::defaultAttenuation()),
	numberOfEmissionSamples_(9),
	isSingleSided_(true)
{
	if (!surface_->hasSurfaceSampling())
	{
		const auto &s = *surface_;
		LASS_THROW("The object used a surface for a LightArea must support surface sampling. "
			"Objects of type '" << typeid(s).name() << "' don't support that.");
	}

	LASS_ASSERT(surface_);
}



const TSceneObjectRef& LightArea::surface() const
{
	return surface_;
}



const TSpectrumRef& LightArea::radiance() const
{
	return radiance_;
}



const TAttenuationRef& LightArea::attenuation() const
{
	return attenuation_;
}



bool LightArea::isDoubleSided() const
{
	return !isSingleSided_;
}



void LightArea::setRadiance(const TSpectrumRef& radiance)
{
	radiance_ = radiance;
}



void LightArea::setAttenuation(const TAttenuationRef& iAttenuation)
{
	attenuation_ = iAttenuation;
}



void LightArea::setNumberOfEmissionSamples(unsigned number)
{
	numberOfEmissionSamples_ = number;
}



void LightArea::setDoubleSided(bool iIsDoubleSided)
{
	isSingleSided_ = !iIsDoubleSided;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightArea::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	LASS_ASSERT(surface_);
	surface_->intersect(sample, ray, result);
	if (result)
	{
		result.push(this);
	}
}



bool LightArea::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	LASS_ASSERT(surface_);
	return surface_->isIntersecting(sample, ray);
}



void LightArea::doLocalContext(const Sample& sample, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	IntersectionDescendor descend(intersection);
	LASS_ASSERT(surface_);
	surface_->localContext(sample, ray, intersection, result);
}



bool LightArea::doContains(const Sample& sample, const TPoint3D& point) const
{
	LASS_ASSERT(surface_);
	return surface_->contains(sample, point);
}




const TAabb3D LightArea::doBoundingBox() const
{
	LASS_ASSERT(surface_);
	return surface_->boundingBox();
}



TScalar LightArea::doArea() const
{
	LASS_ASSERT(surface_);
	return surface_->area();
}



TScalar LightArea::doArea(const TVector3D& normal) const
{
	LASS_ASSERT(surface_);
	return surface_->area(normal);
}



const Spectral LightArea::doEmission(const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	TVector3D normalLight;
	pdf = surface_->angularPdf(sample, ray, shadowRay, normalLight);
	if (pdf <= 0)
	{
		return Spectral();
	}
	if (isSingleSided_ && dot(normalLight, shadowRay.direction()) > 0)
	{
		pdf = 0;
		return Spectral();
	}
	return radiance_->evaluate(sample, SpectralType::Illuminant);
}



const Spectral LightArea::doSampleEmission(
	const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	LASS_ASSERT(surface_);
	TVector3D normalLight;
	const TPoint3D pointLight = surface_->sampleSurface(lightSample, target, normalLight, pdf);

	TVector3D toLight = pointLight - target;
	const TScalar distance = toLight.norm();
	toLight /= distance;

	shadowRay = BoundedRay(target, toLight, tolerance, distance, prim::IsAlreadyNormalized());

	if (isSingleSided_ && dot(normalLight, toLight) > 0)
	{
		pdf = 0;
		return Spectral(0);
	}
	return radiance_->evaluate(sample, SpectralType::Illuminant);
}



const Spectral LightArea::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSample, const TPoint3D& target, const TVector3D& normalTarget,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	LASS_ASSERT(surface_);
	TVector3D normalLight;
	const TPoint3D pointLight =
		surface_->sampleSurface(lightSample, target, normalTarget, normalLight, pdf);

	TVector3D toLight = pointLight - target;
	const TScalar distance = toLight.norm();
	toLight /= distance;

	shadowRay = BoundedRay(target, toLight, tolerance, distance, prim::IsAlreadyNormalized());

	if ((dot(normalTarget, toLight) <= 0) || (isSingleSided_ && dot(normalLight, toLight) > 0))
	{
		pdf = 0;
		return Spectral(0);
	}
	return radiance_->evaluate(sample, SpectralType::Illuminant);
}



const Spectral LightArea::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
		BoundedRay& emissionRay, TScalar& pdf) const
{
	TVector3D originNormal;
	TScalar originPdf;
	const TPoint3D origin = surface_->sampleSurface(lightSampleA, originNormal, originPdf);

	TPoint2D directionSample = lightSampleB;
	if (!isSingleSided_)
	{
		directionSample.x *= 2;
		if (directionSample.x >= 1)
		{
			originNormal = -originNormal;
			directionSample.x -= 1;
		}
	}

	TVector3D originU, originV;
	generateOrthonormal(originNormal, originU, originV);

	TScalar directionPdf;
	TVector3D localDirection = num::cosineHemisphere(lightSampleB, directionPdf).position();
	const TVector3D direction = originU * localDirection.x + originV * localDirection.y
		+ originNormal * localDirection.z;

	emissionRay = BoundedRay(origin, direction, tolerance);
	pdf = originPdf * directionPdf;
	return radiance_->evaluate(sample, SpectralType::Illuminant) * static_cast<Spectral::TValue>(localDirection.z);
}



TScalar LightArea::doTotalPower() const
{
	const TScalar singleSidedPower = (TNumTraits::pi * surface_->area()) * radiance_->luminance();
	return isSingleSided_ ? singleSidedPower : 2 * singleSidedPower;
}



size_t LightArea::doNumberOfEmissionSamples() const
{
	return numberOfEmissionSamples_;
}



bool LightArea::doIsSingular() const
{
	return false;
}



const TPyObjectPtr LightArea::doGetLightState() const
{
	return python::makeTuple(surface_, radiance_, attenuation_,
		numberOfEmissionSamples_, isSingleSided_);
}



void LightArea::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, surface_, radiance_, attenuation_,
		numberOfEmissionSamples_, isSingleSided_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
