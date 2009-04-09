/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2009  Bram de Greve (bramz@users.sourceforge.net)
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
#include "sphere.h"
#include <lass/meta/type_list.h>
#include <lass/prim/impl/plane_3d_impl_detail.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Sphere, "a nice sphere")
PY_CLASS_CONSTRUCTOR_0(Sphere)
PY_CLASS_CONSTRUCTOR_2(Sphere, TPoint3D, TScalar)
PY_CLASS_MEMBER_RW(Sphere, center, setCenter)
PY_CLASS_MEMBER_RW(Sphere, radius, setRadius)



// --- public --------------------------------------------------------------------------------------

Sphere::Sphere():
	sphere_(TPoint3D(0, 0, 0), 1),
	invRadius_(1)
{
}



Sphere::Sphere(const TPoint3D& iCenter, TScalar radius):
    sphere_(iCenter, radius),
    invRadius_(num::inv(radius))
{
}



const TPoint3D& Sphere::center() const
{
    return sphere_.center();
}



void Sphere::setCenter(const TPoint3D& iCenter)
{
	sphere_.center() = iCenter;
}



const TScalar Sphere::radius() const
{
    return sphere_.radius();
}



void Sphere::setRadius(TScalar radius)
{
	sphere_.radius() = radius;
	invRadius_ = num::inv(radius);
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Sphere::doIntersect(const Sample& sample, const BoundedRay& ray, Intersection& result) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(sphere_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		SolidEvent event = 
			sphere_.contains(ray.point(ray.nearLimit())) ? seLeaving : seEntering;
		result = Intersection(this, t, event);
	}
	else
	{
		result = Intersection::empty();
	}
}



