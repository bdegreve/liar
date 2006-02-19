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
#include "light_spot.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_NAME(PyLightSpotAttenuation, "Attenuation")
PY_CLASS_CONSTRUCTOR_0(PyLightSpotAttenuation);
PY_CLASS_CONSTRUCTOR_3(PyLightSpotAttenuation, TScalar, TScalar, TScalar);
PY_CLASS_PUBLIC_MEMBER(PyLightSpotAttenuation, constant);
PY_CLASS_PUBLIC_MEMBER(PyLightSpotAttenuation, linear);
PY_CLASS_PUBLIC_MEMBER(PyLightSpotAttenuation, quadratic);

PY_DECLARE_CLASS(LightSpot)
PY_CLASS_CONSTRUCTOR_0(LightSpot)
PY_CLASS_MEMBER_RW(LightSpot, "position", position, setPosition)
PY_CLASS_MEMBER_RW(LightSpot, "direction", direction, setDirection)
PY_CLASS_MEMBER_RW(LightSpot, "intensity", intensity, setIntensity)
PY_CLASS_MEMBER_RW(LightSpot, "attenuation", attenuation, setAttenuation)
PY_CLASS_MEMBER_RW(LightSpot, "outerAngle", outerAngle, setOuterAngle)
PY_CLASS_MEMBER_RW(LightSpot, "innerAngle", innerAngle, setInnerAngle)
PY_CLASS_METHOD(LightSpot, lookAt);
PY_CLASS_INNER_CLASS_NAME(LightSpot, PyLightSpotAttenuation, "Attenuation")


// --- public --------------------------------------------------------------------------------------

LightSpot::Attenuation::Attenuation():
	constant(TNumTraits::zero),
    linear(TNumTraits::zero),
    quadratic(2 * TNumTraits::pi)
{
}



LightSpot::Attenuation::Attenuation(TScalar iConstant, TScalar iLinear, TScalar iQuadratic):
    constant(iConstant),
    linear(iLinear),
    quadratic(iQuadratic)
{
}



LightSpot::LightSpot():
    SceneLight(&Type),
	position_(TPoint3D()),
	direction_(TVector3D(0, 0, -1)),
	intensity_(Spectrum(1)),
    attenuation_(),
	cosOuterAngle_(num::cos(TNumTraits::pi / 4)),
	cosInnerAngle_(num::cos(TNumTraits::pi / 6))
{
}



const TPoint3D& LightSpot::position() const
{
	return position_;
}



const TVector3D& LightSpot::direction() const
{
	return direction_;
}



const Spectrum& LightSpot::intensity() const
{
	return intensity_;
}



const LightSpot::Attenuation& LightSpot::attenuation() const
{
	return attenuation_;
}



void LightSpot::setPosition(const TPoint3D& iPosition)
{
	position_ = iPosition;
}



const TScalar LightSpot::outerAngle() const
{
	return num::acos(cosOuterAngle_);
}



const TScalar LightSpot::innerAngle() const
{
	return num::acos(cosInnerAngle_);
}



void LightSpot::setDirection(const TVector3D& iDirection)
{
	direction_ = iDirection.normal();
}



void LightSpot::setIntensity(const Spectrum& iIntensity)
{
	intensity_ = iIntensity;
}



void LightSpot::setAttenuation(const LightSpot::Attenuation& iAttenuation)
{
	attenuation_ = iAttenuation;
}



void LightSpot::setOuterAngle(TScalar iRadians)
{
	cosOuterAngle_ = num::cos(num::clamp(iRadians, TNumTraits::zero, TNumTraits::pi));
}



void LightSpot::setInnerAngle(TScalar iRadians)
{
	cosInnerAngle_ = num::cos(num::clamp(iRadians, TNumTraits::zero, TNumTraits::pi));
}



void LightSpot::lookAt(const TPoint3D& iTarget)
{
	setDirection(iTarget - position_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightSpot::doIntersect(const Sample& iSample, const BoundedRay& iRAy, 
							 Intersection& oResult) const
{
	oResult = Intersection::empty();
}



const bool LightSpot::doIsIntersecting(const Sample& iSample, 
										const BoundedRay& iRay) const
{
	return false;
}



void LightSpot::doLocalContext(const Sample& iSample, const BoundedRay& iRay,
								const Intersection& iIntersection, 
								IntersectionContext& oResult) const
{
	LASS_THROW("since LightSpot can never return an intersection, you've called dead code.");
}



const bool LightSpot::doContains(const Sample& iSample, const TPoint3D& iPoint) const
{
	return false;
}




const TAabb3D LightSpot::doBoundingBox() const
{
	return TAabb3D(position_, position_);
}



const TScalar LightSpot::doArea() const
{
	return 0;
}



const Spectrum LightSpot::doSampleEmission(const Sample& iSample,
											const TVector2D& iLightSample, 
											const TPoint3D& iDestination,
											BoundedRay& oShadowRay,
											TScalar& oPdf) const
{
	TVector3D toDestination = iDestination - position_;
	const TScalar squaredDistance = toDestination.squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toDestination /= distance;
	
	const TScalar cosTheta = dot(direction_, toDestination);
	if (cosTheta < cosOuterAngle_)
	{
		return Spectrum(TNumTraits::zero);
	}

	TScalar multiplier = TNumTraits::one;

	if (cosTheta < cosInnerAngle_)
	{
		multiplier = num::cubic((cosTheta - cosOuterAngle_) / (cosInnerAngle_ - cosOuterAngle_));
	}

	const TScalar attenuation = attenuation_.constant + 
		attenuation_.linear * distance + 
		attenuation_.quadratic * squaredDistance;
	multiplier /= attenuation;
	
    Spectrum result = intensity_;
	result *= multiplier;

	oShadowRay = BoundedRay(iDestination, position_, tolerance, distance);
	return result;
}



const unsigned LightSpot::doNumberOfEmissionSamples() const
{
	return 1;
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
