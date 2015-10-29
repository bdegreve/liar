/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
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
#include "light_point.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightPoint, "point light")
PY_CLASS_CONSTRUCTOR_0(LightPoint)
PY_CLASS_CONSTRUCTOR_2(LightPoint, const TPoint3D&, const Spectral&)
PY_CLASS_MEMBER_RW(LightPoint, position, setPosition)
PY_CLASS_MEMBER_RW(LightPoint, intensity, setIntensity)
PY_CLASS_MEMBER_RW(LightPoint, attenuation, setAttenuation)


// --- public --------------------------------------------------------------------------------------

LightPoint::LightPoint():
	position_(TPoint3D()),
	intensity_(XYZ(1, 1, 1)),
	attenuation_(Attenuation::defaultAttenuation())
{
}



LightPoint::LightPoint(const TPoint3D& position, const Spectral& intensity) :
	position_(position),
	intensity_(intensity),
	attenuation_(Attenuation::defaultAttenuation())
{
}



const TPoint3D& LightPoint::position() const
{
	return position_;
}



const Spectral& LightPoint::intensity() const
{
	return intensity_;
}



const TAttenuationPtr& LightPoint::attenuation() const
{
	return attenuation_;
}



void LightPoint::setPosition(const TPoint3D& position)
{
	position_ = position;
}



void LightPoint::setIntensity(const Spectral& intensity)
{
	intensity_ = intensity;
}



void LightPoint::setAttenuation(const TAttenuationPtr& attenuation)
{
	attenuation_ = attenuation;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightPoint::doIntersect(const Sample&, const BoundedRay&, Intersection& result) const
{
	result = Intersection::empty();
}



bool LightPoint::doIsIntersecting(const Sample&, const BoundedRay&) const
{
	return false;
}



void LightPoint::doLocalContext(const Sample&, const BoundedRay&, const Intersection&, IntersectionContext&) const
{
	LASS_THROW("since LightPoint can never return an intersection, you've called dead code.");
}



bool LightPoint::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}




const TAabb3D LightPoint::doBoundingBox() const
{
	return TAabb3D(position_, position_);
}



TScalar LightPoint::doArea() const
{
	return 0;
}



TScalar LightPoint::doArea(const TVector3D&) const
{
	return 0;
}



const Spectral LightPoint::doEmission(const Sample&, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return Spectral();
}


const Spectral LightPoint::doSampleEmission(const Sample&, const TPoint2D&, const TPoint3D& target, BoundedRay& shadowRay, TScalar& pdf) const
{
	TVector3D toLight = position_ - target;
	const TScalar squaredDistance = (position_ - target).squaredNorm();
	const TScalar distance = num::sqrt(squaredDistance);
	toLight /= distance;

	shadowRay = BoundedRay(target, toLight, tolerance, distance, prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;

	return intensity_ / attenuation_->attenuation(distance, squaredDistance);
}



const Spectral LightPoint::doSampleEmission(const Sample&, const TPoint2D& lightSampleA, const TPoint2D&, BoundedRay& emissionRay, TScalar& pdf) const
{
	const TVector3D direction = num::uniformSphere(lightSampleA, pdf).position();
	emissionRay = BoundedRay(position_, direction, tolerance);
	return intensity_;
}



const Spectral LightPoint::doTotalPower() const
{
	return (4 * TNumTraits::pi) * intensity_;
}



size_t LightPoint::doNumberOfEmissionSamples() const
{
	return 1;
}



bool LightPoint::doIsSingular() const
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
