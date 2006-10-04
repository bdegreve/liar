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
#include "light_directional.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(LightDirectional)
PY_CLASS_CONSTRUCTOR_0(LightDirectional)
PY_CLASS_CONSTRUCTOR_2(LightDirectional, const TVector3D&, const Spectrum&)
PY_CLASS_MEMBER_RW(LightDirectional, "direction", direction, setDirection)
PY_CLASS_MEMBER_RW(LightDirectional, "radiance", radiance, setRadiance)


// --- public --------------------------------------------------------------------------------------

LightDirectional::LightDirectional():
	radiance_(Spectrum(1))
{
	setDirection(TVector3D(0, 0, -1));
}



LightDirectional::LightDirectional(const TVector3D& direction, const Spectrum& radiance):
	radiance_(radiance)
{
	setDirection(direction);
}



const TVector3D& LightDirectional::direction() const
{
	return direction_;
}



const Spectrum& LightDirectional::radiance() const
{
	return radiance_;
}



void LightDirectional::setDirection(const TVector3D& direction)
{
	direction_ = direction.normal();
	lass::prim::impl::Plane3DImplDetail::generateDirections(
		direction_, tangentU_, tangentV_);
}



void LightDirectional::setRadiance(const Spectrum& radiance)
{
	radiance_ = radiance;
}




// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightDirectional::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod& period)
{
}



void LightDirectional::doIntersect(const Sample& sample, const BoundedRay& iRAy, 
							 Intersection& result) const
{
	result = Intersection::empty();
}



const bool LightDirectional::doIsIntersecting(const Sample& sample, 
											  const BoundedRay& ray) const
{
	return false;
}



void LightDirectional::doLocalContext(const Sample& sample, const BoundedRay& ray,
								const Intersection& intersection, 
								IntersectionContext& result) const
{
	LASS_THROW("since LightDirectional can never return an intersection, you've called dead code.");
}



const bool LightDirectional::doContains(const Sample& sample, const TPoint3D& point) const
{
	return false;
}




const TAabb3D LightDirectional::doBoundingBox() const
{
	return TAabb3D();
}



const TScalar LightDirectional::doArea() const
{
	return 0;
}



const Spectrum LightDirectional::doSampleEmission(
		const Sample& sample,
		const TPoint2D& lightSample, 
		const TPoint3D& target,
		const TVector3D& targetNormal,
		BoundedRay& shadowRay,
		TScalar& pdf) const
{
	shadowRay = BoundedRay(target, -direction_, tolerance, TNumTraits::infinity,
		prim::IsAlreadyNormalized());
	pdf = TNumTraits::one;
	return radiance_;
}



const Spectrum LightDirectional::doSampleEmission(const TPoint2D& sampleA, const TPoint2D& sampleB,
		const TPoint3D& sceneCenter, TScalar sceneRadius, TRay3D& emissionRay, TScalar& pdf) const
{
	const TPoint2D uv = num::uniformDisk(sampleB, pdf);
	const TPoint3D begin = sceneCenter + 
		sceneRadius * (tangentU_ * uv.x + tangentV_ * uv.y - direction_);
	emissionRay = TRay3D(begin, direction_);
	pdf /= num::sqr(sceneRadius);
	return radiance_;
}



const Spectrum LightDirectional::doTotalPower(TScalar sceneRadius) const
{
	return (2 * TNumTraits::pi * num::sqr(sceneRadius)) * radiance_;
}



const unsigned LightDirectional::doNumberOfEmissionSamples() const
{
	return 1;
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