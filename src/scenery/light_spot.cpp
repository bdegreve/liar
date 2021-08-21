/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
#include "light_spot.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightSpot, "spot light")
PY_CLASS_CONSTRUCTOR_0(LightSpot)
PY_CLASS_MEMBER_RW(LightSpot, position, setPosition)
PY_CLASS_MEMBER_RW(LightSpot, direction, setDirection)
PY_CLASS_MEMBER_RW(LightSpot, intensity, setIntensity)
PY_CLASS_MEMBER_RW(LightSpot, attenuation, setAttenuation)
PY_CLASS_MEMBER_RW(LightSpot, outerAngle, setOuterAngle)
PY_CLASS_MEMBER_RW(LightSpot, innerAngle, setInnerAngle)
PY_CLASS_METHOD(LightSpot, lookAt);

// --- public --------------------------------------------------------------------------------------

LightSpot::LightSpot():
	position_(TPoint3D()),
	direction_(TVector3D(0, 0, -1)),
	intensity_(Spectrum::white()),
	attenuation_(Attenuation::defaultAttenuation()),
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



const TSpectrumPtr& LightSpot::intensity() const
{
	return intensity_;
}



const TAttenuationPtr& LightSpot::attenuation() const
{
	return attenuation_;
}



void LightSpot::setPosition(const TPoint3D& iPosition)
{
	position_ = iPosition;
}



TScalar LightSpot::outerAngle() const
{
	return num::acos(cosOuterAngle_);
}



TScalar LightSpot::innerAngle() const
{
	return num::acos(cosInnerAngle_);
}



void LightSpot::setDirection(const TVector3D& direction)
{
	direction_ = direction.normal();
	prim::impl::Plane3DImplDetail::generateDirections(
		direction_, tangentU_, tangentV_);
	tangentU_.normalize();
	tangentV_.normalize();
}



void LightSpot::setIntensity(const TSpectrumPtr& iIntensity)
{
	intensity_ = iIntensity;
}



void LightSpot::setAttenuation(const TAttenuationPtr& iAttenuation)
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



void LightSpot::lookAt(const TPoint3D& target)
{
	setDirection(target - position_);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightSpot::doIntersect(const Sample&, const BoundedRay&, Intersection& result) const
{
	result = Intersection::empty();
}



bool LightSpot::doIsIntersecting(const Sample&, const BoundedRay&) const
{
	return false;
}



void LightSpot::doLocalContext(const Sample&, const BoundedRay&, const Intersection&, IntersectionContext&) const
{
	LASS_THROW("since LightSpot can never return an intersection, you've called dead code.");
}



bool LightSpot::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}




const TAabb3D LightSpot::doBoundingBox() const
{
	return TAabb3D(position_, position_);
}



TScalar LightSpot::doArea() const
{
	return 0;
}



TScalar LightSpot::doArea(const TVector3D&) const
{
	return 0;
}



const Spectral LightSpot::doSampleEmission(const Sample& sample, const TPoint2D&, const TPoint3D& target, BoundedRay& shadowRay, TScalar& pdf) const
{
	TVector3D toLight = position_ - target;
	const TScalar squaredDistance = toLight.squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toLight /= distance;
	
	const TScalar cosTheta = -dot(direction_, toLight);
	TScalar multiplier = fallOff(cosTheta);
	if (multiplier == TNumTraits::zero)
	{
		pdf = TNumTraits::zero;
		return Spectral();
	}
	multiplier /= attenuation_->attenuation(distance, squaredDistance);
	
	shadowRay = BoundedRay(target, toLight, tolerance, distance, 
		prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;
	return intensity_->evaluate(sample, SpectralType::Illuminant) * static_cast<Spectral::TValue>(multiplier);
}



const Spectral LightSpot::doEmission(const Sample&, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return Spectral(0);
}



const Spectral LightSpot::doSampleEmission(const Sample& sample, const TPoint2D& lightSampleA, const TPoint2D&, BoundedRay& emissionRay, TScalar& pdf) const
{
	const TPoint3D local = num::uniformCone(lightSampleA, cosOuterAngle_, pdf);
	const TVector3D direction = tangentU_ * local.x + tangentV_ * local.y + direction_ * local.z;
	emissionRay = BoundedRay(position_, direction, tolerance);
	const TScalar cosTheta = local.z;
	return intensity_->evaluate(sample, SpectralType::Illuminant) * static_cast<Spectral::TValue>(fallOff(cosTheta));
}



TScalar LightSpot::doTotalPower() const
{
	const TScalar factor = ((1 - cosInnerAngle_) + (cosInnerAngle_ - cosOuterAngle_) / 4);
	return (2 * TNumTraits::pi * factor) * intensity_->luminance();
}


size_t LightSpot::doNumberOfEmissionSamples() const
{
	return 1;
}



bool LightSpot::doIsSingular() const
{
	return true;
}



const TPyObjectPtr LightSpot::doGetLightState() const
{
	return python::makeTuple(position_, direction_, intensity_, attenuation_,
		cosOuterAngle_, cosInnerAngle_);
}



void LightSpot::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, position_, direction_, intensity_, attenuation_,
		cosOuterAngle_, cosInnerAngle_);
}



TScalar LightSpot::fallOff(TScalar cosTheta) const
{
	if (cosTheta < cosOuterAngle_)
	{
		return 0;
	}
	if (cosTheta > cosInnerAngle_)
	{
		return 1;
	}
	return num::cubic((cosTheta - cosOuterAngle_) / (cosInnerAngle_ - cosOuterAngle_));
}


// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
