/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
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
 *  http://liar.sourceforge.net
 */

#include "scenery_common.h"
#include "parallelogram.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS(Parallelogram)
PY_CLASS_CONSTRUCTOR_3(Parallelogram, const TPoint3D&, const TVector3D&, const TVector3D&)

// --- public --------------------------------------------------------------------------------------

Parallelogram::Parallelogram(
		const TPoint3D& iSupport, const TVector3D& iSizeU, const TVector3D& iSizeV):
    parallelogram_(iSupport, iSizeU, iSizeV),
	normal_(cross(iSizeU, iSizeV).normal())
{
	invArea_ = num::inv(parallelogram_.area());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Parallelogram::doIntersect(
		const kernel::Sample& sample, const kernel::BoundedRay& ray, 
		kernel::Intersection& result) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = kernel::Intersection(this, t, seNoEvent);
	}
	else
	{
		result = kernel::Intersection::empty();
	}
}



const bool Parallelogram::doIsIntersecting(
		const kernel::Sample& sample, const kernel::BoundedRay& ray) const
{
    TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



void Parallelogram::doLocalContext(
		const kernel::Sample& sample, const BoundedRay& ray, 
		const kernel::Intersection& intersection, kernel::IntersectionContext& result) const
{
    TPoint2D uv;
	TScalar t;
	const prim::Result hit = prim::intersect(
		parallelogram_, ray.unboundedRay(), uv.x, uv.y, t, ray.nearLimit());
	LASS_ASSERT(hit == prim::rOne && ray.inRange(t));
	LASS_ASSERT(t == intersection.t());

	result.setPoint(ray.point(intersection.t()));
    result.setT(intersection.t());
	result.setUv(uv);
    result.setDPoint_dU(parallelogram_.sizeU());
    result.setDPoint_dV(parallelogram_.sizeV());
    result.setGeometricNormal(normal_);
    result.setNormal(normal_);
    result.setDNormal_dU(TVector3D());
    result.setDNormal_dV(TVector3D());
}



const bool Parallelogram::doContains(const kernel::Sample& sample, const TPoint3D& point) const
{
	return false;
}



const bool Parallelogram::doHasSurfaceSampling() const
{
	return true;
}



const TPoint3D Parallelogram::doSampleSurface(
		const TPoint2D& sample, TVector3D& normal, TScalar& pdf) const
{
	normal = normal_;
	pdf = invArea_;
	return parallelogram_.point(sample.x, sample.y);
}



void Parallelogram::doFun(const TRay3D& ray, BoundedRay& shadowRay, TScalar& pdf) const
{
	TScalar t;
	if (prim::intersect(parallelogram_, ray, t, tolerance) == prim::rOne)
	{
		shadowRay = BoundedRay(ray, tolerance, t);
		pdf = invArea_ * num::sqr(t) / num::abs(dot(ray.direction(), normal_));
	}
	else
	{
		shadowRay = BoundedRay(ray, tolerance);
		pdf = 0;
	}
}



const TAabb3D Parallelogram::doBoundingBox() const
{
	return prim::aabb(parallelogram_);
}



const TScalar Parallelogram::doArea() const
{
	return parallelogram_.area();
}



const TPyObjectPtr Parallelogram::doGetState() const
{
	return python::makeTuple(
		parallelogram_.support(), parallelogram_.sizeU(), parallelogram_.sizeV());
}



void Parallelogram::doSetState(const TPyObjectPtr& state)
{
	TPoint3D support;
	TVector3D sizeU;
	TVector3D sizeV;
	LASS_ENFORCE(python::decodeTuple(state, support, sizeU, sizeV));
	parallelogram_ = TParallelogram3D(support, sizeU, sizeV);
	normal_ = cross(sizeU, sizeV).normal();
	invArea_ = num::inv(parallelogram_.area());
}



// --- free ----------------------------------------------------------------------------------------



}

}

// EOF
