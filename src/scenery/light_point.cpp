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
#include "light_point.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(LightPoint)
PY_CLASS_CONSTRUCTOR_0(LightPoint)
PY_CLASS_CONSTRUCTOR_2(LightPoint, const TPoint3D&, const Spectrum&)
PY_CLASS_MEMBER_RW(LightPoint, "position", position, setPosition)
PY_CLASS_MEMBER_RW(LightPoint, "intensity", intensity, setIntensity)
PY_CLASS_MEMBER_RW(LightPoint, "attenuation", attenuation, setAttenuation)


// --- public --------------------------------------------------------------------------------------

LightPoint::LightPoint():
    SceneLight(&Type),
	position_(TPoint3D()),
	intensity_(Spectrum(1)),
	attenuation_(Attenuation::defaultAttenuation())
{
}



LightPoint::LightPoint(const TPoint3D& iPosition, const Spectrum& iIntensity):
    SceneLight(&Type),
    position_(iPosition),
	intensity_(iIntensity),
	attenuation_(Attenuation::defaultAttenuation())
{
}



const TPoint3D& LightPoint::position() const
{
	return position_;
}



const Spectrum& LightPoint::intensity() const
{
	return intensity_;
}



const TAttenuationPtr& LightPoint::attenuation() const
{
	return attenuation_;
}



void LightPoint::setPosition(const TPoint3D& iPosition)
{
	position_ = iPosition;
}



void LightPoint::setIntensity(const Spectrum& iIntensity)
{
	intensity_ = iIntensity;
}



void LightPoint::setAttenuation(const TAttenuationPtr& iAttenuation)
{
	attenuation_ = iAttenuation;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightPoint::doIntersect(const Sample& iSample, const BoundedRay& iRAy, 
							 Intersection& oResult) const
{
	oResult = Intersection::empty();
}



const bool LightPoint::doIsIntersecting(const Sample& iSample, 
										const BoundedRay& iRay) const
{
	return false;
}



void LightPoint::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								const Intersection& iIntersection, 
								IntersectionContext& oResult) const
{
	LASS_THROW("since LightPoint can never return an intersection, you've called dead code.");
}



const bool LightPoint::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}




const TAabb3D LightPoint::doBoundingBox() const
{
	return TAabb3D(position_, position_);
}



const TScalar LightPoint::doArea() const
{
	return 0;
}



const Spectrum LightPoint::doSampleEmission(
		const Sample& iSample,
		const TVector2D& iLightSample, 
		const TPoint3D& iTarget,
		const TVector3D& iTargetNormal,
		BoundedRay& oShadowRay,
		TScalar& oPdf) const
{
	TVector3D toLight = position_ - iTarget;
    const TScalar squaredDistance = (position_ - iTarget).squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toLight /= distance;

	oShadowRay = BoundedRay(iTarget, toLight, tolerance, distance, 
		prim::IsAlreadyNormalized());
	oPdf = TNumTraits::one;

	return intensity_ / attenuation_->attenuation(distance, squaredDistance);
}



const unsigned LightPoint::doNumberOfEmissionSamples() const
{
	return 1;
}



const TPyObjectPtr LightPoint::doGetLightState() const
{
	return python::makeTuple(position_, intensity_, attenuation_);
}



void LightPoint::doSetLightState(const TPyObjectPtr& iState)
{
	python::decodeTuple(iState, position_, intensity_, attenuation_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
