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
PY_CLASS_MEMBER_RW(LightArea, "intensity", intensity, setIntensity)
PY_CLASS_MEMBER_RW(LightArea, "numberOfEmissionSamples", numberOfEmissionSamples, 
	setNumberOfEmissionSamples)
PY_CLASS_MEMBER_RW(LightArea, "isDoubleSided", isDoubleSided, setDoubleSided)


// --- public --------------------------------------------------------------------------------------

LightArea::LightArea(const TSceneObjectPtr& iSurface):
    SceneLight(&Type),
	surface_(LASS_ENFORCE_POINTER(iSurface)),
	intensity_(Spectrum(1)),
	numberOfEmissionSamples_(9),
	isSingleSided_(true)
{
	LASS_ASSERT(surface_);
	area_ = surface_->area();
}



const TSceneObjectPtr& LightArea::surface() const
{
	return surface_;
}



const Spectrum& LightArea::intensity() const
{
	return intensity_;
}



const bool LightArea::isDoubleSided() const
{
	return !isSingleSided_;
}



void LightArea::setIntensity(const Spectrum& iIntensity)
{
	intensity_ = iIntensity;
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



const Spectrum LightArea::doSampleEmission(const Sample& iSample,
											const TVector2D& iLightSample, 
											const TPoint3D& iTarget,
											BoundedRay& oShadowRay,
											TScalar& oPdf) const
{
	LASS_ASSERT(surface_);
	TVector3D normal;
	const TPoint3D point = surface_->sampleSurface(iLightSample, iTarget, normal);

	TVector3D toTarget = iTarget - point;
	const TScalar squaredDistance = toTarget.squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toTarget /= distance;
	
	const TScalar cosTheta = dot(normal, toTarget);
	if (isSingleSided_ && cosTheta < 0)
	{
		return Spectrum();
	}

	oShadowRay = BoundedRay(iTarget, point, tolerance, distance);
	return intensity_ * area_ * cosTheta / (2 * TNumTraits::pi * distance);
}



const unsigned LightArea::doNumberOfEmissionSamples() const
{
	return numberOfEmissionSamples_;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
