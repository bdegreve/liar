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
#include "disk.h"
#include <lass/num/distribution_transformations.h>

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Disk, "a flat disk")
PY_CLASS_CONSTRUCTOR_0(Disk)
PY_CLASS_CONSTRUCTOR_3(Disk, const TPoint3D&, const TVector3D&, TScalar)
PY_CLASS_MEMBER_RW(Disk, center, setCenter)
PY_CLASS_MEMBER_RW(Disk, normal, setNormal)
PY_CLASS_MEMBER_RW(Disk, radius, setRadius)



// --- public --------------------------------------------------------------------------------------

Disk::Disk():
	disk_(TPoint3D(0, 0, 0), TVector3D(0, 0, 1), 1)
{
}



Disk::Disk(const TPoint3D& center, const TVector3D& normal, TScalar radius):
	disk_(center, normal, radius)
{
}



Disk::Disk(const TDisk3D& disk):
	disk_(disk)
{
}



const TPoint3D& Disk::center() const
{
	return disk_.center();
}



void Disk::setCenter(const TPoint3D& center)
{
	disk_.setCenter(center);
}



const TVector3D& Disk::normal() const
{
	return disk_.normal();
}



void Disk::setNormal(const TVector3D& normal)
{
	disk_.setNormal(normal);
}



TScalar Disk::radius() const
{
	return disk_.radius();
}



void Disk::setRadius(TScalar radius)
{
	disk_.radius() = radius;
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Disk::doIntersect(const Sample&, const BoundedRay& ray, Intersection& result) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(disk_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = Intersection(this, t, seNoEvent);
	}
	else
	{
		result = Intersection::empty();
	}
}



bool Disk::doIsIntersecting(const Sample&, const BoundedRay& ray) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(disk_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



void Disk::doLocalContext(const Sample&, const BoundedRay& ray, const Intersection& intersection, IntersectionContext& result) const
{
	TPoint2D uv;
	TScalar t;
	const prim::Result LASS_UNUSED(hit) = prim::intersect(disk_, ray.unboundedRay(), uv.x, uv.y, t, ray.nearLimit());
	LASS_ASSERT(hit == prim::rOne && ray.inRange(t));
	LASS_ASSERT(t == intersection.t());
	const TPoint3D p = ray.point(intersection.t());

	const TScalar r = disk_.radius() > 0 ? disk_.radius() : 1;
	TVector3D dp_du = r * (p - disk_.center()).normal();
	if (dp_du.isZero())
	{
		dp_du = r * disk_.plane().directionU();
	}
	const TVector3D dp_dv = 2 * TNumTraits::pi * r * prim::cross(disk_.normal(), dp_du).normal();

	result.setPoint(p);
	result.setT(intersection.t());
	result.setUv(uv);
	result.setDPoint_dU(dp_du);
	result.setDPoint_dV(dp_dv);
	result.setGeometricNormal(disk_.normal());
	result.setNormal(disk_.normal());
	result.setDNormal_dU(TVector3D());
	result.setDNormal_dV(TVector3D());
}



bool Disk::doContains(const Sample&, const TPoint3D&) const
{
	return false;
}



const TAabb3D Disk::doBoundingBox() const
{
	return prim::aabb(disk_);
}



TScalar Disk::doArea() const
{
	return disk_.area();
}



TScalar Disk::doArea(const TVector3D& normal) const
{
	return disk_.area() * std::max(prim::dot(normal, disk_.normal()), TNumTraits::zero);
}



const TPyObjectPtr Disk::doGetState() const
{
	return python::makeTuple(disk_.center(), disk_.normal(), disk_.radius());
}



void Disk::doSetState(const TPyObjectPtr& state)
{
	TPoint3D center;
	TVector3D normal;
	TScalar radius;
	LASS_ENFORCE(python::decodeTuple(state, normal, radius));
	disk_ = TDisk3D(center, normal, radius);
}



bool Disk::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Disk::doSampleSurface(const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const
{
	const TPoint2D xy(num::uniformDisk(sample, pdf).position() * disk_.radius());
	normal = disk_.normal();
	pdf = num::inv(disk_.area());
	return disk_.plane().point(xy);
}



void Disk::doFun(const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	TScalar t;
	if (prim::intersect(disk_, ray, t, tolerance) == prim::rOne)
	{
		shadowRay = BoundedRay(ray, tolerance, t);
		pdf = num::sqr(t) / num::abs(disk_.area() * dot(ray.direction(), disk_.normal()));
	}
	else
	{
		shadowRay = BoundedRay(ray, tolerance);
		pdf = 0;
	}
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
