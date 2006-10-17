/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2006  Bram de Greve
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
#include <lass/num/distribution_transformations.h>

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
	position_(TPoint3D()),
	intensity_(Spectrum(1)),
	attenuation_(Attenuation::defaultAttenuation())
{
}



LightPoint::LightPoint(const TPoint3D& iPosition, const Spectrum& iIntensity):
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

void LightPoint::doIntersect(
		const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
	result = Intersection::empty();
}



const bool LightPoint::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
	return false;
}



void LightPoint::doLocalContext(
		const Sample& sample, const BoundedRay& ray,
		const Intersection& intersection, IntersectionContext& result) const
{
	LASS_THROW("since LightPoint can never return an intersection, you've called dead code.");
}



const bool LightPoint::doContains(const Sample& sample, const TPoint3D& point) const
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



const Spectrum LightPoint::doEmission(
		const Sample& sample, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return Spectrum();
}


const Spectrum LightPoint::doSampleEmission(
		const Sample& sample, const TPoint2D& lightSample, 
		const TPoint3D& target,	const TVector3D& targetNormal,
		BoundedRay& shadowRay, TScalar& pdf) const
{
	TVector3D toLight = position_ - target;
    const TScalar squaredDistance = (position_ - target).squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toLight /= distance;

	shadowRay = BoundedRay(target, toLight, tolerance, distance, 
		prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;

	return intensity_ / attenuation_->attenuation(distance, squaredDistance);
}



const Spectrum LightPoint::doSampleEmission(
		const Sample& cameraSample, const TPoint2D& lightSampleA, const TPoint2D& lightSampleB,
		const TAabb3D& sceneBound, BoundedRay& emissionRay, TScalar& pdf) const
{
	const TVector3D direction = num::uniformSphere(lightSampleA, pdf).position();
	emissionRay = BoundedRay(position_, direction, tolerance);
	return intensity_;
}



const Spectrum LightPoint::doTotalPower(const TAabb3D& sceneBound) const
{
	return (4 * TNumTraits::pi) * intensity_;
}



const unsigned LightPoint::doNumberOfEmissionSamples() const
{
	return 1;
}



const bool LightPoint::doIsSingular() const
{
	return true;
}



const TPyObjectPtr LightPoint::doGetLightState() const
{
	return python::makeTuple(position_, intensity_, attenuation_);
}



void LightPoint::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, position_, intensity_, attenuation_);
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
