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
#include "light_directional.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>
#include <lass/prim/sphere_3d.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightDirectional, "directional light")
PY_CLASS_CONSTRUCTOR_0(LightDirectional)
PY_CLASS_CONSTRUCTOR_2(LightDirectional, const TVector3D&, const XYZ&)
PY_CLASS_MEMBER_RW(LightDirectional, direction, setDirection)
PY_CLASS_MEMBER_RW(LightDirectional, radiance, setRadiance)


// --- public --------------------------------------------------------------------------------------

LightDirectional::LightDirectional():
	radiance_(XYZ(1, 1, 1))
{
	setDirection(TVector3D(0, 0, -1));
}



LightDirectional::LightDirectional(const TVector3D& direction, const XYZ& radiance):
	radiance_(radiance)
{
	setDirection(direction);
}



const TVector3D& LightDirectional::direction() const
{
	return direction_;
}



const XYZ& LightDirectional::radiance() const
{
	return radiance_;
}



void LightDirectional::setDirection(const TVector3D& direction)
{
	direction_ = direction.normal();
	generateOrthonormal(direction_, tangentU_, tangentV_);
}



void LightDirectional::setRadiance(const XYZ& radiance)
{
	radiance_ = radiance;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightDirectional::doPreProcess(const TSceneObjectPtr&, const TimePeriod&)
{
}



void LightDirectional::doIntersect(const Sample&, const BoundedRay&, Intersection& result) const
{
	result = Intersection::empty();
}



bool LightDirectional::doIsIntersecting(const Sample&, const BoundedRay&) const
{
	return false;
}



void LightDirectional::doLocalContext(const Sample&, const BoundedRay&, const Intersection&, IntersectionContext&) const
{
	LASS_THROW("since LightDirectional can never return an intersection, you've called dead code.");
}



bool LightDirectional::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}




const TAabb3D LightDirectional::doBoundingBox() const
{
	return TAabb3D();
}



TScalar LightDirectional::doArea() const
{
	return 0;
}



TScalar LightDirectional::doArea(const TVector3D&) const
{
	return 0;
}



const XYZ LightDirectional::doEmission(const Sample&, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return XYZ();
}



const XYZ LightDirectional::doSampleEmission(const Sample&, const TPoint2D&, const TPoint3D& target, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = BoundedRay(target, -direction_, tolerance, TNumTraits::infinity, prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;
	return radiance_;
}



const XYZ LightDirectional::doSampleEmission(const Sample&, const TPoint2D& lightSampleA, const TPoint2D&, const TAabb3D& sceneBound, BoundedRay& emissionRay, TScalar& pdf) const
{
	const prim::Sphere3D<TScalar> worldSphere = boundingSphere(sceneBound);
	const TPoint2D uv = num::uniformDisk(lightSampleA, pdf);
	const TPoint3D begin = worldSphere.center() + worldSphere.radius() * (tangentU_ * uv.x + tangentV_ * uv.y - direction_);
	emissionRay = BoundedRay(begin, direction_, tolerance);
	pdf /= num::sqr(worldSphere.radius());
	return radiance_;
}



const XYZ LightDirectional::doTotalPower(const TAabb3D& sceneBound) const
{
	const prim::Sphere3D<TScalar> worldSphere = boundingSphere(sceneBound);
	return (2 * TNumTraits::pi * num::sqr(worldSphere.radius())) * radiance_;
}



size_t LightDirectional::doNumberOfEmissionSamples() const
{
	return 1;
}



bool LightDirectional::doIsSingular() const
{
	return true;
}



const TPyObjectPtr LightDirectional::doGetLightState() const
{
	return python::makeTuple(direction_, radiance_);
}



void LightDirectional::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, direction_, radiance_);
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
