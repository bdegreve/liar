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
#include "disk.h"
#include <lass/num/distribution_transformations.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(LightDirectional, "directional light")
PY_CLASS_CONSTRUCTOR_0(LightDirectional)
PY_CLASS_CONSTRUCTOR_2(LightDirectional, const TVector3D&, const TSpectrumPtr&)
PY_CLASS_MEMBER_RW(LightDirectional, direction, setDirection)
PY_CLASS_MEMBER_RW(LightDirectional, radiance, setRadiance)
PY_CLASS_MEMBER_RW(LightDirectional, portal, setPortal)


// --- public --------------------------------------------------------------------------------------

LightDirectional::LightDirectional():
radiance_(Spectrum::white())
{
	setDirection(TVector3D(0, 0, -1));
}



LightDirectional::LightDirectional(const TVector3D& direction, const TSpectrumPtr& radiance) :
	radiance_(radiance)
{
	setDirection(direction);
}



const TVector3D& LightDirectional::direction() const
{
	return direction_;
}



const TSpectrumPtr& LightDirectional::radiance() const
{
	return radiance_;
}



const TSceneObjectPtr& LightDirectional::portal() const
{
	return userPortal_;
}



void LightDirectional::setDirection(const TVector3D& direction)
{
	direction_ = direction.normal();
}



void LightDirectional::setRadiance(const TSpectrumPtr& radiance)
{
	radiance_ = radiance;
}



void LightDirectional::setPortal(const TSceneObjectPtr& portal)
{
	if (portal && !portal->hasSurfaceSampling())
	{
		throw python::PythonException(PyExc_TypeError, "portal must support surface sampling", LASS_PRETTY_FUNCTION);
	}
	userPortal_	= portal;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void LightDirectional::doPreProcess(const TSceneObjectPtr& scene, const TimePeriod&)
{
	const prim::Sphere3D<TScalar> bounds = prim::boundingSphere(scene->boundingBox());
	const TPoint3D diskCenter = bounds.center() - bounds.radius() * direction_;
	defaultPortal_.reset(new Disk(diskCenter, direction_, bounds.radius()));
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
	return userPortal_ ? userPortal_->boundingBox() : TAabb3D();
}



TScalar LightDirectional::doArea() const
{
	return 0;
}



TScalar LightDirectional::doArea(const TVector3D&) const
{
	return 0;
}



const Spectral LightDirectional::doEmission(const Sample&, const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = ray;
	pdf = 0;
	return Spectral();
}



const Spectral LightDirectional::doSampleEmission(const Sample& sample, const TPoint2D&, const TPoint3D& target, BoundedRay& shadowRay, TScalar& pdf) const
{
	shadowRay = BoundedRay(target, -direction_, tolerance, TNumTraits::infinity, prim::IsAlreadyNormalized());
	if (userPortal_)
	{
		Intersection intersection;
		userPortal_->intersect(sample, shadowRay, intersection);
		if (!intersection)
		{
			pdf = 0;
			return Spectral(0);
		}
		shadowRay = BoundedRay(target, -direction_, tolerance, intersection.t(), prim::IsAlreadyNormalized());
	} 
	pdf = TNumTraits::one;
	return radiance_->evaluate(sample, Illuminant);
}



const Spectral LightDirectional::doSampleEmission(const Sample& sample, const TPoint2D& lightSampleA, const TPoint2D&, BoundedRay& emissionRay, TScalar& pdf) const
{
	TVector3D normal;
	const TSceneObjectPtr& port = userPortal_ ? userPortal_ : defaultPortal_;
	LASS_ASSERT(port);
	const TPoint3D begin = port->sampleSurface(lightSampleA, -direction_, normal, pdf);
	emissionRay = BoundedRay(begin, direction_, tolerance);
	const TScalar cosTheta = dot(direction_, normal);
	if (pdf > 0 && cosTheta > 0)
	{
		pdf /= cosTheta;
		return radiance_->evaluate(sample, Illuminant);
	}
	else
	{
		pdf = 0;
		return Spectral(0);
	}
}



TScalar LightDirectional::doTotalPower() const
{
	const TSceneObjectPtr& port = userPortal_ ? userPortal_ : defaultPortal_;
	LASS_ASSERT(port);
	return port->area(direction_) * radiance_->luminance();
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
	return python::makeTuple(direction_, radiance_, userPortal_);
}



void LightDirectional::doSetLightState(const TPyObjectPtr& state)
{
	python::decodeTuple(state, direction_, radiance_, userPortal_);
}




// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