const bool Sphere::doIsIntersecting(const Sample& sample, const BoundedRay& ray) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(sphere_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



void Sphere::doLocalContext(
		const Sample& sample, const BoundedRay& ray, const Intersection& intersection,
		IntersectionContext& result) const
{
	const TScalar t = intersection.t();
	const TPoint3D point = ray.point(t);
    result.setT(t);
    result.setPoint(point);

	//         [sin theta * cos phi]
	// R = r * [sin theta * sin phi]
	//         [cos theta          ]
	//
	const TVector3D R = point - sphere_.center();
	const TVector3D normal = R.normal();
	result.setNormal(normal);

	// phi = 2pi * u
	// theta = pi * v
	//
	LASS_ASSERT(normal.z >= -TNumTraits::one && normal.z <= TNumTraits::one);
    const TScalar phi = num::atan2(normal.x, normal.y);
    const TScalar theta = num::acos(normal.z);
	result.setUv(phi / (2 * TNumTraits::pi), theta / TNumTraits::pi);

	//                   [sin theta * -sin phi]                   [cos theta * cos phi]
	// dR_du = r * 2pi * [sin theta * cos phi ]  dR_dv = r * pi * [cos theta * sin phi]
	//                   [0                   ]                   [-sin theta         ]
	//
	const TScalar sinTheta = num::sin(theta);
	const TScalar cosTheta_sinTheta = normal.z / sinTheta;
	const TVector3D dPoint_dU = 2 * TNumTraits::pi * TVector3D(-R.y, R.x, 0);
	const TVector3D dPoint_dV = sphere_.radius() * TNumTraits::pi * TVector3D(
		cosTheta_sinTheta * normal.x, cosTheta_sinTheta * normal.y, -sinTheta);
	result.setDPoint_dU(dPoint_dU);
	result.setDPoint_dV(dPoint_dV);

	//
	// dN/du = d((P-C)/r)/du = (dP/du)/r
	//
	result.setDNormal_dU(dPoint_dU * invRadius_);
	result.setDNormal_dV(dPoint_dV * invRadius_);
}



const bool Sphere::doContains(const Sample& sample, const TPoint3D& point) const
{
	return sphere_.contains(point);
}



const TAabb3D Sphere::doBoundingBox() const
{
    TScalar radius = sphere_.radius();
    TVector3D extent = TVector3D(radius, radius, radius);
    return TAabb3D(sphere_.center() - extent, sphere_.center() + extent);
}



const TScalar Sphere::doArea() const
{
	return sphere_.area();
}



const TPyObjectPtr Sphere::doGetState() const
{
	return python::makeTuple(sphere_.center(), sphere_.radius());
}



void Sphere::doSetState(const TPyObjectPtr& state)
{
	TPoint3D center;
	TScalar radius;
	LASS_ENFORCE(python::decodeTuple(state, center, radius));
	sphere_ = TSphere3D(center, radius);
	invRadius_ = num::inv(radius);
}



const bool Sphere::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Sphere::doSampleSurface(
		const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const
{
	const TScalar z = 2 * sample.y - 1;
	const TScalar rho = num::sqrt(1 - num::sqr(z));
	const TScalar theta = 2 * TNumTraits::pi * sample.x;
	const TScalar x = rho * num::cos(theta);
	const TScalar y = rho * num::sin(theta);

	normal = TVector3D(x, y, z);
	pdf = 1 / (4 * TNumTraits::pi * num::sqr(sphere_.radius()));
	return sphere_.center() + sphere_.radius() * normal;
}



const TPoint3D Sphere::doSampleSurface(
		const TPoint2D& sample, const TPoint3D& target,
		TVector3D& normal, TScalar& pdf) const
{
	if (sphere_.contains(target))
	{
		return sampleSurface(sample, normal, pdf);
	}

	TVector3D k = sphere_.center() - target;
	const TScalar sqrDistance = k.squaredNorm();
	const TScalar distance = num::sqrt(sqrDistance);
	k /= distance;
	TVector3D i, j;
	prim::impl::Plane3DImplDetail::generateDirections(k, i, j);

	const TScalar cosThetaMax = num::sqrt(
		std::max(TNumTraits::zero, 1 - num::sqr(sphere_.radius()) / sqrDistance));
	const TScalar cosTheta = 1 - sample.x * (1 - cosThetaMax);
	const TScalar phi = 2 * TNumTraits::pi * sample.y;
	const TVector3D dir = cosTheta * k + 
		num::sqrt(std::max(TNumTraits::zero, 1 - num::sqr(cosTheta))) * 
			(num::cos(phi) * i + num::sin(phi) * j);

	// http://flipcode.dxbug.com/wiki/index.php?title=Line-Sphere_%28Collision%29
	const TScalar c = sqrDistance - num::sqr(sphere_.radius());
	const TScalar delta = sqrDistance * num::sqr(cosTheta) - c;
	const TScalar t = delta > 0 ? (distance * cosTheta - num::sqrt(delta)) : (distance * cosTheta);

	const TPoint3D point = target + t * dir;
	normal = (point - sphere_.center());
	if (normal.isZero())
	{
		normal = -k;
	}
	else
	{
		normal.normalize();
	}
	pdf = num::inv(2 * TNumTraits::pi * (1 - cosThetaMax));

	return sphere_.center() + sphere_.radius() * normal;
}



void Sphere::doFun(const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	TScalar t;
	const prim::Result r = prim::intersect(sphere_, ray, t, tolerance);
	shadowRay = BoundedRay(ray, tolerance, t);
	if (sphere_.contains(ray.support()))
	{
		LASS_ASSERT(r == prim::rOne);
		pdf = num::inv(4 * TNumTraits::pi);
	}
	else
	{
		const TScalar sqrDistance = (sphere_.center() - ray.support()).squaredNorm();
		const TScalar cosThetaMax = num::sqrt(
			std::max(TNumTraits::zero, 1 - num::sqr(sphere_.radius()) / sqrDistance));
		pdf = num::inv(2 * TNumTraits::pi * (1 - cosThetaMax));		
	}
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
