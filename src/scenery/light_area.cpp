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

#include "scenery_common.h"
#include "light_area.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(LightArea)
PY_CLASS_CONSTRUCTOR_1(LightArea, const TSceneObjectPtr&)
PY_CLASS_MEMBER_R(LightArea, "surface", surface)
PY_CLASS_MEMBER_RW(LightArea, "radiance", radiance, setRadiance)
PY_CLASS_MEMBER_RW(LightArea, "attenuation", attenuation, setAttenuation)
PY_CLASS_MEMBER_RW(LightArea, "numberOfEmissionSamples", numberOfEmissionSamples, 
	setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightArea, "isDoubleSided", isDoubleSided, setDoubleSided)



// --- public --------------------------------------------------------------------------------------

LightArea::LightArea(const TSceneObjectPtr& iSurface):
    SceneLight(&Type),
	surface_(LASS_ENFORCE_POINTER(iSurface)),
	radiance_(Spectrum(1)),
	attenuation_(Attenuation::defaultAttenuation()),
	numberOfEmissionSamples_(9),
	isSingleSided_(true)
{
	if (!surface_->hasSurfaceSampling())
	{
		LASS_THROW("The object used a surface for a LightArea must support surface sampling. "
			"Objects of type '" << typeid(*surface_).name() << "' don't support that.");
	}

	LASS_ASSERT(surface_);
}



const TSceneObjectPtr& LightArea::surface() const
{
	return surface_;
}



const Spectrum& LightArea::radiance() const
{
	return radiance_;
}



const TAttenuationPtr& LightArea::attenuation() const
{
	return attenuation_;
}



const bool LightArea::isDoubleSided() const
{
	return !isSingleSided_;
}



void LightArea::setRadiance(const Spectrum& iRadiance)
{
	radiance_ = iRadiance;
}



void LightArea::setAttenuation(const TAttenuationPtr& iAttenuation)
{
	attenuation_ = iAttenuation;
}



void LightArea::setNumberOfEmissionSamples(unsigned iNumber)
{
	numberOfEmissionSamples_ = iNumber;
}



void LightArea::setDoubleSided(bool iIsDoubleSided)
{
	isSingleSided_ = !iIsDoubleSided;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightArea::doIntersect(const Sample& iSample, const BoundedRay& iRay, 
							 Intersection& oResult) const
{
	LASS_ASSERT(surface_);
	surface_->intersect(iSample, iRay, oResult);
	if (oResult)
	{
		oResult.push(this);
	}
}



const bool LightArea::doIsIntersecting(const Sample& iSample, 
											  const BoundedRay& iRay) const
{
	LASS_ASSERT(surface_);
	return surface_->isIntersecting(iSample, iRay);
}



void LightArea::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								const Intersection& iIntersection, 
								IntersectionContext& oResult) const
{
    IntersectionDescendor descend(iIntersection);
	LASS_ASSERT(surface_);
	surface_->localContext(iSample, iRay, iIntersection, oResult);
}



const bool LightArea::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	LASS_ASSERT(surface_);
	return surface_->contains(iSample, iPoint);
}




const TAabb3D LightArea::doBoundingBox() const
{
	LASS_ASSERT(surface_);
	return surface_->boundingBox();
}



const TScalar LightArea::doArea() const
{
	LASS_ASSERT(surface_);
	return surface_->area();
}



const Spectrum LightArea::doSampleEmission(
		const Sample& iSample,
		const TVector2D& iLightSample, 
		const TPoint3D& iTarget,
		const TVector3D& iNormalTarget,
		BoundedRay& oShadowRay,
		TScalar& oPdf) const
{
	LASS_ASSERT(surface_);
	TVector3D normalLight;
	const TPoint3D pointLight = 
		surface_->sampleSurface(iLightSample, iTarget, iNormalTarget, normalLight, oPdf);

	TVector3D toLight = pointLight - iTarget;
	const TScalar squaredDistance = toLight.squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toLight /= distance;

	const TScalar cosThetaTarget = dot(iNormalTarget, toLight);
	if (cosThetaTarget <= TNumTraits::zero)
	{
		oPdf = TNumTraits::zero;
		return Spectrum();
	}
	
	TScalar cosThetaLight = -dot(normalLight, toLight);
	if (cosThetaLight <= TNumTraits::zero)
	{
		if (isSingleSided_)
		{
			oPdf = TNumTraits::zero;
			return Spectrum();
		}
		else
		{
			cosThetaLight = -cosThetaLight;
		}
	}

	oShadowRay = BoundedRay(iTarget, toLight, tolerance, distance, 
		prim::IsAlreadyNormalized());
	return radiance_;
	//return radiance_ * (cosThetaLight / (attenuation_->attenuation(distance, squaredDistance)));
}



const unsigned LightArea::doNumberOfEmissionSamples() const
{
	return numberOfEmissionSamples_;
}



const TPyObjectPtr LightArea::doGetLightState() const
{
	return python::makeTuple(surface_, radiance_, attenuation_, 
		numberOfEmissionSamples_, isSingleSided_);
}



void LightArea::doSetLightState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, surface_, radiance_, attenuation_,
		numberOfEmissionSamples_, isSingleSided_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
