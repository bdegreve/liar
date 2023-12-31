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
#include "parallelogram.h"

namespace liar
{
namespace scenery
{

PY_DECLARE_CLASS_DOC(Parallelogram, "finite parallelogram")
PY_CLASS_CONSTRUCTOR_3(Parallelogram, const TPoint3D&, const TVector3D&, const TVector3D&)

// --- public --------------------------------------------------------------------------------------

Parallelogram::Parallelogram(const TPoint3D& support, const TVector3D& sizeU, const TVector3D& sizeV):
	parallelogram_(support, sizeU, sizeV),
	normal_(cross(sizeU, sizeV).normal())
{
	invArea_ = num::inv(parallelogram_.area());
}



// --- protected -----------------------------------------------------------------------------------



// --- private -------------------------------------------------------------------------------------

void Parallelogram::doIntersect(const kernel::Sample&, const kernel::BoundedRay& ray, kernel::Intersection& result) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(parallelogram_, ray.unboundedRay(), t, ray.nearLimit());
	if (hit == prim::rOne && ray.inRange(t))
	{
		result = kernel::Intersection(this, t, seNoEvent);
	}
	else
	{
		result = kernel::Intersection::empty();
	}
}



bool Parallelogram::doIsIntersecting(const kernel::Sample&, const kernel::BoundedRay& ray) const
{
	TScalar t;
	const prim::Result hit = prim::intersect(parallelogram_, ray.unboundedRay(), t, ray.nearLimit());
	return hit == prim::rOne && ray.inRange(t);
}



void Parallelogram::doLocalContext(const kernel::Sample&, const BoundedRay& ray, const kernel::Intersection& intersection, kernel::IntersectionContext& result) const
{
	TPoint2D uv;
	TScalar t;
	const prim::Result LASS_UNUSED(hit) = prim::intersect(parallelogram_, ray.unboundedRay(), uv.x, uv.y, t, ray.nearLimit());
	LASS_ASSERT(hit == prim::rOne);// && ray.inRange(t));
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



bool Parallelogram::doContains(const kernel::Sample&, const TPoint3D&) const
{
	return false;
}



bool Parallelogram::doHasSurfaceSampling() const
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



const TAabb3D Parallelogram::doBoundingBox() const
{
	return prim::aabb(parallelogram_);
}



TScalar Parallelogram::doArea() const
{
	return parallelogram_.area();
}



TScalar Parallelogram::doArea(const TVector3D& normal) const
{
	return parallelogram_.area() * std::max(prim::dot(normal, normal_), TNumTraits::zero);
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
